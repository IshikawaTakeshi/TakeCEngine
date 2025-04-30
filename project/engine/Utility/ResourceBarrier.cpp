#include "ResourceBarrier.h"

ResourceBarrier* ResourceBarrier::instance_ = nullptr;

ResourceBarrier* ResourceBarrier::GetInstance() {
 
	if(instance_ == nullptr) {
		instance_ = new ResourceBarrier();
	}
	return instance_;
}

void ResourceBarrier::Initialize(DirectXCommon* dxCommon) {
	dxCommon_ = dxCommon;
}

void ResourceBarrier::Finalize() {
	delete instance_;
	instance_ = nullptr;
}

void ResourceBarrier::Transition(D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState, ID3D12Resource* resource) {
	//リソースバリアの設定
	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Transition.pResource = resource;
	barrier.Transition.StateBefore = beforeState;
	barrier.Transition.StateAfter = afterState;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	//コマンドリストにリソースバリアを設定
	dxCommon_->GetCommandList()->ResourceBarrier(1, &barrier);
}
