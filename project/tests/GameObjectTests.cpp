#include "engine/GameObject/GameObjectManager.h"
#include "engine/GameObject/TransformComponent.h"
#include "engine/Collision/CollisionEvent.h"

#include <cassert>
#include <cmath>
#include <memory>
#include <stdexcept>
#include <vector>

namespace {

/// <summary>
/// テスト用コンポーネントのライフサイクル呼び出し回数を保持します。
/// </summary>
struct LifecycleStats final {
	int attached = 0;
	int started = 0;
	int updated = 0;
	int fixedUpdated = 0;
	int lateUpdated = 0;
	int detached = 0;
};

/// <summary>
/// Componentのライフサイクルを記録するテスト用コンポーネントです。
/// </summary>
class CountingComponent final : public TakeC::Component {
public:
	explicit CountingComponent(LifecycleStats& stats) : stats_(stats) {}

protected:
	void OnAttach() override { ++stats_.attached; }
	void Start() override { ++stats_.started; }
	void Update(const TakeC::ComponentUpdateContext&) override { ++stats_.updated; }
	void FixedUpdate(const TakeC::ComponentUpdateContext&) override { ++stats_.fixedUpdated; }
	void LateUpdate(const TakeC::ComponentUpdateContext&) override { ++stats_.lateUpdated; }
	void OnDetach() override { ++stats_.detached; }

private:
	LifecycleStats& stats_;
};

/// <summary>
/// 更新中のGameObject破棄要求が遅延処理されることを検証するコンポーネントです。
/// </summary>
class DestroyOnUpdateComponent final : public TakeC::Component {
protected:
	void Update(const TakeC::ComponentUpdateContext&) override {
		GetOwner().Destroy();
	}
};

/// <summary>
/// 各更新入口でフェーズ順を記録するテスト用コンポーネントです。
/// </summary>
class PhaseRecorderComponent final : public TakeC::Component {
public:
	PhaseRecorderComponent(std::vector<int>& trace, TakeC::ComponentPhase phase, int value)
		: trace_(trace), phase_(phase), value_(value) {}
	TakeC::ComponentPhase GetUpdatePhase() const noexcept override { return phase_; }

protected:
	void Update(const TakeC::ComponentUpdateContext&) override { trace_.push_back(value_); }
	void FixedUpdate(const TakeC::ComponentUpdateContext&) override { trace_.push_back(value_); }
	void LateUpdate(const TakeC::ComponentUpdateContext&) override { trace_.push_back(value_); }

private:
	std::vector<int>& trace_;
	TakeC::ComponentPhase phase_;
	int value_;
};

/// <summary>
/// 更新中に後続フェーズのComponentを追加するテスト用コンポーネントです。
/// </summary>
class AddDecisionComponent final : public TakeC::Component {
public:
	explicit AddDecisionComponent(std::vector<int>& trace) : trace_(trace) {}
	TakeC::ComponentPhase GetUpdatePhase() const noexcept override { return TakeC::ComponentPhase::Perception; }

protected:
	void Update(const TakeC::ComponentUpdateContext&) override {
		if (!added_) {
			GetOwner().AddComponent<PhaseRecorderComponent>(trace_, TakeC::ComponentPhase::Decision, 2);
			added_ = true;
		}
	}

private:
	std::vector<int>& trace_;
	bool added_ = false;
};

/// <summary>
/// Contextから取得する共有サービスのテスト用データです。
/// </summary>
struct TestWorldService final {
	int value = 42;
};

/// <summary>
/// 各更新入口で渡されたContextの内容を記録します。
/// </summary>
struct ContextSnapshot final {
	float deltaTime = 0.0f;
	std::uint64_t frameIndex = 0;
	TakeC::GameObjectManager* world = nullptr;
	TakeC::EngineServices* services = nullptr;
};

/// <summary>
/// ComponentUpdateContextを記録するテスト用コンポーネントです。
/// </summary>
class ContextCaptureComponent final : public TakeC::Component {
public:
	explicit ContextCaptureComponent(std::vector<ContextSnapshot>& snapshots) : snapshots_(snapshots) {}

protected:
	void Update(const TakeC::ComponentUpdateContext& context) override { Capture(context); }
	void FixedUpdate(const TakeC::ComponentUpdateContext& context) override { Capture(context); }
	void LateUpdate(const TakeC::ComponentUpdateContext& context) override { Capture(context); }

private:
	void Capture(const TakeC::ComponentUpdateContext& context) {
		snapshots_.push_back({ context.deltaTime, context.frameIndex, context.world, context.services });
	}
	std::vector<ContextSnapshot>& snapshots_;
};

bool NearlyEqual(float left, float right) {
	return std::abs(left - right) < 0.0001f;
}

void TestComponentLifecycleAndLookup() {
	LifecycleStats stats;
	TakeC::GameObjectManager manager;
	TakeC::GameObject& object = manager.CreateObject("Player");
	const TakeC::GameObjectId id = object.GetId();

	assert(id != TakeC::kInvalidGameObjectId);
	assert(manager.FindObject(id) == &object);
	assert(manager.FindObjectByName("Player") == &object);
	assert(object.GetComponent<TakeC::TransformComponent>() == &object.GetTransform());

	auto& first = object.AddComponent<CountingComponent>(stats);
	object.AddComponent<CountingComponent>(stats);
	assert(object.GetComponents<CountingComponent>().size() == 2);
	assert(stats.attached == 2);

	manager.Start();
	manager.Start();
	assert(stats.started == 2);

	manager.Update(1.0f / 60.0f);
	manager.FixedUpdate(1.0f / 60.0f);
	manager.LateUpdate(1.0f / 60.0f);
	assert(stats.updated == 2);
	assert(stats.fixedUpdated == 2);
	assert(stats.lateUpdated == 2);

	first.SetEnabled(false);
	manager.Update(1.0f / 60.0f);
	assert(stats.updated == 3);

	manager.DestroyObject(id);
	assert(manager.FindObject(id) == &object);
	manager.FlushPendingChanges();
	assert(manager.FindObject(id) == nullptr);
	assert(stats.detached == 2);
}

void TestTransformHierarchy() {
	TakeC::GameObjectManager manager;
	TakeC::GameObject& parent = manager.CreateObject("Parent");
	TakeC::GameObject& child = manager.CreateObject("Child");

	parent.GetTransform().SetLocalPosition({ 10.0f, 0.0f, 0.0f });
	child.GetTransform().SetLocalPosition({ 0.0f, 2.0f, 0.0f });
	child.GetTransform().SetParent(&parent.GetTransform());

	TakeC::Vector3 worldPosition = child.GetTransform().GetWorldPosition();
	assert(NearlyEqual(worldPosition.x, 10.0f));
	assert(NearlyEqual(worldPosition.y, 2.0f));
	assert(NearlyEqual(worldPosition.z, 0.0f));

	parent.GetTransform().SetLocalPosition({ 20.0f, 0.0f, 0.0f });
	worldPosition = child.GetTransform().GetWorldPosition();
	assert(NearlyEqual(worldPosition.x, 20.0f));

	bool rejectedCycle = false;
	try {
		parent.GetTransform().SetParent(&child.GetTransform());
	} catch (const std::invalid_argument&) {
		rejectedCycle = true;
	}
	assert(rejectedCycle);

	manager.DestroyObject(parent.GetId());
	manager.FlushPendingChanges();
	assert(child.GetTransform().GetParent() == nullptr);
}

void TestDeferredDestroyDuringUpdate() {
	TakeC::GameObjectManager manager;
	TakeC::GameObject& object = manager.CreateObject("Temporary");
	object.AddComponent<DestroyOnUpdateComponent>();

	manager.Update(1.0f / 60.0f);
	assert(manager.GetObjectCount() == 0);
}

void TestComponentPhaseOrderAcrossObjects() {
	using TakeC::ComponentPhase;
	std::vector<int> trace;
	TakeC::GameObjectManager manager;
	auto& first = manager.CreateObject("First");
	first.AddComponent<PhaseRecorderComponent>(trace, ComponentPhase::Presentation, 5);
	first.AddComponent<PhaseRecorderComponent>(trace, ComponentPhase::Decision, 2);
	first.AddComponent<PhaseRecorderComponent>(trace, ComponentPhase::Simulation, 4);
	auto& second = manager.CreateObject("Second");
	second.AddComponent<PhaseRecorderComponent>(trace, ComponentPhase::Navigation, 3);
	second.AddComponent<PhaseRecorderComponent>(trace, ComponentPhase::Perception, 1);

	const std::vector<int> expected{ 1, 2, 3, 4, 5 };
	manager.Update(0.1f);
	assert(trace == expected);
	trace.clear();
	manager.FixedUpdate(0.1f);
	assert(trace == expected);
	trace.clear();
	manager.LateUpdate(0.1f);
	assert(trace == expected);

	trace.clear();
	TakeC::GameObject direct("Direct");
	direct.AddComponent<PhaseRecorderComponent>(trace, ComponentPhase::Presentation, 5);
	direct.AddComponent<PhaseRecorderComponent>(trace, ComponentPhase::Perception, 1);
	direct.AddComponent<PhaseRecorderComponent>(trace, ComponentPhase::Decision, 2);
	direct.AddComponent<PhaseRecorderComponent>(trace, ComponentPhase::Navigation, 3);
	direct.AddComponent<PhaseRecorderComponent>(trace, ComponentPhase::Simulation, 4);
	direct.Update(0.1f);
	assert(trace == expected);
}

void TestComponentAddedDuringPhaseStartsNextUpdate() {
	std::vector<int> trace;
	TakeC::GameObjectManager manager;
	auto& object = manager.CreateObject("Adder");
	object.AddComponent<AddDecisionComponent>(trace);
	manager.Update(0.1f);
	assert(trace.empty());
	manager.Update(0.1f);
	assert((trace == std::vector<int>{ 2 }));
}

void TestComponentUpdateContext() {
	std::vector<ContextSnapshot> snapshots;
	TakeC::GameObjectManager manager;
	manager.GetServices().Register(std::make_shared<TestWorldService>());
	auto& object = manager.CreateObject("Context");
	object.AddComponent<ContextCaptureComponent>(snapshots);

	manager.FixedUpdate(0.02f);
	manager.FixedUpdate(0.02f);
	manager.Update(0.1f);
	manager.LateUpdate(0.1f);
	assert(snapshots.size() == 4);
	for (const auto& snapshot : snapshots) {
		assert(snapshot.frameIndex == 1);
		assert(snapshot.world == &manager);
		assert(snapshot.services == &manager.GetServices());
		assert(snapshot.services->TryGet<TestWorldService>()->value == 42);
	}
	assert(NearlyEqual(snapshots[0].deltaTime, 0.02f));
	assert(NearlyEqual(snapshots[2].deltaTime, 0.1f));
	manager.Update(0.1f);
	manager.Update(0.1f);
	assert(snapshots[4].frameIndex == 2);
	assert(snapshots[5].frameIndex == 3);
	assert(manager.GetServices().Remove<TestWorldService>());
	assert(manager.GetServices().TryGet<TestWorldService>() == nullptr);

	std::vector<ContextSnapshot> standaloneSnapshots;
	TakeC::GameObject standalone("Standalone");
	standalone.AddComponent<ContextCaptureComponent>(standaloneSnapshots);
	standalone.Update(0.1f);
	standalone.LateUpdate(0.1f);
	assert(standaloneSnapshots.size() == 2);
	assert(standaloneSnapshots[0].frameIndex == 1);
	assert(standaloneSnapshots[1].frameIndex == 1);
	assert(standaloneSnapshots[0].world == nullptr);
	assert(standaloneSnapshots[0].services == nullptr);
}

void TestCollisionEventIdentityAndOptionalContact() {
	const TakeC::ColliderId first = TakeC::AllocateColliderId();
	const TakeC::ColliderId second = TakeC::AllocateColliderId();
	assert(first != TakeC::kInvalidColliderId && second > first);

	TakeC::CollisionEvent trigger;
	trigger.selfObjectId = 11;
	trigger.otherObjectId = 12;
	trigger.selfColliderId = first;
	trigger.otherColliderId = second;
	trigger.phase = TakeC::CollisionPhase::Enter;
	trigger.isTrigger = true;
	assert(!trigger.contact.has_value());

	trigger.phase = TakeC::CollisionPhase::Exit;
	assert(trigger.selfColliderId == first && trigger.otherColliderId == second);
	assert(!trigger.contact.has_value());
}

} // namespace

int main() {
	TestComponentLifecycleAndLookup();
	TestTransformHierarchy();
	TestDeferredDestroyDuringUpdate();
	TestComponentPhaseOrderAcrossObjects();
	TestComponentAddedDuringPhaseStartsNextUpdate();
	TestComponentUpdateContext();
	TestCollisionEventIdentityAndOptionalContact();
	return 0;
}
