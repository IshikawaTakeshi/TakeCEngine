#pragma once
#include "GameCharacter.h"
class SampleCharacter : public GameCharacter {
public:

	void Initialize(Object3dCommon* object3dCommon, const std::string& filePath) override;
	void Update() override;
	void Draw() override;
	void OnCollisionAction(Collider* other) override;

private:


};

