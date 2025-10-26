#include "engine/3d/Light/LightCounter.h"

//==============================================================================
//		LightCounter 初期化
//==============================================================================

template<typename LightT>
inline void LightCounter<LightT>::Initialize() {
	used_.fill(0);
    freeList_.clear();
    freeList_.reserve(MaxLights_);
    for (uint32_t i = 0; i < MaxLights_; ++i) freeList_.push_back(i);
    count_ = 0;
}

//==============================================================================
//	ライト追加
//==============================================================================

template<typename LightT>
inline uint32_t LightCounter<LightT>::Add(const LightT& data) {

	//空きインデックスがあればそれを使用
	if (!freeList_.empty()) {
		uint32_t index = freeList_.back();
		// 空きリストから削除し、使用中フラグを立てる
		freeList_.pop_back();
		data_[index] = data;
		used_[index] = 1;
		++count_;
		return index;
	}

	//空きインデックスがなければ新規に割り当て
	for (uint32_t i = 0; i < MaxLights_; ++i) {
		// 未使用のライトを見つけたら割り当て
		if (used_[i] == 0) {
			data_[i] = data;
			used_[i] = 1;
			++count_;
			return i;
		}
	}
	return MaxLights_; // ライトが満杯の場合
}

//==============================================================================
//	ライト削除
//==============================================================================

template<typename LightT>
inline bool LightCounter<LightT>::Remove(uint32_t index) {
	if (index >= MaxLights_ || used_[index] == 0) {
		return false; // 無効なインデックスまたは未使用のライト
	}
	used_[index] = 0;
	freeList_.push_back(index);
	--count_;
	return true;
}

//==============================================================================
//	ライト数取得
//==============================================================================

template<typename LightT>
inline uint32_t LightCounter<LightT>::GetCount() const {
	return static_cast<uint32_t>(count_);
}

//==============================================================================
//	ライトデータ取得
//==============================================================================

template<typename LightT>
inline LightT* LightCounter<LightT>::GetLightData(uint32_t index) {
	if (index >= MaxLights_ || used_[index] == 0) {
		return nullptr; // 無効なインデックスまたは未使用のライト
	}
	return &data_[index];
}