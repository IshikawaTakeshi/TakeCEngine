#pragma once
#include "Object3d.h"
#include "Collision/Collider.h"

class GameCharacter {
public:

	virtual ~GameCharacter() = default;

	virtual void Initialize(Object3dCommon* object3dCommon, const std::string& filePath) = 0;

	virtual void Update() = 0;

	virtual void Draw() = 0;

	virtual void OnCollisionAction(Collider* other) = 0;

	virtual Collider* GetCollider() { return collider_.get(); }

protected:

	std::unique_ptr<Object3d> object3d_;

	std::unique_ptr<Collider> collider_;

};