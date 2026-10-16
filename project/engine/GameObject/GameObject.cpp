#include "engine/GameObject/GameObject.h"

#include "engine/GameObject/GameObjectManager.h"
#include "engine/GameObject/TransformComponent.h"

#include <atomic>
#include <cassert>

namespace TakeC {
namespace {

std::atomic<GameObjectId> gNextGameObjectId{ 1 };

} // namespace

GameObject::GameObject(std::string name)
	: id_(gNextGameObjectId.fetch_add(1, std::memory_order_relaxed)),
	name_(std::move(name)) {
	auto transform = std::make_unique<TransformComponent>();
	transform_ = transform.get();
	AttachComponent(std::move(transform));
}

GameObject::~GameObject() {
	DetachAllComponents();
}

TransformComponent& GameObject::GetTransform() {
	assert(transform_ != nullptr);
	return *transform_;
}

const TransformComponent& GameObject::GetTransform() const {
	assert(transform_ != nullptr);
	return *transform_;
}

EngineServices* GameObject::GetServices() {
	return manager_ ? &manager_->GetServices() : nullptr;
}

const EngineServices* GameObject::GetServices() const {
	return manager_ ? &manager_->GetServices() : nullptr;
}

void GameObject::Initialize() {
	// 無効化されているか破棄要求されている場合は初期化しない
	if (!active_ || destroyRequested_) {
		return;
	}

	// コンポーネントのStartを呼び出す
	iteratingComponents_ = true;

	for (const std::unique_ptr<Component>& component : components_) {
		// Startは有効化されているコンポーネントに対してのみ呼び出す
		component->StartIfNeeded();
	}

	iteratingComponents_ = false;

	FlushPendingComponents();
}

void GameObject::Update(float deltaTime) {
	RunPhasedUpdate(UpdateKind::Update, deltaTime);
}

void GameObject::FixedUpdate(float fixedDeltaTime) {
	RunPhasedUpdate(UpdateKind::FixedUpdate, fixedDeltaTime);
}

void GameObject::LateUpdate(float deltaTime) {
	RunPhasedUpdate(UpdateKind::LateUpdate, deltaTime);
}

//=============================================================================
// フェーズごとの更新処理
//=============================================================================
void GameObject::RunPhasedUpdate(UpdateKind kind, float deltaTime) {
	if (!active_ || destroyRequested_) {
		return;
	}

	// Standaloneモードでは、Updateが呼ばれたフレームでLateUpdateも呼ばれると想定して、フレーム番号を1回だけ進める
	if (!standaloneFrameOpen_ || (kind == UpdateKind::Update && standaloneUpdatedThisFrame_)) {
		++standaloneFrameIndex_;
		standaloneFrameOpen_ = true;
		standaloneUpdatedThisFrame_ = false;
	}

	if (kind == UpdateKind::Update) {
		standaloneUpdatedThisFrame_ = true;
	}

	//
	const ComponentUpdateContext context{
		deltaTime,
		manager_ ? manager_->GetFrameIndex() : standaloneFrameIndex_,
		manager_,
		GetServices()
	};
	BeginComponentUpdate();
	try {
		for (const ComponentPhase phase : kUpdatePhases) {
			UpdatePhase(phase, kind, context);
		}
	} catch (...) {
		EndComponentUpdate();
		if (kind == UpdateKind::LateUpdate) {
			standaloneFrameOpen_ = false;
		}
		throw;
	}
	EndComponentUpdate();
	if (kind == UpdateKind::LateUpdate) {
		standaloneFrameOpen_ = false;
	}
}

void GameObject::BeginComponentUpdate() {
	assert(!iteratingComponents_);
	iteratingComponents_ = true;
}

void GameObject::UpdatePhase(ComponentPhase phase, UpdateKind kind, const ComponentUpdateContext& context) {
	if (!active_ || destroyRequested_) {
		return;
	}

	for (const std::unique_ptr<Component>& component : components_) {
		if (component->GetUpdatePhase() != phase) {
			continue;
		}
		switch (kind) {
		case UpdateKind::Update:
			component->UpdateInternal(context);
			break;
		case UpdateKind::FixedUpdate:
			component->FixedUpdateInternal(context);
			break;
		case UpdateKind::LateUpdate:
			component->LateUpdateInternal(context);
			break;
		}
	}
}

void GameObject::EndComponentUpdate() {
	iteratingComponents_ = false;
	FlushPendingComponents();
}

void GameObject::AttachComponent(std::unique_ptr<Component> component) {
	assert(component != nullptr);
	component->Attach(*this);
	if (iteratingComponents_) {
		pendingComponents_.push_back(std::move(component));
		return;
	}
	components_.push_back(std::move(component));
}

void GameObject::FlushPendingComponents() {
	if (iteratingComponents_ || pendingComponents_.empty()) {
		return;
	}

	for (std::unique_ptr<Component>& component : pendingComponents_) {
		components_.push_back(std::move(component));
	}
	pendingComponents_.clear();
}

void GameObject::DetachAllComponents() {
	for (auto iterator = pendingComponents_.rbegin(); iterator != pendingComponents_.rend(); ++iterator) {
		(*iterator)->Detach();
	}
	pendingComponents_.clear();

	for (auto iterator = components_.rbegin(); iterator != components_.rend(); ++iterator) {
		(*iterator)->Detach();
	}
	components_.clear();
	transform_ = nullptr;
}

} // namespace TakeC
