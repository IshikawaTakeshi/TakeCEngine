#include "BoxCollider.h"
#include "3d/Object3dCommon.h"
#include "3d/Model.h"
#include "engine/math/MatrixMath.h"
#include "engine/math/Vector3Math.h"
#include "Collision/SphereCollider.h"
#include "Base/ModelManager.h"
#include "Base/DirectXCommon.h"
#include "Camera/CameraManager.h"
#include "engine/Entity/GameCharacter.h"
#include "TakeCFrameWork.h"

#include <cmath>

//=============================================================================
// 初期化
//=============================================================================

void BoxCollider::Initialize(TakeC::DirectXCommon* dxCommon, Object3d* collisionObject) {

	dxCommon_ = dxCommon;

	//半分の大きさをセット
	rotateMatrix_ = MatrixMath::MakeRotateMatrix(collisionObject->GetRotate());

	// OBB情報のセット
	obb_.center = collisionObject->GetCenterPosition();
	obb_.axis[0].x = rotateMatrix_.m[0][0];
	obb_.axis[0].y = rotateMatrix_.m[0][1];
	obb_.axis[0].z = rotateMatrix_.m[0][2];

	obb_.axis[1].x = rotateMatrix_.m[1][0];
	obb_.axis[1].y = rotateMatrix_.m[1][1];
	obb_.axis[1].z = rotateMatrix_.m[1][2];

	obb_.axis[2].x = rotateMatrix_.m[2][0];
	obb_.axis[2].y = rotateMatrix_.m[2][1];
	obb_.axis[2].z = rotateMatrix_.m[2][2];

	obb_.halfSize = halfSize_;

	//CPUで動かす用のTransform
	transform_ = {
		collisionObject->GetScale(),
		collisionObject->GetRotate(),
		obb_.center + offset_
	};

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
void BoxCollider::Update(Object3d* collisionObject) {
	minAxis_ = { 0.0f,0.0f,0.0f };
	minPenetration_ = 0.0f;

	transform_ = collisionObject->GetTransform();
	obb_.center = collisionObject->GetCenterPosition() + offset_;

	// スケール行列の更新
	Matrix4x4 scaleMat = MatrixMath::MakeScaleMatrix(transform_.scale);

	// 回転行列の更新
	rotateMatrix_ = MatrixMath::MakeRotateMatrix(transform_.rotate);

	// OBB情報のセット
	obb_.axis[0].x = rotateMatrix_.m[0][0];
	obb_.axis[0].y = rotateMatrix_.m[0][1];
	obb_.axis[0].z = rotateMatrix_.m[0][2];

	obb_.axis[1].x = rotateMatrix_.m[1][0];
	obb_.axis[1].y = rotateMatrix_.m[1][1];
	obb_.axis[1].z = rotateMatrix_.m[1][2];

	obb_.axis[2].x = rotateMatrix_.m[2][0];
	obb_.axis[2].y = rotateMatrix_.m[2][1];
	obb_.axis[2].z = rotateMatrix_.m[2][2];

	// 平行移動行列の更新
	Matrix4x4 translateMat = MatrixMath::MakeTranslateMatrix(obb_.center);

	//アフィン行列の更新
	worldMatrix_ = scaleMat * rotateMatrix_ * translateMat;
}

//=============================================================================
// ImGuiによるパラメータ調整
//=============================================================================
void BoxCollider::UpdateImGui([[maybe_unused]]const std::string& name) {
#if defined(_DEBUG) || defined(_DEVELOP)
	std::string windowName = "BoxCollider" + name;
	std::vector<std::string> surfaceTypes = { "FLOOR", "WALL", "CEILING" };
	if (ImGui::TreeNode("BoxCollider")) {
		ImGui::DragFloat3("HalfSize", &halfSize_.x, 0.01f);
		ImGui::Text("OBB Center: %.2f, %.2f, %.2f", obb_.center.x, obb_.center.y, obb_.center.z);
		ImGui::Text("SurfaceType: %s", surfaceTypes[static_cast<int>(surfaceType_)].c_str());
		ImGui::Text("MinAxis: %.2f, %.2f, %.2f", minAxis_.x, minAxis_.y, minAxis_.z);
		ImGui::TreePop();
	}
#endif // _DEBUG
}
//=============================================================================
// 衝突判定
//=============================================================================

bool BoxCollider::CheckCollision(Collider* other) {

	bool result = false;

	if (BoxCollider* box = dynamic_cast<BoxCollider*>(other)) {
		result = CheckCollisionOBB(box);
		if (result == true) {
			surfaceType_ = CheckSurfaceType(obb_.axis, minAxis_);
		}
		return result;
	}
	if (SphereCollider* sphere = dynamic_cast<SphereCollider*>(other)) {
		result = sphere->CheckCollisionOBB(this);
		return result;
	}
	return false;
}

//=============================================================================
// レイとの衝突判定
//=============================================================================
bool BoxCollider::Intersects(const Ray& ray, RayCastHit& outHit) {
	// OBBとレイの衝突判定
	Vector3 invDir = { 1.0f / ray.direction.x, 1.0f / ray.direction.y, 1.0f / ray.direction.z };
	Vector3 tMin = (obb_.center - obb_.halfSize) - ray.origin;
	Vector3 tMax = (obb_.center + obb_.halfSize) - ray.origin;
	float t1 = tMin.x * invDir.x;
	float t2 = tMax.x * invDir.x;
	float t3 = tMin.y * invDir.y;
	float t4 = tMax.y * invDir.y;
	float t5 = tMin.z * invDir.z;
	float t6 = tMax.z * invDir.z;
	float tMinX = std::min(t1, t2);
	float tMaxX = std::max(t1, t2);
	float tMinY = std::min(t3, t4);
	float tMaxY = std::max(t3, t4);
	float tMinZ = std::min(t5, t6);
	float tMaxZ = std::max(t5, t6);
	if (tMinX > tMaxY || tMinY > tMaxX || tMinX > tMaxZ || tMinZ > tMaxX) {
		return false; // レイとOBBは交差しない
	}
	float tNear = std::max(tMinX, std::max(tMinY, tMinZ));
	float tFar = std::min(tMaxX, std::min(tMaxY, tMaxZ));
	if (tNear > tFar || tFar < 0) {
		return false; // レイとOBBは交差しない
	}
	outHit.distance = tNear;
	outHit.position = ray.origin + ray.direction * tNear;
	outHit.normal = (outHit.position - obb_.center).Normalize(); // 衝突点からOBBの中心への法線ベクトル
	outHit.hitCollider = this; // 衝突したコライダーを設定
	return true; // レイとOBBは交差する
}

//=============================================================================
// 当たり判定範囲の描画
//=============================================================================

bool BoxCollider::IntersectsSphere(const Ray& ray, float radius, RayCastHit& outHit) {
	// 1. RayをOBBのローカル座標系（箱が回転していない状態）に変換する
	Vector3 diff = ray.origin - obb_.center;

	// ローカルでのRayの始点
	Vector3 localOrigin = {
		diff.Dot(obb_.axis[0]),
		diff.Dot(obb_.axis[1]),
		diff.Dot(obb_.axis[2])
	};

	// ローカルでのRayの方向
	Vector3 localDirection = {
		ray.direction.Dot(obb_.axis[0]),
		ray.direction.Dot(obb_.axis[1]),
		ray.direction.Dot(obb_.axis[2])
	};

	// 2. AABB vs Ray の判定を行う（ただし箱のサイズを球の半径分拡張する）
	// これにより、箱の角も「丸く」ではなく「四角く」拡張される近似になるが、
	// すり抜け防止としては十分かつ計算が高速。

	Vector3 expandedHalfSize = obb_.halfSize + Vector3{radius, radius, radius};

	// スラブ法による判定
	// ゼロ除算対策を含めた逆数計算
	Vector3 invDir;
	invDir.x = (std::abs(localDirection.x) < 1e-6f) ? 1e20f : 1.0f / localDirection.x;
	invDir.y = (std::abs(localDirection.y) < 1e-6f) ? 1e20f : 1.0f / localDirection.y;
	invDir.z = (std::abs(localDirection.z) < 1e-6f) ? 1e20f : 1.0f / localDirection.z;

	Vector3 tMin = (-expandedHalfSize - localOrigin) * invDir;
	Vector3 tMax = (expandedHalfSize - localOrigin) * invDir;

	// 各軸の min/max を整理
	Vector3 tNear = { std::min(tMin.x, tMax.x), std::min(tMin.y, tMax.y), std::min(tMin.z, tMax.z) };
	Vector3 tFar  = { std::max(tMin.x, tMax.x), std::max(tMin.y, tMax.y), std::max(tMin.z, tMax.z) };

	// 最も遅い突入時間と、最も早い脱出時間を求める
	float tEnter = std::max(std::max(tNear.x, tNear.y), tNear.z);
	float tExit  = std::min(std::min(tFar.x, tFar.y), tFar.z);

	// 衝突していない条件
	if (tEnter > tExit || tExit < 0 || tEnter > ray.distance) {
		return false;
	}

	// 衝突情報の構築
	if (tEnter < 0) tEnter = 0; // 既に内部にいる場合

	outHit.isHit = true;
	outHit.distance = tEnter;
	// ワールド座標での衝突点を計算
	outHit.position = ray.origin + ray.direction * tEnter;
	outHit.hitCollider = this;

	// 法線の計算（ローカル座標系でどの面に当たったか）
	Vector3 localHitPos = localOrigin + localDirection * tEnter;
	Vector3 localNormal = { 0, 0, 0 };

	// 許容誤差
	const float epsilon = 0.001f;

	// どの面に一番近いかで法線を決定
	if (std::abs(localHitPos.x - expandedHalfSize.x) < epsilon) localNormal = { 1, 0, 0 };
	else if (std::abs(localHitPos.x + expandedHalfSize.x) < epsilon) localNormal = { -1, 0, 0 };
	else if (std::abs(localHitPos.y - expandedHalfSize.y) < epsilon) localNormal = { 0, 1, 0 };
	else if (std::abs(localHitPos.y + expandedHalfSize.y) < epsilon) localNormal = { 0, -1, 0 };
	else if (std::abs(localHitPos.z - expandedHalfSize.z) < epsilon) localNormal = { 0, 0, 1 };
	else if (std::abs(localHitPos.z + expandedHalfSize.z) < epsilon) localNormal = { 0, 0, -1 };

	// ローカル法線をワールド法線へ変換
	outHit.normal = (obb_.axis[0] * localNormal.x + 
		obb_.axis[1] * localNormal.y + 
		obb_.axis[2] * localNormal.z).Normalize();

	return true;
}

bool BoxCollider::IntersectsCapsule(const Capsule& capsule, RayCastHit& outHit){
	// カプセルの半径分だけOBBを膨張させて、線分との判定に変換
	Vector3 expandedHalfSize = obb_.halfSize + Vector3(capsule.radius, capsule.radius, capsule.radius);

	// カプセルの軸をOBBのローカル座標系に変換
	Vector3 startLocal = WorldToLocal(capsule.start);
	Vector3 endLocal = WorldToLocal(capsule.end);

	// 膨張したAABBと線分の判定
	Vector3 direction = endLocal - startLocal;
	float length = Vector3Math::Length(direction);

	if (length < 0.0001f) {
		// 移動量がほぼ0の場合、点と膨張AABBの判定
		if (std::abs(startLocal.x) <= expandedHalfSize.x &&
			std::abs(startLocal.y) <= expandedHalfSize.y &&
			std::abs(startLocal.z) <= expandedHalfSize.z) {
			outHit.isHit = true;
			outHit.distance = 0.0f;
			outHit.position = capsule.start;
			outHit.hitCollider = this;
			CalculateBoxNormal(startLocal, expandedHalfSize, outHit.normal);
			return true;
		}
		return false;
	}

	Vector3 dir = direction / length;

	// スラブ法による線分とAABBの交差判定
	float tMin = 0.0f;
	float tMax = length;

	for (int i = 0; i < 3; ++i) {
		float d = (i == 0) ? dir.x : (i == 1) ? dir.y : dir.z;
		float origin = (i == 0) ? startLocal.x : (i == 1) ? startLocal.y : startLocal.z;
		float halfSize = (i == 0) ? expandedHalfSize.x : (i == 1) ? expandedHalfSize.y : expandedHalfSize.z;

		if (std::abs(d) < 1e-6f) {
			// 軸に平行な場合
			if (std::abs(origin) > halfSize) {
				return false;
			}
		}
		else {
			float t1 = (-halfSize - origin) / d;
			float t2 = (halfSize - origin) / d;

			if (t1 > t2) std::swap(t1, t2);

			tMin = std::max(tMin, t1);
			tMax = std::min(tMax, t2);

			if (tMin > tMax) {
				return false;
			}
		}
	}

	// 衝突情報の設定
	outHit.isHit = true;
	outHit.distance = tMin;

	// ワールド座標での衝突点
	Vector3 localHit = startLocal + dir * tMin;
	outHit.position = LocalToWorld(localHit);
	outHit.hitCollider = this;

	// 法線の計算
	CalculateBoxNormal(localHit, expandedHalfSize, outHit.normal);

	return true;
}

void BoxCollider::DrawCollider() {

	TakeCFrameWork::GetWireFrame()->DrawOBB(obb_, color_);
}

//=============================================================================
// SurfaceTypeの取得
//=============================================================================

SurfaceType BoxCollider::CheckSurfaceType(const Vector3* Axis, const Vector3& normal) const {
	float dotX = normal.Dot(Axis[0]);
	float dotY = normal.Dot(Axis[1]);
	float dotZ = normal.Dot(Axis[2]);

	float absDotX = std::fabs(dotX);
	float absDotY = std::fabs(dotY);
	float absDotZ = std::fabs(dotZ);

	// Y軸方向の衝突が最も大きい場合 (地面 or 天井)
	if (absDotY >= absDotX && absDotY >= absDotZ) {
		return dotY > 0 ? SurfaceType::TOP : SurfaceType::BOTTOM;
	}
	// X軸方向の衝突が最も大きい場合 (左右の壁)
	else if (absDotX >= absDotY && absDotX >= absDotZ) {
		return SurfaceType::WALL;
	}
	// Z軸方向の衝突が最も大きい場合 (前後の壁)
	else {
		return SurfaceType::WALL;
	}
}




//=============================================================================
// OBBとの衝突判定
//=============================================================================

bool BoxCollider::CheckCollisionOBB(BoxCollider* otherBox) {
	const Vector3* thisAxis = this->obb_.axis;
	const Vector3* otherAxis = otherBox->obb_.axis;

	//他OBBの中心から自OBBの中心へのベクトル
	Vector3 direction = otherBox->obb_.center - this->obb_.center;

	//他OBBの座標を自OBBのローカル座標系へ変換
	float rotation[3][3], absRotation[3][3];

	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			rotation[i][j] = thisAxis[i].Dot(otherAxis[j]);
			absRotation[i][j] = std::fabs(rotation[i][j]) + 1e-6f;
		}
	}

	// 中心間距離を自OBBのローカル座標系で表現
	float centerToCenter[3] = { direction.Dot(thisAxis[0]), direction.Dot(thisAxis[1]), direction.Dot(thisAxis[2]) };
	minPenetration_ = FLT_MAX;

	// 1. 自OBBの各軸を分離軸として判定
	for (int i = 0; i < 3; i++) {
		float selfProjection =
			obb_.halfSize.x * std::fabs(thisAxis[i].Dot(thisAxis[0])) +
			obb_.halfSize.y * std::fabs(thisAxis[i].Dot(thisAxis[1])) +
			obb_.halfSize.z * std::fabs(thisAxis[i].Dot(thisAxis[2]));

		float otherProjection =
			otherBox->obb_.halfSize.x * absRotation[i][0] +
			otherBox->obb_.halfSize.y * absRotation[i][1] +
			otherBox->obb_.halfSize.z * absRotation[i][2];

		// 他OBBの中心から自OBBの中心への距離
		float dist = std::fabs(centerToCenter[i]);

		// 分離軸としての判定
		if (dist > selfProjection + otherProjection) return false;
		// 貫通量の計算と最小貫通量の更新
		float penetration = (selfProjection + otherProjection) - dist;
		if (penetration < minPenetration_) {
			minPenetration_ = penetration;
			minAxis_ = thisAxis[i];

			// 中心間の方向ベクトルと軸の向きを合わせる
			if (centerToCenter[i] < 0) {
				minAxis_ = -minAxis_;
			}
		}
		
	}

	// 2. 他OBBの各軸を分離軸として判定
	for (int i = 0; i < 3; i++) {
		float selfProjection =
			obb_.halfSize.x * absRotation[0][i] +
			obb_.halfSize.y * absRotation[1][i] +
			obb_.halfSize.z * absRotation[2][i];

		float otherProjection =
			otherBox->obb_.halfSize.x * std::fabs(otherAxis[i].Dot(otherAxis[0])) +
			otherBox->obb_.halfSize.y * std::fabs(otherAxis[i].Dot(otherAxis[1])) +
			otherBox->obb_.halfSize.z * std::fabs(otherAxis[i].Dot(otherAxis[2]));

		// 他OBBの中心から自OBBの中心への距離
		float dist = std::fabs(direction.Dot(otherAxis[i]));
		// 分離軸としての判定
		if (dist > selfProjection + otherProjection) return false;

		// 貫通量の計算と最小貫通量の更新
		float penetration = (selfProjection + otherProjection) - dist;
		if (penetration < minPenetration_) {
			minPenetration_ = penetration;
			minAxis_ = otherAxis[i];
			// 中心間の方向ベクトルと軸の向きを合わせる
			if (direction.Dot(otherAxis[i]) < 0) {
				minAxis_ = -minAxis_;
			}
		}
		
	}

	// 3. OBBの交差軸 (Aの3軸 × Bの3軸) を分離軸として判定
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			Vector3 axis = thisAxis[i].Cross(otherAxis[j]); // 交差軸を求める

			// 交差軸がゼロベクトルならスキップ
			if (axis.Length() < 1e-6f) continue;

			axis = axis.Normalize();

			float selfProjection =
				obb_.halfSize.x * std::fabs(thisAxis[0].Dot(axis)) +
				obb_.halfSize.y * std::fabs(thisAxis[1].Dot(axis)) +
				obb_.halfSize.z * std::fabs(thisAxis[2].Dot(axis));

			float otherProjection =
				otherBox->obb_.halfSize.x * std::fabs(otherAxis[0].Dot(axis)) +
				otherBox->obb_.halfSize.y * std::fabs(otherAxis[1].Dot(axis)) +
				otherBox->obb_.halfSize.z * std::fabs(otherAxis[2].Dot(axis));

			// 交差軸に投影した中心間距離
			float dist = std::fabs(direction.Dot(axis));
			// 分離軸としての判定
			if (dist > selfProjection + otherProjection) return false;

			float penetration = (selfProjection + otherProjection) - dist;
			if (penetration < minPenetration_) {
				minPenetration_ = penetration;
				minAxis_ = axis;

				// 中心間の方向ベクトルと軸の向きを合わせる
				if (direction.Dot(axis) < 0) {
					minAxis_ = -minAxis_;
				}
			}
			
		}
	}

	// すべての軸で分離ができなかった場合、衝突
	//minAxisの正規化
	minAxis_ = minAxis_.Normalize();

	return true;
}

//ワールド座標の取得
Vector3 BoxCollider::GetWorldPos() {
	return obb_.center;
}

//半径の取得
void BoxCollider::SetHalfSize(const Vector3& halfSize) {
	halfSize_ = halfSize;
	obb_.halfSize = halfSize_;
}

//=============================================================================
// ワールド座標 -> ローカル座標変換
//=============================================================================
Vector3 BoxCollider::WorldToLocal(const Vector3& worldPos){
	Vector3 diff = worldPos - obb_.center;
	return Vector3(
		diff.Dot(obb_.axis[0]),
		diff.Dot(obb_.axis[1]),
		diff.Dot(obb_.axis[2])
	);
}

//=============================================================================
// ローカル座標 -> ワールド座標変換
//=============================================================================
Vector3 BoxCollider::LocalToWorld(const Vector3& localPos){
	return obb_.center +
		obb_.axis[0] * localPos.x +
		obb_.axis[1] * localPos.y +
		obb_.axis[2] * localPos.z;
}

//=============================================================================
// AABBのどの面に当たったかを判定して法線を設定する関数
//=============================================================================
void BoxCollider::CalculateBoxNormal(const Vector3& localHit, const Vector3& halfSize, Vector3& outNormal) {
	const float epsilon = 0.01f;
	Vector3 localNormal(0, 0, 0);

	if (std::abs(localHit.x - halfSize.x) < epsilon) localNormal = Vector3(1, 0, 0);
	else if (std::abs(localHit.x + halfSize.x) < epsilon) localNormal = Vector3(-1, 0, 0);
	else if (std::abs(localHit.y - halfSize.y) < epsilon) localNormal = Vector3(0, 1, 0);
	else if (std::abs(localHit.y + halfSize.y) < epsilon) localNormal = Vector3(0, -1, 0);
	else if (std::abs(localHit.z - halfSize.z) < epsilon) localNormal = Vector3(0, 0, 1);
	else if (std::abs(localHit.z + halfSize.z) < epsilon) localNormal = Vector3(0, 0, -1);

	// ローカル法線をワールド法線に変換
	outNormal = (obb_.axis[0] * localNormal.x +
		obb_.axis[1] * localNormal.y +
		obb_.axis[2] * localNormal.z).Normalize();
}