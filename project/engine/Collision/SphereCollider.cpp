#include "SphereCollider.h"
#include "engine/Base/TakeCFrameWork.h"
#include "engine/Base/ModelManager.h"
#include "engine/Base/DirectXCommon.h"
#include "engine/Collision/BoxCollider.h"
#include "engine/3d/Model.h"
#include "engine/Camera/CameraManager.h"
#include "engine/math/MatrixMath.h"
#include "engine/math/Vector3Math.h"
#include <algorithm>

//=============================================================================
// 初期化
//=============================================================================

void SphereCollider::Initialize(TakeC::DirectXCommon* dxCommon, Object3d* collisionObject) {

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
	camera_ = TakeC::CameraManager::GetInstance().GetActiveCamera();
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

	// ワイヤーフレームの描画
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
// 球との衝突判定
//=============================================================================
bool SphereCollider::IntersectsSphere(const Ray& ray, float radius, RayCastHit& outHit) {

	// 相手の半径分だけ、自分の当たり判定を大きくしてRayCastするのと同じ計算
	float totalRadius = radius_ + radius;

	Vector3 oc = ray.origin - transform_.translate;
	float a = ray.direction.Dot(ray.direction);
	float b = 2.0f * oc.Dot(ray.direction);
	float c = oc.Dot(oc) - totalRadius * totalRadius; // ここで半径を合成
	float discriminant = b * b - 4 * a * c;

	if (discriminant < 0) return false;

	float sqrtDiscriminant = std::sqrt(discriminant);
	float t1 = (-b - sqrtDiscriminant) / (2.0f * a);
	float t2 = (-b + sqrtDiscriminant) / (2.0f * a);

	float t = t1;
	if (t < 0 || t > ray.distance) t = t2;
	if (t < 0 || t > ray.distance) return false;

	outHit.isHit = true;
	outHit.distance = t;
	outHit.position = ray.origin + ray.direction * t;
	outHit.normal = (outHit.position - transform_.translate).Normalize();
	outHit.hitCollider = this;

	return true;
}

//=============================================================================
// カプセルとの衝突判定
//=============================================================================
bool SphereCollider::IntersectsCapsule(const Capsule& capsule, RayCastHit& outHit) {
	// カプセルの軸ベクトル
	Vector3 capsuleAxis = capsule.end - capsule.start;
	float capsuleLength = Vector3Math::Length(capsuleAxis);

	// カプセルが点の場合（移動量がほぼ0）
	if (capsuleLength < 0.0001f) {
		// 単純な球同士の判定
		float totalRadius = radius_ + capsule.radius;
		Vector3 diff = transform_.translate - capsule.start;
		float distSq = Vector3Math::LengthSq(diff);

		if (distSq <= totalRadius * totalRadius) {
			outHit.isHit = true;
			outHit.distance = 0.0f;
			outHit.position = capsule.start;
			outHit.normal = Vector3Math::Normalize(diff);
			outHit.hitCollider = this;
			return true;
		}
		return false;
	}

	Vector3 capsuleDir = capsuleAxis / capsuleLength;

	// 球の中心からカプセル軸上の最近点を求める
	Vector3 toSphere = transform_.translate - capsule.start;
	float t = Vector3Math::Dot(toSphere, capsuleDir);

	// tをカプセルの範囲[0, capsuleLength]にクランプ
	t = std::max(0.0f, std::min(t, capsuleLength));

	// カプセル軸上の最近点
	Vector3 closestPoint = capsule.start + capsuleDir * t;

	// 最近点から球の中心までの距離
	Vector3 diff = transform_.translate - closestPoint;
	float distSq = Vector3Math::LengthSq(diff);

	// 衝突判定（カプセルの半径 + 球の半径）
	float totalRadius = radius_ + capsule.radius;

	if (distSq <= totalRadius * totalRadius) {
		float dist = std::sqrt(distSq);

		outHit.isHit = true;
		// カプセルの開始点からの距離（衝突が発生した位置のパラメータt）
		outHit.distance = t;
		// 衝突点はカプセル表面上の点
		outHit.position = closestPoint + Vector3Math::Normalize(diff) * capsule.radius;
		// 法線は球の中心からの方向
		outHit.normal = (dist > 0.0001f) ? (diff / dist) : Vector3(0, 1, 0);
		outHit.hitCollider = this;

		return true;
	}

	return false;
}

//=============================================================================
// 衝突面のタイプを判定
//=============================================================================

SurfaceType SphereCollider::CheckSurfaceType() const {
	return SurfaceType::WALL; // 球は特定の面を持たないため、WALLを返す
}

//=============================================================================
// OBBとの衝突判定
//=============================================================================

bool SphereCollider::CheckCollisionOBB(BoxCollider* otherBox) {
	OBB obb = otherBox->GetOBB();
	Vector3 closestPoint = obb.center;

	// 各軸に沿って最近接点を計算
	for (int i = 0; i < 3; i++) {
		float distance = Vector3(transform_.translate - obb.center).Dot(obb.axis[i]);
		float clampedDistance = std::min(distance, obb.halfSize.Dot(obb.axis[i]));
		clampedDistance = std::max(-obb.halfSize.Dot(obb.axis[i]), clampedDistance);

		closestPoint = closestPoint + otherBox->GetOBB().axis[i] * clampedDistance;
	}

	// 球の中心と最近接点の距離を計算
	Vector3 diff = transform_.translate - closestPoint;
	
	// 衝突判定
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

//=============================================================================
// OBBのローカル座標系への変換
//=============================================================================
Vector3 SphereCollider::SphereCenterToOBBLocal(const OBB& obb, const Vector3& sphereCenter) {
	Vector3 local;
	Vector3 rel = sphereCenter - obb.center;
	local.x = rel.Dot(obb.axis[0]);
	local.y = rel.Dot(obb.axis[1]);
	local.z = rel.Dot(obb.axis[2]);
	return local;
}


Vector3 SphereCollider::GetWorldPos() {
	// ワールド座標の取得	
	return transform_.translate;
}
