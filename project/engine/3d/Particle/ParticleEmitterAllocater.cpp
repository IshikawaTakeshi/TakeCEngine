#include "ParticleEmitterAllocater.h"
#include <cassert>

//==================================================================================
// エミッターIDの割り当て（登録）
//==================================================================================
uint32_t ParticleEmitterAllocator::Allocate(ParticleEmitter* emitter) {
	assert(emitter != nullptr && "エミッターがnullptrです");

	// 上限に達していないかチェック
	assert(activeEmitters_.size() < kMaxEmitterCount_ && "エミッターの上限に達しています");

	uint32_t emitterID = 0;

	// 解放されたIDがあれば再利用
	if (!freeIDs_.empty()) {
		emitterID = freeIDs_.back();
		freeIDs_.pop_back();
	} else {
		// 新しいIDを発行
		emitterID = nextEmitterID_++;
	}

	// エミッターを登録
	activeEmitters_[emitterID] = emitter;

	return emitterID;
}

//==================================================================================
// エミッターIDの解放（登録解除）
//==================================================================================
void ParticleEmitterAllocator::Release(uint32_t emitterID) {
	// エミッターIDが0（無効）の場合は何もしない
	if (emitterID == 0) {
		return;
	}

	// エミッターが登録されているか確認
	auto it = activeEmitters_.find(emitterID);
	if (it != activeEmitters_.end()) {
		// エミッターを削除
		activeEmitters_.erase(it);

		// IDを再利用リストに追加
		freeIDs_.push_back(emitterID);
	}
}

//==================================================================================
// エミッターIDから現在のエミッター位置を取得
//==================================================================================
std::optional<Vector3> ParticleEmitterAllocator::GetEmitterPosition(uint32_t emitterID) const {
	auto it = activeEmitters_.find(emitterID);
	if (it != activeEmitters_.end() && it->second != nullptr) {
		return it->second->GetPosition();
	}
	return std::nullopt; // エミッターが見つからない
}

//=================================================================================
// エミッターIDから発射方向を取得
//=================================================================================
std::optional<Vector3> ParticleEmitterAllocator::GetEmitDirection(uint32_t emitterID) const {
	auto it = activeEmitters_.find(emitterID);
	if (it != activeEmitters_.end() && it->second != nullptr) {
		return it->second->GetEmitDirection();
	}
	return std::nullopt; // エミッターが見つからない
}

//==================================================================================
// エミッターIDからエミッターポインタを取得
//==================================================================================
ParticleEmitter* ParticleEmitterAllocator::GetEmitter(uint32_t emitterID) const {
	auto it = activeEmitters_.find(emitterID);
	if (it != activeEmitters_.end()) {
		return it->second;
	}
	return nullptr;
}

//==================================================================================
// 全エミッターのクリア
//==================================================================================
void ParticleEmitterAllocator::Clear() {
	activeEmitters_.clear();
	freeIDs_.clear();
	nextEmitterID_ = 1; // IDカウンターをリセット
}