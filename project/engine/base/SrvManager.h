#pragma once
#include <cstdint>
#include <wrl.h>
#include <d3d12.h>

class DirectXCommon;
class SrvManager {
public:

	//エイリアステンプレート
	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;

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
	void PreDraw();


	/// <summary>
	/// SRV生成（テクスチャ用）
	/// </summary>
	void CreateSRVforTexture2D(DXGI_FORMAT Format, UINT MipLevels, ID3D12Resource* pResource, uint32_t srvIndex);

	/// <summary>
	/// SRV生成（Structured Buffer用）
	/// </summary>
	void CreateSRVforStructuredBuffer(UINT numElements, UINT stride, ID3D12Resource* pResource, uint32_t srvIndex);

	//テクスチャ確保可能チェック
	bool CheckTextureAllocate();


public:
	//================================================================================================
	// getter/setter
	//================================================================================================

	/// <summary>
	/// CPUディスクリプタハンドルの取得
	/// </summary>
	D3D12_CPU_DESCRIPTOR_HANDLE GetSrvDescriptorHandleCPU(uint32_t index);

	/// <summary>
	/// GPUディスクリプタハンドルの取得
	/// </summary>
	D3D12_GPU_DESCRIPTOR_HANDLE GetSrvDescriptorHandleGPU(uint32_t index);

	/// srvHeapの取得
	ID3D12DescriptorHeap* GetSrvHeap() { return descriptorHeap_.Get(); }

	/// <summary>
	/// SRVの設定
	/// </summary>
	/// <param name="RootParameterIndex"></param>
	/// <param name="srvIndex"></param>
	void SetGraphicsRootDescriptorTable(UINT RootParameterIndex, uint32_t srvIndex);
	
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

