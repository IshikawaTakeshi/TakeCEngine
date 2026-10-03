#pragma once

namespace TakeC {

class GameObject;

/// <summary>
/// コンポーネントのフレーム更新に渡す共通情報
/// </summary>
struct ComponentUpdateContext final {
	float deltaTime = 0.0f;
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
