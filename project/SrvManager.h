#pragma once
#include <cstdint>
#include <wrl.h>
#include <d3d12.h>

class DirectXCommon;
class SrvManager {
public:

	//エイリアステンプレート
	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;

	void Initialize(DirectXCommon* directXCommon);

	uint32_t Allocate();

	void PreDraw();

	/// <summary>
	/// CPUディスクリプタハンドルの取得
	/// </summary>
	D3D12_CPU_DESCRIPTOR_HANDLE GetSRVCPUDescriptorHandle(uint32_t index);

	/// <summary>
	/// GPUディスクリプタハンドルの取得
	/// </summary>
	D3D12_GPU_DESCRIPTOR_HANDLE GetSRVGPUDescriptorHandle(uint32_t index);

	/// <summary>
	/// SRV生成（テクスチャ用）
	/// </summary>
	void CreateSRVforTexture2D(uint32_t srvIndex, ID3D12Resource* pResource, DXGI_FORMAT Format, UINT MipLevels);

	/// <summary>
	/// SRV生成（Structured Buffer用）
	/// </summary>
	void CreateSRVforStructuredBuffer(uint32_t srvIndex, ID3D12Resource* pResource, UINT numElements, UINT stride);

	/// <summary>
	/// SRVの設定
	/// </summary>
	/// <param name="RootParameterIndex"></param>
	/// <param name="srvIndex"></param>
	void SetGraphicsRootDescriptorTable(UINT RootParameterIndex, uint32_t srvIndex);

	//テクスチャ確保可能チェック
	bool CheckTextureAllocate();

	//最大SRV数
	static const uint32_t kMaxSRVCount_;

private:

	DirectXCommon* dxCommon_ = nullptr;
	
	//SRV用のデスクリプタサイズ
	uint32_t descriptorSize_;
	//SRV用のデスクリプタヒープ
	ComPtr<ID3D12DescriptorHeap> descriptorHeap_ = nullptr;
	//次に使用するSRVインデックス
	uint32_t useIndex_ = 0;
};

