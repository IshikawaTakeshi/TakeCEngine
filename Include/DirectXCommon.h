#pragma once
#include <Windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>

#include <wrl.h>
#include <string>
#include <vector>
#include <iostream>
#include <array>
#include <chrono>

#include "../Include/WinApp.h"
#include "../MyMath/Matrix4x4.h"
#include "../Include/ResourceDataStructure.h"

class DXC;
class PSO;
class DirectXCommon {

public:
	/////////////////////////////////////////////////////////////////////////////////////
	///			エイリアステンプレート
	/////////////////////////////////////////////////////////////////////////////////////

	//エイリアステンプレート
	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;


public:
	/////////////////////////////////////////////////////////////////////////////////////
	///			publicメンバ関数
	/////////////////////////////////////////////////////////////////////////////////////

	/// <summary>
	/// コンストラクタ
	/// </summary>
	DirectXCommon() = default;

	/// <summary>
	/// デストラクタ
	/// </summary>
	~DirectXCommon();

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(WinApp* winApp);

	/// <summary>
	/// 終了・開放処理
	/// </summary>
	void Finalize();

	/// <summary>
	/// 描画前処理
	/// </summary>
	void PreDraw();

	/// <summary>
	/// 描画後処理
	/// </summary>
	void PostDraw();

	


	/// <summary>
	/// Resource生成関数
	/// </summary>
	static ComPtr<ID3D12Resource> CreateBufferResource(
		ID3D12Device* device, size_t sizeInBytes);


public:
	/////////////////////////////////////////////////////////////////////////////////////
	///			Getter
	/////////////////////////////////////////////////////////////////////////////////////

	/// <summary>
	/// デバイスの取得
	/// </summary>
	/// <returns>デバイス</returns>
	ID3D12Device* GetDevice() const { return device_.Get(); }

	/// <summary>
	/// 描画コマンドリストの取得
	/// </summary>
	/// <returns>描画コマンドリスト</returns>
	ID3D12GraphicsCommandList* GetCommandList() const { return commandList_.Get(); }

	/// <summary>
	/// srvHeapの取得
	/// </summary>
	ID3D12DescriptorHeap* GetSrvHeap() { return srvHeap_.Get(); }

	/// <summary>
	/// dsvHeapの取得
	/// </summary>
	ID3D12DescriptorHeap* GetDsvHeap() { return dsvHeap_.Get(); }

	/// <summary>
	/// rtvHeapの取得
	/// </summary>
	ID3D12DescriptorHeap* GetRtvHeap() { return rtvHeap_.Get(); }

	/// <summary>
	/// BufferCountの取得
	/// </summary>
	/// <returns></returns>
	UINT GetBufferCount() { return swapChainDesc_.BufferCount; }

	/// <summary>
	/// rtvDescの取得
	/// </summary>
	DXGI_FORMAT GetRtvFormat() { return rtvDesc_.Format; }

	/// <summary>
	/// Dxcの取得
	/// </summary>
	/// <returns></returns>
	DXC* GetDXC() { return dxc_; }

	uint32_t GetDescriptorSizeSRV() { return descriptorSizeSRV_; }
	uint32_t GetDescriptorSizeRTV() { return descriptorSizeRTV_; }
	uint32_t GetDescriptorSizeDSV() { return descriptorSizeDSV_; }

	/// <summary>
	/// CPUディスクリプタハンドルの取得
	/// </summary>
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(ID3D12DescriptorHeap* descriptorHeap, uint32_t descriptorSize, uint32_t index);

	/// <summary>
	/// GPUディスクリプタハンドルの取得
	/// </summary>
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(ID3D12DescriptorHeap* descriptorHeap, uint32_t descriptorSize, uint32_t index);

	/// <summary>
	/// SRV専用のCPUディスクリプタハンドルの取得
	/// </summary>
	D3D12_CPU_DESCRIPTOR_HANDLE GetSRVCPUDescriptorHandle(uint32_t index);

	/// <summary>
	/// SRV専用のGPUディスクリプタハンドルの取得
	/// </summary>
	D3D12_GPU_DESCRIPTOR_HANDLE GetSRVGPUDescriptorHandle(uint32_t index);


public:
	/////////////////////////////////////////////////////////////////////////////////////
	///			定数
	/////////////////////////////////////////////////////////////////////////////////////

	static const uint32_t kMaxSRVCount;

private:
	/////////////////////////////////////////////////////////////////////////////////////
	///			メンバ変数
	/////////////////////////////////////////////////////////////////////////////////////

	//WindowApp
	WinApp* winApp_ = nullptr;
	//DirectXShaderCompiler
	DXC* dxc_ = nullptr;
	//PipelineStateObject
	PSO* pso_ = nullptr;
	//DXGIファクトリーの作成
	ComPtr<IDXGIFactory7> dxgiFactory_ = nullptr;
	//使用するアダプタ用の変数
	ComPtr<IDXGIAdapter4> useAdapter_ = nullptr;
	//D3D12Deviceの生成
	ComPtr<ID3D12Device> device_ = nullptr;
	//コマンドキューの生成
	ComPtr<ID3D12CommandQueue> commandQueue_ = nullptr;
	//コマンドアロケータの作成
	ComPtr<ID3D12CommandAllocator> commandAllocator_ = nullptr;
	//コマンドリストの生成
	ComPtr<ID3D12GraphicsCommandList> commandList_ = nullptr;
	//スワップチェーンの生成
	ComPtr<IDXGISwapChain4> swapChain_ = nullptr;
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc_{};
	std::array<ComPtr<ID3D12Resource>,2> swapChainResources_;
	//深度ステンシルバッファの生成
	Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilResource_ = nullptr;


	//ディスクリプタヒープの生成
	ComPtr<ID3D12DescriptorHeap> rtvHeap_ = nullptr;
	ComPtr<ID3D12DescriptorHeap> srvHeap_ = nullptr;
	ComPtr<ID3D12DescriptorHeap> dsvHeap_ = nullptr;

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc_{};
	//RTVHandleの要素数
	static inline const uint32_t rtvCount_ = 2;
	//RTVを2つ作るのでディスクリプタを2つ用意
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles_[rtvCount_] = {};
	//DSVを設定
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle_{};
	//フェンスの生成
	ComPtr<ID3D12Fence> fence_ = nullptr;
	uint64_t fenceVal_;
	HANDLE fenceEvent_;
	// TransitionBarrierの設定
	D3D12_RESOURCE_BARRIER barrier_{};

	uint32_t descriptorSizeSRV_;
	uint32_t descriptorSizeRTV_;
	uint32_t descriptorSizeDSV_;

	// ビューポート
	D3D12_VIEWPORT viewport_{};
	// シザー矩形
	D3D12_RECT scissorRect_{};

	//記録時間(FPS固定)
	std::chrono::steady_clock::time_point reference_;


private:

	/////////////////////////////////////////////////////////////////////////////////////
	///			privateメンバ関数
	/////////////////////////////////////////////////////////////////////////////////////

	/// <summary>
	/// DXGIデバイス初期化
	/// </summary>
	void InitializeDXGIDevice();

	// <summary>
	/// コマンドキュー・リスト・アロケータ初期化
	/// </summary>
	void InitializeCommand();

	/// <summary>
	/// Viewport初期化
	/// </summary>
	void InitViewport();

	/// <summary>
	/// シザー矩形初期化
	/// </summary>
	void InitScissorRect();

	/// <summary>
	/// スワップチェーンの生成
	/// </summary>
	void CreateSwapChain();

	/// <summary>
	/// 深度バッファの生成
	/// </summary>
	void CreateDepthStencilTextureResource(
		const Microsoft::WRL::ComPtr<ID3D12Device>& device, int32_t width, int32_t height);

	/// <summary>
	/// ディスクリプタヒープ生成
	/// </summary>
	void CreateDescriptorHeap();

	/// <summary>
	/// レンダーターゲットのクリア
	/// </summary>
	void ClearRenderTarget();
	
	/// <summary>
	/// RTVの初期化
	/// </summary>
	void InitializeRenderTargetView();

	/// <summary>
	/// DSVの初期化
	/// </summary>
	void InitializeDepthStencilView();

	/// <summary>
	/// フェンス生成
	/// </summary>
	void CreateFence();

	/// <summary>
	/// FPS固定の初期化
	/// </summary>
	void InitializeFixFPS();

	/// <summary>
	///	FPS固定の更新
	/// </summary>
	void UpdateFixFPS();

	/// <summary>
	/// DescriptorHeap作成関数
	/// </summary>
	ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(
		ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE heapType,
		UINT numDescriptors, bool shaderVisible
	);


};

