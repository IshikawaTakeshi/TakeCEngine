#include "engine/GameObject/GameObjectManager.h"

#include <algorithm>
#include <utility>

namespace TakeC {

GameObject& GameObjectManager::CreateObject(std::string name) {
	auto object = std::make_unique<GameObject>(std::move(name));
	object->manager_ = this;
	GameObject& reference = *object;
	if (iteratingObjects_) {
		pendingObjects_.push_back(std::move(object));
	} else {
		objects_.push_back(std::move(object));
	}
	return reference;
}

void GameObjectManager::DestroyObject(GameObjectId id) {
	if (GameObject* object = FindObject(id)) {
		object->Destroy();
	}
}

GameObject* GameObjectManager::FindObject(GameObjectId id) {
	const auto findById = [id](const std::unique_ptr<GameObject>& object) {
		return object->GetId() == id;
	};
	if (const auto iterator = std::ranges::find_if(objects_, findById);
		iterator != objects_.end()) {
		return iterator->get();
	}
	if (const auto iterator = std::ranges::find_if(pendingObjects_, findById);
		iterator != pendingObjects_.end()) {
		return iterator->get();
	}
	return nullptr;
}

const GameObject* GameObjectManager::FindObject(GameObjectId id) const {
	const auto findById = [id](const std::unique_ptr<GameObject>& object) {
		return object->GetId() == id;
	};
	if (const auto iterator = std::ranges::find_if(objects_, findById);
		iterator != objects_.end()) {
		return iterator->get();
	}
	if (const auto iterator = std::ranges::find_if(pendingObjects_, findById);
		iterator != pendingObjects_.end()) {
		return iterator->get();
	}
	return nullptr;
}

GameObject* GameObjectManager::FindObjectByName(std::string_view name) {
	const auto findByName = [name](const std::unique_ptr<GameObject>& object) {
		return object->GetName() == name;
	};
	if (const auto iterator = std::ranges::find_if(objects_, findByName);
		iterator != objects_.end()) {
		return iterator->get();
	}
	if (const auto iterator = std::ranges::find_if(pendingObjects_, findByName);
		iterator != pendingObjects_.end()) {
		return iterator->get();
	}
	return nullptr;
}

const GameObject* GameObjectManager::FindObjectByName(std::string_view name) const {
	const auto findByName = [name](const std::unique_ptr<GameObject>& object) {
		return object->GetName() == name;
	};
	if (const auto iterator = std::ranges::find_if(objects_, findByName);
		iterator != objects_.end()) {
		return iterator->get();
	}
	if (const auto iterator = std::ranges::find_if(pendingObjects_, findByName);
		iterator != pendingObjects_.end()) {
		return iterator->get();
	}
	return nullptr;
}

void GameObjectManager::Start() {
	ForEachObject([](GameObject& object) { object.Start(); });
}

void GameObjectManager::Update(float deltaTime) {
	RunPhasedUpdate(GameObject::UpdateKind::Update, deltaTime);
}

void GameObjectManager::FixedUpdate(float fixedDeltaTime) {
	RunPhasedUpdate(GameObject::UpdateKind::FixedUpdate, fixedDeltaTime);
}

void GameObjectManager::LateUpdate(float deltaTime) {
	RunPhasedUpdate(GameObject::UpdateKind::LateUpdate, deltaTime);
}

void GameObjectManager::BeginFrameIfNeeded(GameObject::UpdateKind kind) {
	if (!frameOpen_ || (kind == GameObject::UpdateKind::Update && updatedThisFrame_)) {
		++frameIndex_;
		frameOpen_ = true;
		updatedThisFrame_ = false;
	}
	if (kind == GameObject::UpdateKind::Update) {
		updatedThisFrame_ = true;
	}
}

void GameObjectManager::RunPhasedUpdate(GameObject::UpdateKind kind, float deltaTime) {
	BeginFrameIfNeeded(kind);
	const ComponentUpdateContext context{ deltaTime, frameIndex_, this, &services_ };
	iteratingObjects_ = true;
	for (const std::unique_ptr<GameObject>& object : objects_) {
		object->BeginComponentUpdate();
	}
	try {
		for (const ComponentPhase phase : GameObject::kUpdatePhases) {
			for (const std::unique_ptr<GameObject>& object : objects_) {
				if (clearRequested_) {
					break;
				}
				object->UpdatePhase(phase, kind, context);
			}
		}
	} catch (...) {
		for (const std::unique_ptr<GameObject>& object : objects_) {
			object->EndComponentUpdate();
		}
		iteratingObjects_ = false;
		FlushPendingChanges();
		if (kind == GameObject::UpdateKind::LateUpdate) {
			frameOpen_ = false;
		}
		throw;
	}
	for (const std::unique_ptr<GameObject>& object : objects_) {
		object->EndComponentUpdate();
	}
	iteratingObjects_ = false;
	FlushPendingChanges();
	if (kind == GameObject::UpdateKind::LateUpdate) {
		frameOpen_ = false;
	}
}

void GameObjectManager::FlushPendingChanges() {
	if (iteratingObjects_) {
		return;
	}

	if (clearRequested_) {
		pendingObjects_.clear();
		objects_.clear();
		clearRequested_ = false;
		return;
	}

	std::erase_if(objects_, [](const std::unique_ptr<GameObject>& object) {
		return object->IsDestroyRequested();
	});
	std::erase_if(pendingObjects_, [](const std::unique_ptr<GameObject>& object) {
		return object->IsDestroyRequested();
	});

	for (std::unique_ptr<GameObject>& object : pendingObjects_) {
		objects_.push_back(std::move(object));
	}
	pendingObjects_.clear();
}

void GameObjectManager::Clear() {
	if (iteratingObjects_) {
		clearRequested_ = true;
		return;
	}
	pendingObjects_.clear();
	objects_.clear();
}

} // namespace TakeC
