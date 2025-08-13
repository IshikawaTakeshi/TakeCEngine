#include "BaseBehavior.h"

std::optional<Behavior> BaseBehavior::TransitionNextBehavior(Behavior nextBehavior) {
	if (nextBehavior != Behavior::NONE) {
		// 次の行動がある場合はその行動を返す
		return nextBehavior;
	}
	return std::nullopt; // 次の行動がない場合はnulloptを返す
}