#pragma once
#include <Windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <cassert>
#include <wrl.h>
#include <vector>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

#include "WinApp.h"

class DirectXCommon {
public:

	/// <summary>
	/// シングルトンインスタンスの取得
	/// </summary>
	/// <returns></returns>
	static DirectXCommon* GetInstance();

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

	/// <summary>
	/// 描画前処理
	/// </summary>
	void PreDraw();

	/// <summary>
	/// 描画後処理
	/// </summary>
	void PostDraw();

	/// <summary>
	/// レンダーターゲットのクリア
	/// </summary>
	void ClearRenderTarget();

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

	

private:
	// ウィンドウズアプリケーション管理
	WinApp* winApp_ = WinApp::GetInstance();

	//DXGIファクトリーの作成
	Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory_;
	//D3D12Deviceの生成
	Microsoft::WRL::ComPtr<ID3D12Device> device_;
	//コマンドキューの生成
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue_;
	//コマンドアロケータの作成
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator_;
	//コマンドリストの生成
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList_;
	//スワップチェーンの生成
	Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain_;
	//ディスクリプタヒープの生成
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvHeap_;
	//RTVを2つ作るのでディスクリプタを2つ用意
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles_[2];
	//フェンスの生成
	Microsoft::WRL::ComPtr<ID3D12Fence> fence_;

	uint64_t fenceVal_ = 0;

	HANDLE fenceEvent_;

	Microsoft::WRL::ComPtr<ID3D12Resource> swapChainResources_[2];

	IDXGIAdapter4* useAdapter_;

	// TransitionBarrierの設定
	D3D12_RESOURCE_BARRIER barrier_;


private:
	DirectXCommon() = default;
	~DirectXCommon() = default;

	/// <summary>
	/// DXGIデバイス初期化
	/// </summary>
	void InitializeDXGIDevice();

	// <summary>
	/// コマンド関連初期化
	/// </summary>
	void InitializeCommand();


	/// <summary>
	/// スワップチェーンの生成
	/// </summary>
	void CreateSwapChain();

	/// <summary>
	/// レンダーターゲット生成
	/// </summary>
	void CreateFinalRenderTargets();

	/// <summary>
	/// フェンス生成
	/// </summary>
	void CreateFence();

};

