#include "engine/GameObject/GameObjectManager.h"
#include "engine/GameObject/TransformComponent.h"
#include "engine/Collision/CollisionEvent.h"

#include <cassert>
#include <cmath>
#include <stdexcept>

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
	TestCollisionEventIdentityAndOptionalContact();
	return 0;
}
