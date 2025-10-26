#include "EnemyInputProvider.h"
#include "engine/math/Vector3Math.h"
#include "engine/math/Quaternion.h"
#include "engine/base/TakeCFrameWork.h"
#include "application/Entity/Enemy/Enemy.h"

//=============================================================================
//　移動方向の取得
//=============================================================================
Vector3 EnemyInputProvider::GetMoveDirection() const {

	// 移動方向の計算
	// ターゲットの周囲を回るための目標座標を計算
	enemy_->SetOrbitAngle(enemy_->GetOrbitAngle() + enemy_->GetOrbitSpeed() * TakeCFrameWork::GetDeltaTime());

	if (enemy_->GetOrbitAngle() > 2 * std::numbers::pi_v<float>) {
		enemy_->SetOrbitAngle(enemy_->GetOrbitAngle() - (2 * std::numbers::pi_v<float>));
	}
	// ターゲットの周囲を回る座標を計算（XZ平面で円運動）
	Vector3 orbitPos;
	orbitPos.x = enemy_->GetFocusTargetPos().x + enemy_->GetOrbitRadius() * cos(enemy_->GetOrbitAngle());
	orbitPos.y = enemy_->GetFocusTargetPos().y; // 高さはターゲットに合わせる（必要に応じて調整）
	orbitPos.z = enemy_->GetFocusTargetPos().z + enemy_->GetOrbitRadius() * sin(enemy_->GetOrbitAngle());
	// 目的座標までの方向ベクトルを計算
	return Vector3Math::Normalize(orbitPos - enemy_->GetTransform().translate);
}


//=============================================================================
//　ジャンプ入力の取得
//=============================================================================
bool EnemyInputProvider::IsJumpRequested() const {
	return aiBrainSystem_->GetBestAction() == Action::FLOATING;
}

//=============================================================================
//　攻撃入力の取得
//=============================================================================
void EnemyInputProvider::RequestAttack() {
}

//=============================================================================
//　チャージ攻撃入力の取得
//=============================================================================
void EnemyInputProvider::RequestChargeAttack() {
}
