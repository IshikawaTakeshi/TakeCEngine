#include "AnimationMapper.h"

//===================================================================================
//　ビヘイビア → アニメーション の登録
//===================================================================================
void AnimationMapper::Register(GameCharacterBehavior behavior, Animation* animation, float blendDuration) {

	entries_[behavior] = { animation, blendDuration };
}

//===================================================================================
//　ビヘイビア → アニメーション の検索
//===================================================================================
const AnimationEntry* AnimationMapper::Find(GameCharacterBehavior behavior) const {
	auto it = entries_.find(behavior);
	if (it != entries_.end()) {
		return &it->second;
	}
	return nullptr;
}
