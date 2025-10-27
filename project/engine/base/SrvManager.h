#pragma once
#include "engine/base/DirectXCommon.h"
#include "engine/base/ComPtrAliasTemplates.h"
#include <cstdint>

//============================================================================
// SrvManager class
//============================================================================
class SrvManager {
public:

	//========================================================================
	// functions
	//========================================================================

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="directXCommon"></param>
	void Initialize(DirectXCommon* directXCommon);

	/// <summary>
	/// SRVインデックスの取得
	/// </summary>
	/// <returns></returns>
	uint32_t Allocate();

	/// <summary>
	/// 描画前処理
	/// </summary>
	void SetDescriptorHeap();


	/// <summary>
	/// SRV生成（Texture2D用）
	/// </summary>
	/// <param name="isCubeMap"></param>
	/// <param name="Format"></param>
	/// <param name="MipLevels"></param>
	/// <param name="pResource"></param>
	/// <param name="srvIndex"></param>
	void CreateSRVforTexture2D(bool isCubeMap, DXGI_FORMAT Format, UINT MipLevels, ID3D12Resource* pResource, uint32_t srvIndex);

	/// <summary>
	/// SRV生成（レンダーターゲット用）
	/// </summary>
	/// <param name="pResource"></param>
	/// <param name="Format"></param>
	/// <param name="srvIndex"></param>
	void CreateSRVforRenderTexture(	ID3D12Resource* pResource,DXGI_FORMAT Format, uint32_t srvIndex);

	/// <summary>
	/// SRV生成（Structured Buffer用）
	/// </summary>
	void CreateSRVforStructuredBuffer(UINT numElements, UINT stride, ID3D12Resource* pResource, uint32_t srvIndex);

	/// <summary>
	/// SRV生成（Depth Texture用）
	/// </summary>
	/// <param name="pResource"></param>
	/// <param name="srvIndex"></param>
	void CreateSRVforDepthTexture(ID3D12Resource* pResource, uint32_t srvIndex);

	/// <summary>
	/// UAV生成（RWStructured Buffer用）
	/// </summary>
	void CreateUAVforStructuredBuffer(UINT numElements, UINT stride, ID3D12Resource* pResource, uint32_t uavIndex);

	/// <summary>
	/// UAV生成（レンダーターゲット用）
	/// </summary>
	/// <param name="pResource"></param>
	/// <param name="Format"></param>
	/// <param name="uavIndex"></param>
	void CreateUAVforRenderTexture(ID3D12Resource* pResource,DXGI_FORMAT Format, uint32_t uavIndex);

	/// <summary>
	/// テクスチャ確保可能チェック
	/// </summary>
	/// <returns></returns>
	bool CheckTextureAllocate();


public:
	//================================================================================================
	// accessor
	//================================================================================================

	//----- getter ---------------

	// CPU用SRVデスクリプタハンドルの取得
	D3D12_CPU_DESCRIPTOR_HANDLE GetSrvDescriptorHandleCPU(uint32_t index);
	// GPU用SRVデスクリプタハンドルの取得
	D3D12_GPU_DESCRIPTOR_HANDLE GetSrvDescriptorHandleGPU(uint32_t index);
	/// srvHeapの取得
	ID3D12DescriptorHeap* GetSrvHeap() { return descriptorHeap_.Get(); }

	//----- setter ---------------

	//ディスクリプタテーブルの設定(Graphics)
	void SetGraphicsRootDescriptorTable(UINT RootParameterIndex, uint32_t srvIndex);
	//ディスクリプタテーブルの設定(Compute)
	void SetComputeRootDescriptorTable(UINT RootParameterIndex, uint32_t srvIndex);
	
public:
	//================================================================================================
	// 定数
	//================================================================================================
	
	//最大SRV数
	static const uint32_t kMaxSRVCount_;

private:

	//DirectXCommon
	DirectXCommon* dxCommon_ = nullptr;
	
	//SRV用のデスクリプタサイズ
	uint32_t descriptorSize_;
	//SRV用のデスクリプタヒープ
	ComPtr<ID3D12DescriptorHeap> descriptorHeap_ = nullptr;
	//次に使用するSRVインデックス
	uint32_t useIndex_ = 0;
};

