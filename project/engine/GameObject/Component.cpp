#include "engine/GameObject/Component.h"

#include "engine/GameObject/GameObject.h"

#include <cassert>

using namespace TakeC;

GameObject& Component::GetOwner() {
	assert(owner_ != nullptr);
	return *owner_;
}

const GameObject& Component::GetOwner() const {
	assert(owner_ != nullptr);
	return *owner_;
}

void Component::Attach(GameObject& owner) {
	assert(owner_ == nullptr);
	owner_ = &owner;
	OnAttach();
}

void Component::Detach() {
	if (owner_ == nullptr) {
		return;
	}

	OnDetach();
	owner_ = nullptr;
	started_ = false;
}

void Component::StartIfNeeded() {
	if (!enabled_ || started_) {
		return;
	}

	Start();
	started_ = true;
}

void Component::UpdateInternal(const ComponentUpdateContext& context) {
	if (!enabled_) {
		return;
	}

	StartIfNeeded();
	Update(context);
}

void Component::FixedUpdateInternal(const ComponentUpdateContext& context) {
	if (!enabled_) {
		return;
	}

	StartIfNeeded();
	FixedUpdate(context);
}

void Component::LateUpdateInternal(const ComponentUpdateContext& context) {
	if (!enabled_) {
		return;
	}

	StartIfNeeded();
	LateUpdate(context);
}
