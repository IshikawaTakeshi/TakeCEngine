#pragma once
#include "engine/Entity/GameCharacter.h"

class BulletSensor : public GameCharacter {
	public:

	BulletSensor() = default;
	~BulletSensor() override = default;
	void Initialize(Object3dCommon* object3dCommon, const std::string& filePath) override;
	void Update() override;
	void Draw() override;
	void DrawCollider() override;
	void OnCollisionAction(GameCharacter* other) override;


	//===============================================================================
	// accessor
	//===============================================================================

	//------ getter ------
	bool IsActive() const { return isActive_; }
	float GetSensorRadius() const { return sensorRadius_; }

	//------ setter ------
	void SetActive(bool isActive) { isActive_ = isActive; }
	void SetSensorRadius(float radius) { sensorRadius_ = radius; collider_->SetRadius(radius); }
private:

	bool isActive_ = true; // センサーが有効かどうか
	float activeDuration_ = 0.8f; // センサーが有効な時間（秒）
	float activeTimer_ = 0.0f; // センサーが有効になってからの経過時間
	float sensorRadius_ = 5.0f; // センサーの半径
};

