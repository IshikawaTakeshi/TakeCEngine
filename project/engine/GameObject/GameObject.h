#pragma once

#include "engine/GameObject/Component.h"

#include <concepts>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace TakeC {

class TransformComponent;

using GameObjectId = std::uint64_t;
inline constexpr GameObjectId kInvalidGameObjectId = 0;

/// <summary>
/// 複数のコンポーネントを組み合わせてゲーム内の一つの実体を表すクラスです。
/// </summary>
class GameObject final {
public:
	explicit GameObject(std::string name = "GameObject");
	~GameObject();

	GameObject(const GameObject&) = delete;
	GameObject& operator=(const GameObject&) = delete;
	GameObject(GameObject&&) = delete;
	GameObject& operator=(GameObject&&) = delete;

	template<class T, class... Args>
		requires std::derived_from<T, Component> && (!std::same_as<T, TransformComponent>)
	T& AddComponent(Args&&... args) {
		auto component = std::make_unique<T>(std::forward<Args>(args)...);
		T& reference = *component;
		AttachComponent(std::move(component));
		return reference;
	}

	template<class T>
		requires std::derived_from<T, Component>
	T* GetComponent() {
		for (const std::unique_ptr<Component>& component : components_) {
			if (T* result = dynamic_cast<T*>(component.get())) {
				return result;
			}
		}
		for (const std::unique_ptr<Component>& component : pendingComponents_) {
			if (T* result = dynamic_cast<T*>(component.get())) {
				return result;
			}
		}
		return nullptr;
	}

	template<class T>
		requires std::derived_from<T, Component>
	const T* GetComponent() const {
		for (const std::unique_ptr<Component>& component : components_) {
			if (const T* result = dynamic_cast<const T*>(component.get())) {
				return result;
			}
		}
		for (const std::unique_ptr<Component>& component : pendingComponents_) {
			if (const T* result = dynamic_cast<const T*>(component.get())) {
				return result;
			}
		}
		return nullptr;
	}

	template<class T>
		requires std::derived_from<T, Component>
	std::vector<T*> GetComponents() {
		std::vector<T*> results;
		for (const std::unique_ptr<Component>& component : components_) {
			if (T* result = dynamic_cast<T*>(component.get())) {
				results.push_back(result);
			}
		}
		for (const std::unique_ptr<Component>& component : pendingComponents_) {
			if (T* result = dynamic_cast<T*>(component.get())) {
				results.push_back(result);
			}
		}
		return results;
	}

	GameObjectId GetId() const { return id_; }
	const std::string& GetName() const { return name_; }
	void SetName(std::string name) { name_ = std::move(name); }

	TransformComponent& GetTransform();
	const TransformComponent& GetTransform() const;

	bool IsActive() const { return active_; }
	void SetActive(bool active) { active_ = active; }

	void Start();
	void Update(float deltaTime);
	void FixedUpdate(float fixedDeltaTime);
	void LateUpdate(float deltaTime);

	void Destroy() { destroyRequested_ = true; }
	bool IsDestroyRequested() const { return destroyRequested_; }

private:
	void AttachComponent(std::unique_ptr<Component> component);
	void FlushPendingComponents();
	void DetachAllComponents();

	GameObjectId id_ = kInvalidGameObjectId;
	std::string name_;
	std::vector<std::unique_ptr<Component>> components_;
	std::vector<std::unique_ptr<Component>> pendingComponents_;
	TransformComponent* transform_ = nullptr;
	bool active_ = true;
	bool destroyRequested_ = false;
	bool iteratingComponents_ = false;
};

} // namespace TakeC
