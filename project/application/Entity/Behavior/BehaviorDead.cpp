#include "BehaviorDead.h"
#include "engine/io/Input.h"
#include "engine/math/Vector3Math.h"
#include "application/Provider/BaseInputProvider.h"
#include "engine/base/TakeCFrameWork.h"

BehaviorDead::BehaviorDead(baseInputProvider* provider) {
	inputProvider_ = provider;
	deltaTime_ = TakeCFrameWork::GetDeltaTime();
}

void BehaviorDead::Initialize([[maybe_unused]]GameCharacterContext& characterInfo) {
}

void BehaviorDead::Update(GameCharacterContext& characterInfo) {
	characterInfo;
}
