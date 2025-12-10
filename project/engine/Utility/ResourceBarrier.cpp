#include "ResourceBarrier.h"
#include <cassert>
#include <format>


//=============================================================================
// インスタンス取得
//=============================================================================
ResourceBarrier& ResourceBarrier::GetInstance() {
	static ResourceBarrier instance_;
	return instance_;
}

//=============================================================================
// 初期化
//=============================================================================
void ResourceBarrier::Initialize(DirectXCommon* dxCommon) {
	dxCommon_ = dxCommon;
}

//=============================================================================
// リソースバリアの設定(状態遷移)
//=============================================================================
void ResourceBarrier::Transition(D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState, ID3D12Resource* resource) {

	// 現在の状態が一致しているか確認
	auto it = resourceStates_.find(resource);
	if (it != resourceStates_.end()) {
		assert(it->second == beforeState && "Resource state mismatch in ResourceBarrier::Transition");
	}
	//リソースバリアの設定
	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Transition.pResource = resource;
	barrier.Transition.StateBefore = beforeState;
	barrier.Transition.StateAfter = afterState;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	//コマンドリストにリソースバリアを設定
	dxCommon_->GetCommandList()->ResourceBarrier(1, &barrier);

	// 状態を更新
	resourceStates_[resource] = afterState;
}
