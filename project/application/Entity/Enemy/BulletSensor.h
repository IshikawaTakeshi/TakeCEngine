#pragma once
#include "engine/Entity/GameCharacter.h"

//===================================================================================
// BulletSensor class
//===================================================================================
class BulletSensor : public GameCharacter {
	public:

	BulletSensor() = default;
	~BulletSensor() override = default;

	//===============================================================================
	// functions
	//===============================================================================

	// 初期化
	void Initialize(Object3dCommon* object3dCommon, const std::string& filePath) override;
	// 更新
	void Update() override;
	// ImGuiの更新
	void UpdateImGui();
	// 描画
	void Draw() override;
	// 描画（コライダー）
	void DrawCollider() override;
	// 衝突時のアクション
	void OnCollisionAction(GameCharacter* other) override;

	//===============================================================================
	// accessor
	//===============================================================================

	//------ getter -----------------------

	// センサーが有効かどうか
	bool IsActive() const { return isActive_; }
	// センサーの半径
	float GetSensorRadius() const { return sensorRadius_; }

	//------ setter -----------------------

	// センサーの設定
	void SetActive(bool isActive) { isActive_ = isActive; }
	// センサーの半径設定
	void SetSensorRadius(float radius) { sensorRadius_ = radius; collider_->SetRadius(radius); }
private:

	bool isActive_ = true;        // センサーが有効かどうか
	float activeDuration_ = 0.8f; // センサーが有効な時間（秒）
	float activeTimer_ = 0.0f;    // センサーが有効になってからの経過時間
	float sensorRadius_ = 5.0f;   // センサーの半径
};

