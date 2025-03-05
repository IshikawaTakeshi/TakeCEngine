#pragma once
#include "Object3d.h"
#include "Collision/Collider.h"

enum class CharacterType {
	NONE,
	PLAYER,
	ENEMY,
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

protected:

	std::unique_ptr<Object3d> object3d_;

	std::unique_ptr<Collider> collider_;

	CharacterType characterType_ = CharacterType::NONE;
};