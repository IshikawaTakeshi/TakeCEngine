#pragma once

#include "engine/GameObject/Component.h"

#include <array>
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
/// 複数のコンポーネントを組み合わせてゲーム内のオブジェクトを構成させるクラス
/// </summary>
class GameObject final {
public:

	/// コンストラクタ、デストラクタ
	explicit GameObject(std::string name = "GameObject");
	~GameObject();

	/// コピー禁止、ムーブ禁止
	GameObject(const GameObject&) = delete;
	GameObject& operator=(const GameObject&) = delete;
	GameObject(GameObject&&) = delete;
	GameObject& operator=(GameObject&&) = delete;

	/// コンポーネントの追加処理
	template<class T, class... Args>
	requires std::derived_from<T, Component> && (!std::same_as<T, TransformComponent>)
	T& AddComponent(Args&&... args);

	/// コンポーネントの取得
	template<class T>
	requires std::derived_from<T, Component>
	T* GetComponent();
	template<class T>
	requires std::derived_from<T, Component>
	const T* GetComponent() const;
	template<class T>
	requires std::derived_from<T, Component>
	std::vector<T*> GetComponents();

public:

	/// ゲームオブジェクトIDの取得
	GameObjectId GetId() const { return id_; }
	/// オブジェクト名の取得
	const std::string& GetName() const { return name_; }
	/// GameObjectManagerの取得
	GameObjectManager* GetWorld() { return manager_; }
	const GameObjectManager* GetWorld() const { return manager_; }
	/// EngineServicesの取得
	EngineServices* GetServices();
	const EngineServices* GetServices() const;
	/// TransformComponentの取得
	TransformComponent& GetTransform();
	const TransformComponent& GetTransform() const;

	/// GameObjectの有効状態の取得と設定
	bool IsActive() const { return active_; }
	void SetActive(bool active) { active_ = active; }

	/// GameObjectの名前の設定
	void SetName(std::string name) { name_ = std::move(name); }
	
	/// 初期化処理
	void Initialize();
	/// 更新処理
	void Update(float deltaTime);

	/// 固定更新処理
	void FixedUpdate(float fixedDeltaTime);
	///　遅延更新処理
	void LateUpdate(float deltaTime);

	void Destroy() { destroyRequested_ = true; }
	bool IsDestroyRequested() const { return destroyRequested_; }

private:
	friend class GameObjectManager;

	enum class UpdateKind { Update, FixedUpdate, LateUpdate };
	static constexpr std::array kUpdatePhases{
		ComponentPhase::Perception,
		ComponentPhase::Decision,
		ComponentPhase::Navigation,
		ComponentPhase::Simulation,
		ComponentPhase::Presentation
	};
	void RunPhasedUpdate(UpdateKind kind, float deltaTime);
	void BeginComponentUpdate();
	void UpdatePhase(ComponentPhase phase, UpdateKind kind, const ComponentUpdateContext& context);
	void EndComponentUpdate();
	void AttachComponent(std::unique_ptr<Component> component);
	void FlushPendingComponents();
	void DetachAllComponents();

	GameObjectId id_ = kInvalidGameObjectId;
	std::string name_;
	std::vector<std::unique_ptr<Component>> components_;
	std::vector<std::unique_ptr<Component>> pendingComponents_;
	GameObjectManager* manager_ = nullptr;
	TransformComponent* transform_ = nullptr;
	// Standaloneモードでのフレーム番号。Update、FixedUpdate、LateUpdateで共有される
	std::uint64_t standaloneFrameIndex_ = 0;
	// Standaloneモードでのフレーム番号が進んだかどうか
	bool standaloneFrameOpen_ = false;
	// StandaloneモードでUpdateが呼ばれたフレームで、フレーム番号を1回だけ進めるためのフラグ
	bool standaloneUpdatedThisFrame_ = false;
	// GameObjectが有効かどうか
	bool active_ = true;
	// GameObjectの破棄が要求されたか
	bool destroyRequested_ = false;
	// コンポーネントの更新中かどうか
	bool iteratingComponents_ = false;
};

//-----------------------------------------------------------------
// コンポーネントの追加
//-----------------------------------------------------------------
template<class T, class ...Args>
requires std::derived_from<T, Component> && (!std::same_as<T, TransformComponent>)
inline T& TakeC::GameObject::AddComponent(Args && ...args) {
	auto component = std::make_unique<T>(std::forward<Args>(args)...);
	T& reference = *component;
	AttachComponent(std::move(component));
	return reference;
}

//-----------------------------------------------------------------
// コンポーネントの取得
//-----------------------------------------------------------------
template<class T>
requires std::derived_from<T, Component>
inline T* GameObject::GetComponent() {
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
inline const T* GameObject::GetComponent() const {
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
inline std::vector<T*> GameObject::GetComponents() {
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


} // namespace TakeC
