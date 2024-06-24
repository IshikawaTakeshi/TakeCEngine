#pragma once
#include <Windows.h>
#include <d3d12.h>
#include <dxcapi.h>
#include <wrl.h>

class DirectXCommon;
class DXC;
class PSO {

//////////////////////////////////////////////////////////////////////////////////////////
///			PSO
//////////////////////////////////////////////////////////////////////////////////////////

public:

	PSO() = default;
	~PSO() = default;

	/// <summary>
	/// 開放処理
	/// </summary>
	void Finalize();

	/// <summary>
	/// ルートシグネチャ初期化
	/// </summary>
	void CreateRootSignature(Microsoft::WRL::ComPtr<ID3D12Device> device);

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
	void CreatePSO(Microsoft::WRL::ComPtr<ID3D12Device> device, DXC* dxc);

	/// <summary>
	/// rootSignatureの取得
	/// </summary>
	Microsoft::WRL::ComPtr<ID3D12RootSignature> GetRootSignature() { return rootSignature_; }

	/// <summary>
	///graphicPipelineStateの取得
	/// </summary>
	Microsoft::WRL::ComPtr<ID3D12PipelineState> GetGraphicPipelineState() { return graphicPipelineState_; }

private:

	//rootSignature
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature_{};
	D3D12_ROOT_PARAMETER rootParameters_[4] = {};
	D3D12_DESCRIPTOR_RANGE descriptorRange_[1] = {};
	Microsoft::WRL::ComPtr<ID3D10Blob> signatureBlob_;
	Microsoft::WRL::ComPtr<ID3D10Blob> errorBlob_;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
	D3D12_STATIC_SAMPLER_DESC staticSamplers_[1] = {};
	//InputLayout
	D3D12_INPUT_ELEMENT_DESC inputElementDescs_[3] = {};
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


};

