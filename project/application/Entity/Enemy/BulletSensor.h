#pragma once
#include "engine/Entity/GameCharacter.h"

class BulletSensor : public GameCharacter {
	public:

	BulletSensor() = default;
	~BulletSensor() override = default;
	void Initialize(Object3dCommon* object3dCommon, const std::string& filePath) override;
	void Update() override;
	void UpdateImGui();
	void Draw() override;
	void DrawCollider() override;
	void OnCollisionAction(GameCharacter* other) override;


	//===============================================================================
	// accessor
	//===============================================================================

	//------ getter ------
	bool IsActive() const { return isActive_; }


	//------ setter ------
	void SetActive(bool isActive) { isActive_ = isActive; }

private:

	bool isActive_ = true; // センサーが有効かどうか
};

