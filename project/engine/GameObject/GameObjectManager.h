#pragma once

#include "engine/GameObject/GameObject.h"

#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace TakeC {

/// <summary>
/// GameObjectの所有、検索、一括更新、安全な遅延破棄を担当するクラスです。
/// </summary>
class GameObjectManager final {
public:
	GameObjectManager() = default;
	~GameObjectManager() = default;

	GameObjectManager(const GameObjectManager&) = delete;
	GameObjectManager& operator=(const GameObjectManager&) = delete;
	GameObjectManager(GameObjectManager&&) = delete;
	GameObjectManager& operator=(GameObjectManager&&) = delete;

	GameObject& CreateObject(std::string name = "GameObject");
	void DestroyObject(GameObjectId id);

	GameObject* FindObject(GameObjectId id);
	const GameObject* FindObject(GameObjectId id) const;
	GameObject* FindObjectByName(std::string_view name);
	const GameObject* FindObjectByName(std::string_view name) const;

	void Start();
	void Update(float deltaTime);
	void FixedUpdate(float fixedDeltaTime);
	void LateUpdate(float deltaTime);
	void FlushPendingChanges();
	void Clear();

	std::size_t GetObjectCount() const {
		return objects_.size() + pendingObjects_.size();
	}

private:
	template<class Function>
	void ForEachObject(Function&& function) {
		iteratingObjects_ = true;
		for (const std::unique_ptr<GameObject>& object : objects_) {
			if (!object->IsDestroyRequested()) {
				function(*object);
			}
		}
		iteratingObjects_ = false;
		FlushPendingChanges();
	}

	std::vector<std::unique_ptr<GameObject>> objects_;
	std::vector<std::unique_ptr<GameObject>> pendingObjects_;
	bool iteratingObjects_ = false;
	bool clearRequested_ = false;
};

} // namespace TakeC
