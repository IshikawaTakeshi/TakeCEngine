#pragma once
#include <Windows.h>
#include <d3d12.h>
#include <dxcapi.h>
#include <wrl.h>
#include <cstdint>
#include <array>

#include "PSOType.h"

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



	void CreateRootSignatureForSprite(ID3D12Device* device);

	/// <summary>
	/// Object3D用のルートシグネチャ生成
	/// </summary>
	void CreateRootSignatureForObject3D(ID3D12Device* device);

	void CreateRootSignatureForSkinnedObject3D(ID3D12Device* device);

	/// <summary>
	/// パーティクル用のルートシグネチャ生成
	/// </summary>
	/// <param name="device"></param>
	void CreateRootSignatureForParticle(ID3D12Device* device);

	/// <summary>
	/// インプットレイアウト初期化
	/// </summary>
	void CreateInputLayout();
	void CreateInputLayoutForSkyBox();
	void CreateInputLayoutForSkinningObject();

	/// <summary>
	/// ブレンドステート初期化
	/// </summary>
	void CreateBlendStateForObject3d();
	void CreateBlendStateForParticle();
	void CreateBlendStateForSprite();

	/// <summary>
	/// ラスタライザステート初期化
	/// </summary>
	void CreateRasterizerState(D3D12_FILL_MODE fillMode);

	/// <summary>
	/// PSO生成
	/// </summary>
	void CreatePSOForSprite(ID3D12Device* device, DXC* dxc_, D3D12_FILL_MODE fillMode);
	void CreatePSOForObject3D(ID3D12Device* device, DXC* dxc_, D3D12_FILL_MODE fillMode);
	void CreatePSOForSkinningObject3D(ID3D12Device* device, DXC* dxc_, D3D12_FILL_MODE fillMode);
	void CreatePSOForParticle(ID3D12Device* device, DXC* dxc_, D3D12_FILL_MODE fillMode);
	void CreatePSOForSkyBox(ID3D12Device* device, DXC* dxc_, D3D12_FILL_MODE fillMode);
	void CreatePSO(PSOType psoType, ID3D12Device* device, DXC* dxc_, D3D12_FILL_MODE fillMode);


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

	void SetGraphicPipelineStateDesc();

	///////////////////////////////////////////////////////////////////////////////////////////
	///			privateメンバ変数
	///////////////////////////////////////////////////////////////////////////////////////////

	//rootSignature
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature_{};
	D3D12_ROOT_PARAMETER rootParametersForObject3d_[8] = {};
	D3D12_ROOT_PARAMETER rootParametersForSkinningObject3d_[9] = {};
	D3D12_ROOT_PARAMETER rootParametersForParticle_[3] = {};
	D3D12_ROOT_PARAMETER rootParametersForSprite_[3] = {};
	D3D12_DESCRIPTOR_RANGE descriptorRange_[1] = {};
	D3D12_DESCRIPTOR_RANGE descriptorRangeForObject3d_[2] = {};
	D3D12_DESCRIPTOR_RANGE descriptorRangeForInstancing_[1] = {};
	ComPtr<ID3D10Blob> signatureBlob_;
	ComPtr<ID3D10Blob> errorBlob_;
	ComPtr<ID3D12RootSignature> rootSignature_;
	D3D12_STATIC_SAMPLER_DESC staticSamplers_[1] = {};
	//InputLayout
	std::array<D3D12_INPUT_ELEMENT_DESC,3> inputElementDescs_ = {};
	std::array<D3D12_INPUT_ELEMENT_DESC,2> inputElementDescsForSkyBox_ = {};
	std::array < D3D12_INPUT_ELEMENT_DESC,5> inputElementDescsForSkinningObject_ = {};
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