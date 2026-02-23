#include "AnimationMapper.h"

//===================================================================================
//　ビヘイビア → アニメーション の登録
//===================================================================================
void AnimationMapper::Register(GameCharacterBehavior behavior, Animation* animation, float blendDuration) {

	map_[behavior] = { animation, blendDuration };
}

//===================================================================================
//　ビヘイビア → アニメーション の検索
//===================================================================================
const AnimationMapper::Entry* AnimationMapper::Get(GameCharacterBehavior behavior) const {
	auto it = map_.find(behavior);
	if (it != map_.end()) {
		return &it->second;
	}
	return nullptr;
}
