#include "BoxCollider.h"
#include "3d/Object3dCommon.h"
#include "MatrixMath.h"
#include "SphereCollider.h"

#include <cmath>

void BoxCollider::Initialize(Object3dCommon* object3dCommon, const std::string& filePath) {
	collisionObject_ = std::make_unique<Object3d>();
	collisionObject_->Initialize(object3dCommon, filePath);

	rotateMatrix_ = MatrixMath::MakeRotateMatrix(collisionObject_->GetRotation());

	obb_.center = collisionObject_->GetPosition();
	obb_.axis[0].x = rotateMatrix_.m[0][0];
	obb_.axis[0].y = rotateMatrix_.m[0][1];
	obb_.axis[0].z = rotateMatrix_.m[0][2];

	obb_.axis[1].x = rotateMatrix_.m[1][0];
	obb_.axis[1].y = rotateMatrix_.m[1][1];
	obb_.axis[1].z = rotateMatrix_.m[1][2];

	obb_.axis[2].x = rotateMatrix_.m[2][0];
	obb_.axis[2].y = rotateMatrix_.m[2][1];
	obb_.axis[2].z = rotateMatrix_.m[2][2];

	obb_.halfSize = collisionObject_->GetScale() / 2.0f;
}
//=============================================================================
// OBBの衝突判定
//=============================================================================

bool BoxCollider::CheckCollision(Collider* other) {

	if (BoxCollider* box = dynamic_cast<BoxCollider*>(other)) {
		return CheckCollisionOBB(box);
	}
	if (SphereCollider* sphere = dynamic_cast<SphereCollider*>(other)) {
		return sphere->CheckCollisionOBB(this);
	}
	return false;
}

bool BoxCollider::CheckCollisionOBB(BoxCollider* otherBox) {
	const Vector3* thisAxis = this->obb_.axis;
	const Vector3* otherAxis = otherBox->obb_.axis;

	//他OBBの中心から自OBBの中心へのベクトル
	Vector3 T = otherBox->obb_.center - this->obb_.center;

	//他OBBの座標を自OBBのローカル座標系へ変換
	float rotation[3][3], absRotation[3][3];

	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			rotation[i][j] = thisAxis[i].Dot(otherAxis[j]);
			absRotation[i][j] = std::fabs(rotation[i][j]) + 1e-6f;
		}
	}

	// 中心間距離を自OBBのローカル座標系で表現
	float centerToCenter[3](T.Dot(thisAxis[0]), T.Dot(thisAxis[1]), T.Dot(thisAxis[2]));


	// 1.自OBBの各軸を分離軸として判定
	for (int i = 0; i < 3; i++) {
		float selfProjection = obb_.halfSize.x * std::fabs(thisAxis[i].x) + obb_.halfSize.y * std::fabs(thisAxis[i].y) + obb_.halfSize.z * std::fabs(thisAxis[i].z);
		float otherProjection = otherBox->obb_.halfSize.x * absRotation[i][0] + otherBox->obb_.halfSize.y * absRotation[i][1] + otherBox->obb_.halfSize.z * absRotation[i][2];
		if (std::fabs(centerToCenter[i]) > selfProjection + otherProjection) return false;
	}

	// 2. 他OBBの各軸を分離軸として判定
	for (int i = 0; i < 3; i++) {
		float selfProjection = obb_.halfSize.x * absRotation[0][i] + obb_.halfSize.y * absRotation[1][i] + obb_.halfSize.z * absRotation[2][i];
		float otherProjection = otherBox->obb_.halfSize.x * std::fabs(otherAxis[i].x) + otherBox->obb_.halfSize.y * std::fabs(otherAxis[i].y) + otherBox->obb_.halfSize.z * std::fabs(otherAxis[i].z);
		if (std::fabs(T.Dot(otherAxis[i])) > selfProjection + otherProjection) return false;
	}

	// 衝突がある場合trueを返す
	return true;
}