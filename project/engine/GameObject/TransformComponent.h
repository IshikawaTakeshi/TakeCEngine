#pragma once

#include "engine/GameObject/Component.h"
#include "engine/3d/Transform.h"

#include <vector>

namespace TakeC {

/// <summary>
/// GameObjectのローカル変換、親子関係、ワールド行列を管理するコンポーネントです。
/// </summary>
class TransformComponent final : public Component {
public:
	TransformComponent();
	~TransformComponent() override = default;

	const Vector3& GetLocalPosition() const { return localTransform_.translate; }
	const Quaternion& GetLocalRotation() const { return localTransform_.rotate; }
	const Vector3& GetLocalScale() const { return localTransform_.scale; }

	void SetLocalPosition(const Vector3& position);
	void SetLocalRotation(const Quaternion& rotation);
	void SetLocalScale(const Vector3& scale);

	TransformComponent* GetParent() { return parent_; }
	const TransformComponent* GetParent() const { return parent_; }
	const std::vector<TransformComponent*>& GetChildren() const { return children_; }
	void SetParent(TransformComponent* parent);

	const Matrix4x4& GetWorldMatrix() const;
	Vector3 GetWorldPosition() const;

protected:
	void OnDetach() override;

private:
	void MarkDirty();
	void UpdateWorldMatrix() const;
	bool WouldCreateCycle(const TransformComponent* parent) const;

	QuaternionTransform localTransform_{
		{ 1.0f, 1.0f, 1.0f },
		QuaternionMath::Identity(),
		{ 0.0f, 0.0f, 0.0f }
	};
	TransformComponent* parent_ = nullptr;
	std::vector<TransformComponent*> children_;
	mutable Matrix4x4 worldMatrix_{};
	mutable bool worldMatrixDirty_ = true;
};

} // namespace TakeC
