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
/// コンポーネントのフレーム更新に渡す共通情報
/// </summary>
struct ComponentUpdateContext final {
	float deltaTime = 0.0f;
	/// FixedUpdate、Update、LateUpdateで共有するフレーム番号
	std::uint64_t frameIndex = 0;
	GameObjectManager* world = nullptr;
	EngineServices* services = nullptr;
};

/// <summary>
/// GameObjectへ追加できる全コンポーネントの共通基底クラス
/// </summary>
class Component {
public:

	/// コンストラクタ・デストラクタ
	Component() = default;
	virtual ~Component() = default;

	/// コピー禁止・ムーブ禁止
	Component(const Component&) = delete;
	Component& operator=(const Component&) = delete;
	Component(Component&&) = delete;
	Component& operator=(Component&&) = delete;

public:

	/// 親ゲームオブジェクトの取得
	GameObject& GetOwner();
	const GameObject& GetOwner() const;
	/// コンポーネントの有効状態の取得
	bool IsEnabled() const { return enabled_; }
	/// コンポーネントが開始されたかどうかの取得
	bool HasStarted() const { return started_; }
	/// 更新フェーズの取得
	virtual ComponentPhase GetUpdatePhase() const noexcept { return ComponentPhase::Simulation; }

	/// コンポーネントの有効状態の設定
	void SetEnabled(bool enabled) { enabled_ = enabled; }
protected:

	///　アタッチ処理
	virtual void OnAttach() {}
	/// デタッチ処理
	virtual void OnDetach() {}

	/// 初期化
	virtual void Start() {}
	/// 更新処理
	virtual void Update([[maybe_unused]] const ComponentUpdateContext& context) {}
	/// 固定更新処理
	virtual void FixedUpdate([[maybe_unused]] const ComponentUpdateContext& context) {}
	/// 遅延更新処理
	virtual void LateUpdate([[maybe_unused]] const ComponentUpdateContext& context) {}

private:

	friend class GameObject;

	// ゲームオブジェクトへのアタッチ、デタッチ
	void Attach(GameObject& owner);
	void Detach();

	// 初期化処理が必要な場合に呼び出す
	void StartIfNeeded();
	// 内部更新処理
	void UpdateInternal(const ComponentUpdateContext& context);
	// 内部固定更新処理
	void FixedUpdateInternal(const ComponentUpdateContext& context);
	// 内部遅延更新処理
	void LateUpdateInternal(const ComponentUpdateContext& context);

	// 所有するゲームオブジェクトへのポインタ
	GameObject* owner_ = nullptr;
	// コンポーネントの有効状態
	bool enabled_ = true;
	// 初期化処理が呼ばれたかどうかのフラグ
	bool started_ = false;
};

} // namespace TakeC
