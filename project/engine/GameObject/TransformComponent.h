#pragma once

#include "engine/GameObject/Component.h"
#include "engine/Math/Matrix4x4.h"
#include "engine/Math/Quaternion.h"
#include "engine/Math/Vector3.h"

#include <vector>

namespace TakeC {

/// <summary>
/// GameObjectのローカル変換、親子関係、ワールド行列を管理するコンポーネントです。
/// </summary>
class TransformComponent final : public Component {
public:
	TransformComponent();
	~TransformComponent() override = default;

	const Vector3& GetLocalPosition() const { return localPosition_; }
	const Quaternion& GetLocalRotation() const { return localRotation_; }
	const Vector3& GetLocalScale() const { return localScale_; }

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

	Vector3 localPosition_ = { 0.0f, 0.0f, 0.0f };
	Quaternion localRotation_ = { 0.0f, 0.0f, 0.0f, 1.0f };
	Vector3 localScale_ = { 1.0f, 1.0f, 1.0f };
	TransformComponent* parent_ = nullptr;
	std::vector<TransformComponent*> children_;
	mutable Matrix4x4 worldMatrix_{};
	mutable bool worldMatrixDirty_ = true;
};

} // namespace TakeC
