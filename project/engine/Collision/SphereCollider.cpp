#include "SphereCollider.h"
#include "BoxCollider.h"
#include "Model.h"
#include "ModelManager.h"
#include "CameraManager.h"
#include "DirectXCommon.h"
#include "MatrixMath.h"
#include "TakeCFrameWork.h"
#include <algorithm>

//=============================================================================
// 初期化
//=============================================================================

void SphereCollider::Initialize(DirectXCommon* dxCommon, Object3d* collisionObject) {

	dxCommon_ = dxCommon;

	transform_.translate = collisionObject->GetCenterPosition();
	if (radius_ == 0.0f) {

		radius_ = 1.0f;
	}

	color_ = { 1.0f,1.0f,1.0f,1.0f };

	//CPUで動かす用のTransform
	transform_ = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };

	//アフィン行列
	worldMatrix_ = MatrixMath::MakeAffineMatrix(
		transform_.scale,
		transform_.rotate,
		transform_.translate
	);

	//カメラのセット
	camera_ = CameraManager::GetInstance()->GetActiveCamera();
}

//=============================================================================
// 更新処理
//=============================================================================

void SphereCollider::Update(Object3d* collisionObject) {

	transform_ = collisionObject->GetTransform();
	transform_.translate = collisionObject->GetCenterPosition();

	//アフィン行列の更新
	worldMatrix_ = MatrixMath::MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);
}

void SphereCollider::UpdateImGui([[maybe_unused]]const std::string& name) {
#ifdef _DEBUG
	std::string windowName = "SphereCollider" + name;
	if (ImGui::TreeNode("SphereCollider")) {
		ImGui::DragFloat("Radius", &radius_, 0.01f);
		ImGui::Text("Position: %.2f, %.2f, %.2f", transform_.translate.x, transform_.translate.y, transform_.translate.z);
		ImGui::TreePop();
	}
#endif // _DEBUG
}

//=============================================================================
// 衝突判定
//=============================================================================

bool SphereCollider::CheckCollision(Collider* other) {

	// OBBとの衝突処理
	if (BoxCollider* box = dynamic_cast<BoxCollider*>(other)) {
		return CheckCollisionOBB(box);
	}
	// Sphereとの衝突処理
	if (SphereCollider* sphere = dynamic_cast<SphereCollider*>(other)) {
		return CheckCollisionSphere(sphere);
	}

	return false;
}

//=============================================================================
// 当たり判定範囲の描画
//=============================================================================

void SphereCollider::DrawCollider() {


	TakeCFrameWork::GetWireFrame()->DrawSphere(transform_.translate, radius_, color_);
}

//=============================================================================
// レイとの衝突判定
//=============================================================================

bool SphereCollider::Intersects(const Ray& ray, RayCastHit& outHit) {
	Vector3 oc = ray.origin - transform_.translate; // 球の中心からレイの始点までのベクトル
	float a = ray.direction.Dot(ray.direction);
	float b = 2.0f * oc.Dot(ray.direction);
	float c = oc.Dot(oc) - radius_ * radius_;
	float discriminant = b * b - 4 * a * c;

	if (discriminant < 0) return false; // レイと球は交差しない

	float sqrtDiscriminant = std::sqrt(discriminant);
	float t1 = (-b - sqrtDiscriminant) / (2.0f * a);
	float t2 = (-b + sqrtDiscriminant) / (2.0f * a);

	float t = t1;
	if (t <0 || t > ray.distance) t = t2; // t1が負またはrayの距離より大きい場合、t2を使用
	if (t < 0 || t > ray.distance) return false; // レイの範囲外

	//衝突情報設定
	outHit.isHit = true;
	outHit.distance = t;
	outHit.position = ray.origin + ray.direction * t;                     // 衝突位置
	outHit.normal = (outHit.position - transform_.translate).Normalize(); // 衝突面の法線ベクトル
	outHit.hitCollider = this;                                            // 衝突したコライダを設定
	
	// 衝突した
	return true;
}

//=============================================================================
// 衝突面のタイプを判定
//=============================================================================

SurfaceType SphereCollider::CheckSurfaceType() const {
	return SurfaceType::WALL; // 球は特定の面を持たないため、WALLを返す
}

Vector3 SphereCollider::GetWorldPos() {
	
	return transform_.translate;
}
//=============================================================================
// OBBとの衝突判定
//=============================================================================

bool SphereCollider::CheckCollisionOBB(BoxCollider* otherBox) {
	OBB obb = otherBox->GetOBB();
	Vector3 closestPoint = obb.center;

	for (int i = 0; i < 3; i++) {
		float distance = Vector3(transform_.translate - obb.center).Dot(obb.axis[i]);
		float clampedDistance = std::min(distance, obb.halfSize.Dot(obb.axis[i]));
		clampedDistance = std::max(-obb.halfSize.Dot(obb.axis[i]), clampedDistance);

		closestPoint = closestPoint + otherBox->GetOBB().axis[i] * clampedDistance;
	}

	Vector3 diff = transform_.translate - closestPoint;
	
	if (diff.Dot(diff) <= (radius_ * radius_)) {
		return true;
	}

	return false;
}

//=============================================================================
// Sphereとの衝突判定
//=============================================================================

bool SphereCollider::CheckCollisionSphere(SphereCollider* sphere) {
	Vector3 diff = transform_.translate - sphere->transform_.translate;
	float distanceSquared = diff.Dot(diff);
	float radiusSum = radius_ + sphere->radius_;
	return distanceSquared <= (radiusSum * radiusSum);
}

Vector3 SphereCollider::SphereCenterToOBBLocal(const OBB& obb, const Vector3& sphereCenter) {
	Vector3 local;
	Vector3 rel = sphereCenter - obb.center;
	local.x = rel.Dot(obb.axis[0]);
	local.y = rel.Dot(obb.axis[1]);
	local.z = rel.Dot(obb.axis[2]);
	return local;
}
