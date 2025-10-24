#pragma once
#include "application/Weapon/BaseWeapon.h"

//============================================================================
// VerticalMissileLauncher class
//============================================================================
class VerticalMissileLauncher : public BaseWeapon {
public:

	VerticalMissileLauncher() = default;
	~VerticalMissileLauncher() = default;

	//=====================================================================
	// functions
	//=====================================================================

	// 初期化処理
	void Initialize(Object3dCommon* object3dCommon, BulletManager* bulletManager, const std::string& filePath) override;
	// 更新処理
	void Update() override;
	// ImGuiの更新
	void UpdateImGui() override;
	// 描画処理
	void Draw() override;
	// 攻撃処理
	void Attack() override;

	// チャージ処理
	virtual void Charge(float deltaTime) override;
	// チャージ攻撃実行
	virtual void ChargeAttack() override;

	//----- setter ---------------------------
	
	// 武器タイプの取得
	void SetOwnerObject(GameCharacter* owner) override;
	// ターゲットの設定
	void SetTarget(const Vector3& targetPos) override { targetPos_ = targetPos; }

	//チャージ攻撃可能か
	bool IsChargeAttack() const override;
	//移動撃ち可能か
	bool IsMoveShootable() const override;
	//停止撃ち専用か
	bool IsStopShootOnly() const override;

private:

	const float kAttackInterval = 5.0f; // 攻撃間隔定数

	bool isBursting_ = false;                // 3連射中かどうか
	int burstCount_ = 0;                     // 3連射のカウント
	float burstInterval_ = 0.0f;             // 3連射の間隔
	const int kMaxBurstCount = 3;            // 3連射の最大カウント
	const float kBurstAttackInterval = 0.1f; // 3連射時の攻撃間隔
};