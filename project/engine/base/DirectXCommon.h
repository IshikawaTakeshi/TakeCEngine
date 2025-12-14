#pragma once
#include <Windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>

#include <wrl.h>
#include <string>
#include <iostream>
#include <array>
#include <chrono>
#include <memory>

#include "engine/Base/DirectXShaderCompiler.h"
#include "engine/Base/WinApp.h"
#include "engine/Base/RtvManager.h"
#include "engine/Base/DsvManager.h"
#include "engine/math/Vector4.h"

//============================================================================
// DirectXCommon class
//============================================================================
namespace TakeC {
	class DirectXCommon {
	public:

		//エイリアステンプレート
		template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;

	public:

		//============================================================================
		// functions
		//============================================================================

		/// <summary>
		/// コンストラクタ・デストラクタ
		/// </summary>
		DirectXCommon() = default;
		~DirectXCommon() = default;

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

		void WaitForGPU();

		/// <summary>
		/// Resource生成関数
		/// </summary>
		static ComPtr<ID3D12Resource> CreateBufferResource(ID3D12Device* device, size_t sizeInBytes);

		/// <summary>
		/// UAVを使用するResource生成関数
		/// </summary>
		static ComPtr<ID3D12Resource> CreateBufferResourceUAV(ID3D12Device* device, size_t sizeInBytes);

		/// <summary>
		/// 深度バッファの生成
		/// </summary>
		static ComPtr<ID3D12Resource> CreateDepthStencilTextureResource(
			const Microsoft::WRL::ComPtr<ID3D12Device>& device, int32_t width, int32_t height);


		/// <summary>
		/// DescriptorHeap作成関数
		/// </summary>
		ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(
			D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible);

		ComPtr<ID3D12Resource> CreateRenderTextureResource(ComPtr<ID3D12Device> device, uint32_t width, uint32_t height, DXGI_FORMAT format, const Vector4& clearColor);
		ComPtr<ID3D12Resource> CreateTextureResource(ComPtr<ID3D12Device> device, uint32_t width, uint32_t height, DXGI_FORMAT format);

		ComPtr<ID3D12Resource> CreateTextureResourceUAV(ComPtr<ID3D12Device> device, uint32_t width, uint32_t height, DXGI_FORMAT format);

		void DrawFPS();


	public:
		//============================================================================
		// accessors
		//============================================================================

		//----- getter ----------------------------

		/// デバイスの取得
		ID3D12Device* GetDevice() const { return device_.Get(); }

		/// 描画コマンドリストの取得
		ID3D12GraphicsCommandList* GetCommandList() const { return commandList_.Get(); }

		/// Dxcの取得
		DXC* GetDXC() { return dxc_.get(); }

		/// RTVManagerの取得
		RtvManager* GetRtvManager() { return rtvManager_.get(); }
		/// DSVManagerの取得
		DsvManager* GetDsvManager() { return dsvManager_.get(); }
		/// BufferCountの取得
		const UINT& GetBufferCount() { return swapChainDesc_.BufferCount; }

		/// CPUディスクリプタハンドルの取得
		D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(ID3D12DescriptorHeap* descriptorHeap, uint32_t descriptorSize, uint32_t index);

		/// GPUディスクリプタハンドルの取得
		D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(ID3D12DescriptorHeap* descriptorHeap, uint32_t descriptorSize, uint32_t index);
		/// ビューポートの取得
		const D3D12_VIEWPORT& GetViewport() { return viewport_; }
		/// シザー矩形の取得
		const D3D12_RECT& GetScissorRect() { return scissorRect_; }
		/// 現在のFPSの取得
		float GetCurrentFPS() const { return currentFPS_; }

	private:
		/////////////////////////////////////////////////////////////////////////////////////
		///			メンバ変数
		/////////////////////////////////////////////////////////////////////////////////////

		//WindowApp(借り物)
		WinApp* winApp_ = nullptr;
		//DirectXShaderCompiler
		std::unique_ptr<DXC> dxc_ = nullptr;
		//DXGIファクトリーの作成
		ComPtr<IDXGIFactory7> dxgiFactory_ = nullptr;
		//使用するアダプタ用の変数
		ComPtr<IDXGIAdapter4> useAdapter_ = nullptr;
		//D3D12Device
		ComPtr<ID3D12Device> device_ = nullptr;

		//command
		ComPtr<ID3D12CommandQueue> commandQueue_ = nullptr;
		ComPtr<ID3D12CommandAllocator> commandAllocator_ = nullptr;
		ComPtr<ID3D12GraphicsCommandList> commandList_ = nullptr;

		//swapChain
		ComPtr<IDXGISwapChain4> swapChain_ = nullptr;
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc_{};
		std::array<ComPtr<ID3D12Resource>, 2> swapChainResources_;

		//rtvManager
		std::unique_ptr<RtvManager> rtvManager_ = nullptr;
		uint32_t swapchainRtvIndex_[2];
		//dsvManager
		std::unique_ptr<DsvManager> dsvManager_ = nullptr;
		uint32_t dsvIndex_ = 0;

		//depthStencil
		Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilResource_ = nullptr;

		//fence
		ComPtr<ID3D12Fence> fence_ = nullptr;
		uint64_t fenceVal_;
		HANDLE fenceEvent_;
		// TransitionBarrierの設定
		D3D12_RESOURCE_BARRIER barrier_{};


		// ビューポート
		D3D12_VIEWPORT viewport_{};
		// シザー矩形
		D3D12_RECT scissorRect_{};

		//画面の色
		float clearColor_[4] = { 0.1f, 0.25f, 0.5f, 1.0f }; // 青っぽい色

		//記録時間(FPS固定)
		std::chrono::steady_clock::time_point reference_;
		int frameCount_ = 0;
		float currentFPS_ = 0.0f;


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
		/// レンダーターゲットのクリア
		/// </summary>
		void ClearRenderTarget();

		/// <summary>
		/// RTVの初期化
		/// </summary>
		void InitializeRenderTargetView();

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
		/// バリア設定
		/// </summary>
		void SetBarrier(D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState, ID3D12Resource* resource);

	};
}