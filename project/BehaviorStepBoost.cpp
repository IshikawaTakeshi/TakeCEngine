#include "BehaviorStepBoost.h"
#include "application/Provider/IMoveDirectionProvider.h"
#include "engine/math/Vector3Math.h"
#include "engine/math/Quaternion.h"
#include "engine/math/Easing.h"
#include "engine/base/TakeCFrameWork.h"

BehaviorStepBoost::BehaviorStepBoost(IMoveDirectionProvider* provider) {
	moveDirectionProvider_ = provider;
}

void BehaviorStepBoost::Initialize(GameCharacterContext& characterInfo) {

}

void BehaviorStepBoost::Update(GameCharacterContext& characterInfo) {


}

std::pair<bool, Behavior> BehaviorStepBoost::TransitionNextBehavior(Behavior nextBehavior) {
	if (nextBehavior != Behavior::NONE) {
		// 次の行動がある場合はその行動を返す
		return { isTransition_, nextBehavior };
	}

	return { false, Behavior::NONE };
}
