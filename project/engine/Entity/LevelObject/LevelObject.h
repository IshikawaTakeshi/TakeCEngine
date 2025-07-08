#pragma once
#include "engine/Entity/GameCharacter.h"
#include "engine/3d/Object3dCommon.h"
#include "scene/LevelData.h"
#include <string>


class LevelObject : public GameCharacter {
public:
	LevelObject() = default;
	~LevelObject() override = default;
	void Initialize(Object3dCommon* object3dCommon, const std::string& filePath) override;
	void CollisionInitialize(const LevelData::BoxCollider& boxInfo);
	void CollisionInitialize(const LevelData::SphereCollider& sphereInfo);
	void Update() override;
	void UpdateImGui();
	void Draw() override;
	void DrawCollider() override;
	void OnCollisionAction(GameCharacter* other) override;
	Collider* GetCollider() override { return collider_.get(); }
	CharacterType GetCharacterType() override { return characterType_; }
	void SetCharacterType(CharacterType type) override { characterType_ = type; }

	void SetName(const std::string& name) { name_ = name; }

private:

	Object3dCommon* object3dCommon_ = nullptr; // Object3dCommonの参照

	std::string name_; // オブジェクトの名前

	float deltaTime_ = 0.0f; // 更新間隔

	//衝突時の点滅タイマー
	bool isBlinking_ = false; // 点滅中かどうか
	float blinkTimer_ = 0.0f;
	const float kBlinkInterval = 0.1f; // 点滅間隔
	const float kBlinkDuration = 1.0f; // 点滅持続時間
};

