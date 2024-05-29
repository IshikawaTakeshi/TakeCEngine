#pragma once
#include <Windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>

#include <dxcapi.h>

#include <wrl.h>
#include <cassert>
#include <string>
#include <vector>
#include <iostream>
#include <memory>

#include "VertexData.h"
#include "Matrix4x4.h"
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
	/// 終了・開放処理
	/// </summary>
	void Finalize();

	/// <summary>
	/// 描画前処理
	/// </summary>
	void PreDraw(D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU);

	/// <summary>
	/// 描画後処理
	/// </summary>
	void PostDraw();

	/// <summary>
	/// レンダーターゲットのクリア
	/// </summary>
	void ClearRenderTarget(D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU);

	/// <summary>
	/// デバイスの取得
	/// </summary>
	/// <returns>デバイス</returns>
	Microsoft::WRL::ComPtr<ID3D12Device> GetDevice() const { return device_; }

	/// <summary>
	/// 描画コマンドリストの取得
	/// </summary>
	/// <returns>描画コマンドリスト</returns>
	ID3D12GraphicsCommandList* GetCommandList() const { return commandList_.Get(); }

	/// <summary>
	/// srvHeapの取得
	/// </summary>
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> GetSrvHeap() { return srvHeap_; }

	/// <summary>
	/// dsvHeapの取得
	/// </summary>
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> GetDsvHeap() { return dsvHeap_; }
private:
	// ウィンドウズアプリケーション管理
	WinApp* winApp_ = WinApp::GetInstance();
	//DXGIファクトリーの作成
	Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory_;
	//使用するアダプタ用の変数
	Microsoft::WRL::ComPtr<IDXGIAdapter4> useAdapter_;
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
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc_{};
//	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> swapChainResources_;
	Microsoft::WRL::ComPtr<ID3D12Resource> swapChainResources_[2];
	//ディスクリプタヒープの生成
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap_;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvHeap_;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvHeap_;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvHeap_;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> drawHeaps_[2];
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc_{};
	//RTVを2つ作るのでディスクリプタを2つ用意
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles_[2];
	//DSVを設定
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle_;
	//フェンスの生成
	Microsoft::WRL::ComPtr<ID3D12Fence> fence_;
	uint64_t fenceVal_;
	HANDLE fenceEvent_;
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
	/// <summary>
	/// DescriptorHeap作成関数
	/// </summary>
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(
		Microsoft::WRL::ComPtr<ID3D12Device> device, D3D12_DESCRIPTOR_HEAP_TYPE heapType,
		UINT numDescriptors, bool shaderVisible
	);
#pragma region dxc
private:
	//////////////////////////////////////////////////////////////////////////////////////////
	///			dxc
	//////////////////////////////////////////////////////////////////////////////////////////
	
	//dxc関連のメンバ変数
	Microsoft::WRL::ComPtr<IDxcUtils> dxcUtils_;
	Microsoft::WRL::ComPtr<IDxcCompiler3> dxcCompiler_;
	Microsoft::WRL::ComPtr<IDxcIncludeHandler> includeHandler_;
	Microsoft::WRL::ComPtr<IDxcBlobEncoding> shaderSource_;
	Microsoft::WRL::ComPtr<IDxcResult> shaderResult_;
	Microsoft::WRL::ComPtr<IDxcBlobUtf8> shaderError_;
	Microsoft::WRL::ComPtr<IDxcBlob> shaderBlob_;

private:
	//dxc関連のメンバ関数

	// <summary>
	/// dxc初期化
	/// </summary>
	void InitializeDxc();

	Microsoft::WRL::ComPtr<IDxcBlob> CompileShader(
		const std::wstring& filePath,
		const wchar_t* profile,
		IDxcUtils* dxcUtils,
		IDxcCompiler3* dxcCompiler,
		IDxcIncludeHandler* includeHandler
	);
#pragma endregion

#pragma region PSO
private:
	//////////////////////////////////////////////////////////////////////////////////////////
	///			PSO
	//////////////////////////////////////////////////////////////////////////////////////////
	
	//rootSignature
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature_{};
	D3D12_ROOT_PARAMETER rootParameters_[3] = {};
	D3D12_DESCRIPTOR_RANGE descriptorRange_[1] = {};
	Microsoft::WRL::ComPtr<ID3D10Blob> signatureBlob_;
	Microsoft::WRL::ComPtr<ID3D10Blob> errorBlob_;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
	D3D12_STATIC_SAMPLER_DESC staticSamplers_[1] = {};
	//InputLayout
	D3D12_INPUT_ELEMENT_DESC inputElementDescs_[2] = {};
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc_{};
	/// ブレンドステート
	D3D12_BLEND_DESC blendDesc_{};
	// ラスタライザステート
	D3D12_RASTERIZER_DESC rasterizerDesc_{};
	//PSO生成
	Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob_;
	Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob_;
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc_{};
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicPipelineState_;
	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc_{};

private:
	//PSO関連のメンバ関数
	
	/// <summary>
	/// ルートシグネチャ初期化
	/// </summary>
	void CreateRootSignature();

	/// <summary>
	/// インプットレイアウト初期化
	/// </summary>
	void CreateInputLayout();

	/// <summary>
	/// ブレンドステート初期化
	/// </summary>
	void CreateBlendState();

	/// <summary>
	/// ラスタライザステート初期化
	/// </summary>
	void CreateRasterizerState();

	/// <summary>
	/// PSO生成
	/// </summary>
	void CreatePSO();
#pragma endregion

#pragma region VertexResource
private:
	//////////////////////////////////////////////////////////////////////////////////////////
	///			vertexResource
	//////////////////////////////////////////////////////////////////////////////////////////


	////ヒーププロパティ
	//D3D12_HEAP_PROPERTIES uploadHeapProperties_{};
	////リソースディスク
	//D3D12_RESOURCE_DESC resourceDesc_{};

	//頂点リソース
	Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource_;
	//sprite用のTransformationMatrix用の頂点リソース
	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResourceSprite_;

	//頂点バッファビュー
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};
	//sprite用頂点バッファビュー
	D3D12_VERTEX_BUFFER_VIEW vertexBufferViewSprite_{};

	//三角形用頂点データ
	VertexData* vertexData_ = nullptr;;

	// ビューポート
	D3D12_VIEWPORT viewport_{};
	// シザー矩形
	D3D12_RECT scissorRect_{};

public:
	/// <summary>
	/// Resource生成関数
	/// </summary>
	static Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(
		Microsoft::WRL::ComPtr<ID3D12Device> device, size_t sizeInBytes);

private:

	/// <summary>
	/// Viewport初期化
	/// </summary>
	void InitViewport();

	/// <summary>
	/// シザー矩形初期化
	/// </summary>
	void InitScissorRect();

public:

	/// <summary>
	///頂点データの代入
	/// </summary>
	void SetWvpResource(
		Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource) {
		wvpResource_ = wvpResource;
	}

	/// <summary>
	/// 頂点バッファビューの代入
	/// </summary>
	void SetVertexBufferView(D3D12_VERTEX_BUFFER_VIEW vertexBufferView) {
		vertexBufferView_ = vertexBufferView;
	}

	/// <summary>
	/// sprite用のTransformationMatrix用の頂点データの代入
	/// </summary>
	void SetTransformationMatrixResourceSprite(
		Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResourceSprite) {
		transformationMatrixResourceSprite_ = transformationMatrixResourceSprite;
	}

	/// <summary>
	/// sprite用頂点バッファビューの代入
	/// </summary>
	void SetVertexBufferViewSprite(D3D12_VERTEX_BUFFER_VIEW vertexBufferViewSprite) {
		vertexBufferViewSprite_ = vertexBufferViewSprite;
	}

#pragma endregion

#pragma region materialResource
private:
	//メンバ変数
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_;
	Vector4* materialData_;

private:
	//メンバ関数
	/// <summary>
	/// VertexData初期化
	/// </summary>
	void InitializeMaterialData();
	
#pragma endregion
};

