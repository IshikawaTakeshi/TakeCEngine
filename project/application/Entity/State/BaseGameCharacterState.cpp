#include "BaseGameCharacterState.h"

//===================================================================================
//　遷移先のビヘイビアチェック
//===================================================================================
std::pair<bool,State> BaseGameCharacterState::TransitionNextState(State nextBehavior) {
	if (nextBehavior != State::NONE) {
		// 次の行動がある場合はその行動を返す
		return { isTransition_, nextBehavior };
	}

	return { false, State::NONE };
}