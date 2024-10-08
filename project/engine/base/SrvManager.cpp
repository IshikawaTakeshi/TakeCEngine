#include "SrvManager.h"
#include "DirectXCommon.h"
#include <cassert>



const uint32_t SrvManager::kMaxSRVCount_ = 512;

//================================================================================================
// 初期化
//================================================================================================

void SrvManager::Initialize(DirectXCommon* directXCommon) {

	dxCommon_ = directXCommon;

	//デスクリプタヒープの生成
	descriptorHeap_ = dxCommon_->CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, kMaxSRVCount_, true);
	//デスクリプタ1個分のサイズを取得して記録
	descriptorSize_ = dxCommon_->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

//================================================================================================
// SRVインデックスの取得
//================================================================================================

uint32_t SrvManager::Allocate() {

	//上限に達してないかチェック
	assert(useIndex_ < kMaxSRVCount_);

	//returnする番号をいったん記録
	int index = useIndex_;
	//次回のために番号を1進める
	useIndex_++;
	//記録した番号をreturn
	return index;
}

//================================================================================================
// CPUディスクリプタハンドルの取得
//================================================================================================

D3D12_CPU_DESCRIPTOR_HANDLE SrvManager::GetSrvDescriptorHandleCPU(uint32_t index) {
	D3D12_CPU_DESCRIPTOR_HANDLE handleCPU = descriptorHeap_->GetCPUDescriptorHandleForHeapStart();
	handleCPU.ptr += (descriptorSize_ * index);
	return handleCPU;
}

//================================================================================================
// GPUディスクリプタハンドルの取得
//================================================================================================

D3D12_GPU_DESCRIPTOR_HANDLE SrvManager::GetSrvDescriptorHandleGPU(uint32_t index) {
	D3D12_GPU_DESCRIPTOR_HANDLE handleGPU = descriptorHeap_->GetGPUDescriptorHandleForHeapStart();
	handleGPU.ptr += (descriptorSize_ * index);
	return handleGPU;
}

//================================================================================================
// SRV生成（テクスチャ用）
//================================================================================================

void SrvManager::CreateSRVforTexture2D(DXGI_FORMAT Format, UINT MipLevels,ID3D12Resource* pResource, uint32_t srvIndex) {
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = Format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = MipLevels;

	dxCommon_->GetDevice()->CreateShaderResourceView(pResource, &srvDesc, GetSrvDescriptorHandleCPU(srvIndex));
}

//================================================================================================
// SRV生成（Structured Buffer用）
//================================================================================================

void SrvManager::CreateSRVforStructuredBuffer(UINT numElements, UINT stride, ID3D12Resource* pResource,uint32_t srvIndex) {

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Buffer.NumElements = numElements;
	srvDesc.Buffer.StructureByteStride = stride;
	srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

	dxCommon_->GetDevice()->CreateShaderResourceView(pResource, &srvDesc, GetSrvDescriptorHandleCPU(srvIndex));
}

//================================================================================================
// SRVの設定
//================================================================================================

void SrvManager::SetGraphicsRootDescriptorTable(UINT RootParameterIndex, uint32_t srvIndex) {
	dxCommon_->GetCommandList()->SetGraphicsRootDescriptorTable(RootParameterIndex, GetSrvDescriptorHandleGPU(srvIndex));
}

//================================================================================================
// テクスチャ確保可能チェック
//================================================================================================

bool SrvManager::CheckTextureAllocate() {
	
	if (useIndex_ < kMaxSRVCount_) {
		return true;
	}

	return false;
}

//================================================================================================
// 描画前処理
//================================================================================================

void SrvManager::PreDraw() {

	//SRV用ディスクリプタヒープの設定
	ID3D12DescriptorHeap* drawHeaps_[] = { descriptorHeap_.Get() };
	dxCommon_->GetCommandList()->SetDescriptorHeaps(1, drawHeaps_);
}


