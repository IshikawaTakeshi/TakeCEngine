#include "ParticleEmitterAllocater.h"
#include <cassert>

//==================================================================================
// エミッターの割り当て
//==================================================================================
uint32_t ParticleEmitterAllocator::Allocate() {

	//上限に達してないかチェック
	assert(emitterCount_ < kMaxEmitterCount_);

	//returnする番号をいったん記録
	int index = emitterCount_;
	//次回のために番号を1進める
	emitterCount_++;
	//記録した番号をreturn
	return index;
}

//==================================================================================
// エミッターのクリア
//==================================================================================
void ParticleEmitterAllocator::Clear() {

	emitterCount_ = 0;
}