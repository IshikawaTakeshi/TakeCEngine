#include "BaseBehavior.h"

std::pair<bool,Behavior> BaseBehavior::TransitionNextBehavior(Behavior nextBehavior) {
	if (nextBehavior != Behavior::NONE) {
		// 次の行動がある場合はその行動を返す
		return { true, nextBehavior };
	}

	return { false, Behavior::NONE };
}