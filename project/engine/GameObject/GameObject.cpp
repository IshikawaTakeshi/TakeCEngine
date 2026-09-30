#include "engine/GameObject/GameObject.h"

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

void GameObject::Start() {
	if (!active_ || destroyRequested_) {
		return;
	}

	iteratingComponents_ = true;
	for (const std::unique_ptr<Component>& component : components_) {
		component->StartIfNeeded();
	}
	iteratingComponents_ = false;
	FlushPendingComponents();
}

void GameObject::Update(float deltaTime) {
	if (!active_ || destroyRequested_) {
		return;
	}

	const ComponentUpdateContext context{ deltaTime };
	iteratingComponents_ = true;
	for (const std::unique_ptr<Component>& component : components_) {
		component->UpdateInternal(context);
	}
	iteratingComponents_ = false;
	FlushPendingComponents();
}

void GameObject::FixedUpdate(float fixedDeltaTime) {
	if (!active_ || destroyRequested_) {
		return;
	}

	const ComponentUpdateContext context{ fixedDeltaTime };
	iteratingComponents_ = true;
	for (const std::unique_ptr<Component>& component : components_) {
		component->FixedUpdateInternal(context);
	}
	iteratingComponents_ = false;
	FlushPendingComponents();
}

void GameObject::LateUpdate(float deltaTime) {
	if (!active_ || destroyRequested_) {
		return;
	}

	const ComponentUpdateContext context{ deltaTime };
	iteratingComponents_ = true;
	for (const std::unique_ptr<Component>& component : components_) {
		component->LateUpdateInternal(context);
	}
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
