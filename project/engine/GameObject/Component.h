#pragma once

#include <cstdint>

namespace TakeC {

class GameObject;
class GameObjectManager;
class EngineServices;

/// <summary>
/// Componentの更新順序を決めるフェーズです。
/// </summary>
enum class ComponentPhase {
	Perception,
	Decision,
	Navigation,
	Simulation,
	Presentation
};

/// <summary>
/// コンポーネントのフレーム更新に渡す共通情報です。
/// 単独GameObjectの更新ではworldとservicesはnullptrです。
/// </summary>
struct ComponentUpdateContext final {
	float deltaTime = 0.0f;
	/// FixedUpdate、Update、LateUpdateで共有する1始まりのフレーム番号です。
	std::uint64_t frameIndex = 0;
	GameObjectManager* world = nullptr;
	EngineServices* services = nullptr;
};

/// <summary>
/// GameObjectへ追加できる全コンポーネントの共通基底クラス
/// </summary>
class Component {
public:
	Component() = default;
	virtual ~Component() = default;

	Component(const Component&) = delete;
	Component& operator=(const Component&) = delete;
	Component(Component&&) = delete;
	Component& operator=(Component&&) = delete;

	GameObject& GetOwner();
	const GameObject& GetOwner() const;

	bool IsEnabled() const { return enabled_; }
	void SetEnabled(bool enabled) { enabled_ = enabled; }
	bool HasStarted() const { return started_; }
	/// <summary>既定はSimulation。同じフェーズ内では追加順に更新されます。</summary>
	virtual ComponentPhase GetUpdatePhase() const noexcept { return ComponentPhase::Simulation; }

protected:
	virtual void OnAttach() {}
	virtual void Start() {}
	virtual void Update([[maybe_unused]] const ComponentUpdateContext& context) {}
	virtual void FixedUpdate([[maybe_unused]] const ComponentUpdateContext& context) {}
	virtual void LateUpdate([[maybe_unused]] const ComponentUpdateContext& context) {}
	virtual void OnDetach() {}

private:
	friend class GameObject;

	void Attach(GameObject& owner);
	void Detach();
	void StartIfNeeded();
	void UpdateInternal(const ComponentUpdateContext& context);
	void FixedUpdateInternal(const ComponentUpdateContext& context);
	void LateUpdateInternal(const ComponentUpdateContext& context);

	GameObject* owner_ = nullptr;
	bool enabled_ = true;
	bool started_ = false;
};

} // namespace TakeC
