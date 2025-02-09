#pragma once
#include <Windows.h>
#include <d3d12.h>
#include <dxcapi.h>
#include <wrl.h>
#include <cstdint>
#include <array>
#include <vector>
#include <string>
#include <unordered_map>

#include "PSOType.h"

// シェーダーリソース情報を一意に識別するためのキー
struct ShaderResourceKey {
	D3D_SHADER_INPUT_TYPE type;
	D3D12_SHADER_VISIBILITY visibility;
	UINT bindPoint;
	UINT space;

	bool operator==(const ShaderResourceKey& other) const {
		return type == other.type && visibility == other.visibility &&
			bindPoint == other.bindPoint && space == other.space;
	}
};

struct ShaderResourceKeyHash {
	std::size_t operator()(const ShaderResourceKey& key) const {
		return std::hash<UINT>()(static_cast<UINT>(key.type)) ^
			std::hash<UINT>()(static_cast<UINT>(key.visibility)) ^
			std::hash<UINT>()(key.bindPoint) ^
			std::hash<UINT>()(key.space);
	}
};

// リソース情報をまとめるデータ構造
struct ShaderResourceInfo {
	ShaderResourceKey key;
	std::string name;
};

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
	/// shaderからルートシグネチャ生成
	/// </summary>
	/// <param name="device"></param>
	/// <param name="shaderBlobs"></param>
	/// <param name="rootSignature"></param>
	ComPtr<ID3D12RootSignature> CreateRootSignatureFromShaders(
		ID3D12Device* device, const std::vector<ComPtr<IDxcBlob>>& shaderBlobs);

	std::unordered_map<ShaderResourceKey, ShaderResourceInfo, ShaderResourceKeyHash> LoadShaderResourceInfo
	(const std::vector<ComPtr<IDxcBlob>>& shaderBlobs);

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
	ID3D12RootSignature* GetGraphicRootSignature() const { return graphicRootSignature_.Get(); }
	ID3D12RootSignature* GetComputeRootSignature() const { return computeRootSignature_.Get(); }

	/// <summary>
	///graphicPipelineStateの取得
	/// </summary>
	ID3D12PipelineState* GetGraphicPipelineState() const { return graphicPipelineState_.Get(); }
	ID3D12PipelineState* GetComputePipelineState() const { return computePipelineState_.Get(); }

private:

	void SetGraphicPipelineStateDesc();
	void SetComputePipelineStateDesc();

	///////////////////////////////////////////////////////////////////////////////////////////
	///			privateメンバ変数
	///////////////////////////////////////////////////////////////////////////////////////////

	//rootSignature
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature_{};
	D3D12_ROOT_PARAMETER graphicRootParameters_[9] = {};
	D3D12_ROOT_PARAMETER computeRootParameters_[5] = {};
	D3D12_ROOT_PARAMETER rootParametersForParticle_[3] = {};

	D3D12_DESCRIPTOR_RANGE descriptorRange_[1] = {};
	D3D12_DESCRIPTOR_RANGE graphicDescriptorRange_[2] = {};
	D3D12_DESCRIPTOR_RANGE computeDescriptorRange_[4] = {};
	D3D12_DESCRIPTOR_RANGE descriptorRangeForInstancing_[1] = {};
	ComPtr<ID3D10Blob> signatureBlob_;
	ComPtr<ID3D10Blob> errorBlob_;
	ComPtr<ID3D12RootSignature> graphicRootSignature_;
	ComPtr<ID3D12RootSignature> computeRootSignature_;
	D3D12_STATIC_SAMPLER_DESC staticSamplers_[1] = {};
	//InputLayout
	std::array<D3D12_INPUT_ELEMENT_DESC,3> inputElementDescs_ = {};
	std::array<D3D12_INPUT_ELEMENT_DESC,2> inputElementDescsForSkyBox_ = {};
	std::array < D3D12_INPUT_ELEMENT_DESC,3> inputElementDescsForSkinningObject_ = {};
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc_{};
	D3D12_BLEND_DESC blendDesc_{};
	D3D12_RASTERIZER_DESC rasterizerDesc_{};
	//shaderBlob
	ComPtr<IDxcBlob> vertexShaderBlob_;
	ComPtr<IDxcBlob> pixelShaderBlob_;
	ComPtr<IDxcBlob> computeShaderBlob_;
	//depthStencilState
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc_{};
	//graphicPipelineState
	ComPtr<ID3D12PipelineState> graphicPipelineState_;
	ComPtr<ID3D12PipelineState> computePipelineState_;
	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc_{};
	D3D12_COMPUTE_PIPELINE_STATE_DESC computePipelineStateDesc_{};
	//BlendMode
	uint32_t itemCurrentIdx = 0;
	//device
	ID3D12Device* device_ = nullptr;
};