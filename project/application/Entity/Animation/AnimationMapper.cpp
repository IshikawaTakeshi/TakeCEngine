#include "AnimationMapper.h"

//===================================================================================
//　ビヘイビア → アニメーション の登録
//===================================================================================
void AnimationMapper::Register(GameCharacterState behavior, Animation* animation, float blendDuration, bool isLoop) {

	entries_[behavior] = { animation, blendDuration, isLoop };
}

//===================================================================================
//　ビヘイビア → アニメーション の検索
//===================================================================================
const AnimationEntry* AnimationMapper::Find(GameCharacterState behavior) const {
	auto it = entries_.find(behavior);
	if (it != entries_.end()) {
		return &it->second;
	}
	return nullptr;
}
