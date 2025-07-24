#pragma once
#include "engine/3d/Object3d.h"
#include "engine/Collision/Collider.h"

enum CharacterType {
	NONE,
	PLAYER,
	PLAYER_BULLET,
	PLAYER_MISSILE,
	ENEMY,
	ENEMY_BULLET,
	ENEMY_MISSILE,
	LEVEL_OBJECT,
};

class GameCharacter {
public:

	virtual ~GameCharacter() = default;

	virtual void Initialize(Object3dCommon* object3dCommon, const std::string& filePath) = 0;

	virtual void Update() = 0;

	virtual void Draw() = 0;

	virtual void DrawCollider() = 0;


	virtual void OnCollisionAction(GameCharacter* other) = 0;

	virtual Collider* GetCollider() { return collider_.get(); }

	virtual CharacterType GetCharacterType() { return characterType_; }

	Object3d* GetObject3d() { return object3d_.get(); }

	virtual void SetCharacterType(CharacterType type) { characterType_ = type; }

	virtual void SetTranslate(const Vector3& translate) { object3d_->SetTranslate(translate); }

	virtual void SetRotate(const Vector3& rotate) { object3d_->SetRotate(rotate); }

	virtual void SetScale(const Vector3& scale) { object3d_->SetScale(scale); }

	virtual void SetCamera(Camera* camera) { object3d_->SetCamera(camera); }

protected:

	std::unique_ptr<Object3d> object3d_;

	std::unique_ptr<Collider> collider_;

	CharacterType characterType_;
};