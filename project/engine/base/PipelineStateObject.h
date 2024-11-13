#pragma once
#include <Windows.h>
#include <d3d12.h>
#include <dxcapi.h>
#include <wrl.h>
#include <cstdint>

class DXC;
class PSO {

///////////////////////////////////////////////////////////////////////////////////////////////
///			PSO
///////////////////////////////////////////////////////////////////////////////////////////////

public:

	///	エイリアステンプレート
	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;

public:

	///////////////////////////////////////////////////////////////////////////////////////////
	///			publicメンバ関数
	///////////////////////////////////////////////////////////////////////////////////////////

	PSO() = default;
	~PSO();

	/// <summary>
	/// 開放処理
	/// </summary>
	//void Finalize();

	
	void CreateRootSignatureForSprite(ID3D12Device* device);

	/// <summary>
	/// Object3D用のルートシグネチャ生成
	/// </summary>
	void CreateRootSignatureForObject3D(ID3D12Device* device);

	/// <summary>
	/// パーティクル用のルートシグネチャ生成
	/// </summary>
	/// <param name="device"></param>
	void CreateRootSignatureForParticle(ID3D12Device* device);

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
	void CreateRasterizerState(D3D12_CULL_MODE cullMode);

	/// <summary>
	/// PSO生成
	/// </summary>
	void CreatePSOForSprite(ID3D12Device* device, DXC* dxc_, D3D12_CULL_MODE cullMode);
	void CreatePSOForObject3D(ID3D12Device* device, DXC* dxc_, D3D12_CULL_MODE cullMode);

	void CreatePSOForParticle(ID3D12Device* device, DXC* dxc_, D3D12_CULL_MODE cullMode);

	void UpdateImGui();

	bool UpdateImGuiCombo();

public:

	///////////////////////////////////////////////////////////////////////////////////////////
	///			getter
	///////////////////////////////////////////////////////////////////////////////////////////


	/// <summary>
	/// rootSignatureの取得
	/// </summary>
	ID3D12RootSignature* GetRootSignature() const { return rootSignature_.Get(); }

	/// <summary>
	///graphicPipelineStateの取得
	/// </summary>
	ID3D12PipelineState* GetGraphicPipelineState() const { return graphicPipelineState_.Get(); }

private:

	///////////////////////////////////////////////////////////////////////////////////////////
	///			privateメンバ変数
	///////////////////////////////////////////////////////////////////////////////////////////

	//rootSignature
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature_{};
	D3D12_ROOT_PARAMETER rootParameters_[6] = {};
	D3D12_DESCRIPTOR_RANGE descriptorRange_[1] = {};
	D3D12_DESCRIPTOR_RANGE descriptorRangeForInstancing_[1] = {};
	ComPtr<ID3D10Blob> signatureBlob_;
	ComPtr<ID3D10Blob> errorBlob_;
	ComPtr<ID3D12RootSignature> rootSignature_;
	D3D12_STATIC_SAMPLER_DESC staticSamplers_[1] = {};
	//InputLayout
	D3D12_INPUT_ELEMENT_DESC inputElementDescs_[3] = {};
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc_{};
	D3D12_BLEND_DESC blendDesc_{};
	D3D12_RASTERIZER_DESC rasterizerDesc_{};
	//shaderBlob
	ComPtr<IDxcBlob> vertexShaderBlob_;
	ComPtr<IDxcBlob> pixelShaderBlob_;
	//depthStencilState
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc_{};
	//graphicPipelineState
	ComPtr<ID3D12PipelineState> graphicPipelineState_;
	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc_{};
	//BlendMode
	uint32_t itemCurrentIdx = 0;
	//device
	ID3D12Device* device_ = nullptr;
};