#include "DsvManager.h"
#include "DirectXCommon.h"
#include <cassert>

using namespace TakeC;

// DSVヒープのディスクリプタ数
const uint32_t DsvManager::kMaxDSVCount = 10;

//================================================================================================
// 初期化
//================================================================================================
void TakeC::DsvManager::Initialize(TakeC::DirectXCommon* dxCommon) {

	dxCommon_ = dxCommon;

	//ディスクリプタヒープの生成
	descriptorHeap_= dxCommon_->CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, kMaxDSVCount, false);
	//ディスクリプタヒープのサイズを取得
	descriptorSize_ = dxCommon_->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	//descの初期化
	dsvDesc_.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc_.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
}

//================================================================================================
// DSVインデックスの取得
//================================================================================================
uint32_t TakeC::DsvManager::Allocate() {
	//上限に達してないかチェック
	assert(useIndex_ < kMaxDSVCount);
	//returnする番号をいったん記録
	int index = useIndex_;
	//次回のために番号を1進める
	useIndex_++;
	//記録した番号をreturn
	return index;
}

//================================================================================================
// DSV生成
//================================================================================================
void TakeC::DsvManager::CreateDSV(ComPtr<ID3D12Resource> depthStencilResource, uint32_t dsvIndex) {
	//デスクリプタヒープのハンドルを取得
	D3D12_CPU_DESCRIPTOR_HANDLE handle = GetDsvDescriptorHandleCPU(dsvIndex);
	//DSVの生成
	dxCommon_->GetDevice()->CreateDepthStencilView(depthStencilResource.Get(), &dsvDesc_, handle);
}

//================================================================================================
// CPUデスクリプタハンドルの取得
//================================================================================================
D3D12_CPU_DESCRIPTOR_HANDLE TakeC::DsvManager::GetDsvDescriptorHandleCPU(uint32_t index) {
	
	//デスクリプタヒープの先頭ハンドルを取得
	D3D12_CPU_DESCRIPTOR_HANDLE handle = descriptorHeap_->GetCPUDescriptorHandleForHeapStart();
	//index分ずらす
	handle.ptr += descriptorSize_ * index;
	return handle;
}

//================================================================================================
// GPUデスクリプタハンドルの取得
//================================================================================================
D3D12_GPU_DESCRIPTOR_HANDLE TakeC::DsvManager::GetDsvDescriptorHandleGPU(uint32_t index) {
	
	//デスクリプタヒープの先頭ハンドルを取得
	D3D12_GPU_DESCRIPTOR_HANDLE handle = descriptorHeap_->GetGPUDescriptorHandleForHeapStart();
	//index分ずらす
	handle.ptr += descriptorSize_ * index;
	return handle;
}
