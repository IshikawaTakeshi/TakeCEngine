#pragma once
#include <Windows.h>
#include <d3d12.h>
#include <dxcapi.h>
#include <wrl.h>
#include <cstdint>
#include <array>
#include <vector>
#include <string>
#include <map>
#include <d3d12shader.h>
#include <d3dcompiler.h>

#include "PSOType.h"

///	エイリアステンプレート
template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;


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

	// std::map 用の比較関数
	bool operator<(const ShaderResourceKey& other) const {
		return std::tie(type, visibility, bindPoint, space) <
			std::tie(other.type, other.visibility, other.bindPoint, other.space);
	}
};

//struct ShaderResourceKeyHash {
//	std::size_t operator()(const ShaderResourceKey& key) const {
//		return std::hash<UINT>()(static_cast<UINT>(key.type)) ^
//			std::hash<UINT>()(static_cast<UINT>(key.visibility)) ^
//			std::hash<UINT>()(key.bindPoint) ^
//			std::hash<UINT>()(key.space);
//	}
//};

// リソース情報をまとめるデータ構造
struct BindResourceInfo {
	ShaderResourceKey key;
	std::string name;
};

using ShaderResourceMap = std::map<ShaderResourceKey, BindResourceInfo>;


struct GraphicShaderData {
	ComPtr<IDxcBlob> vertexBlob;
	ComPtr<IDxcBlob> pixelBlob;
};

class DXC;
class PSO {

	///////////////////////////////////////////////////////////////////////////////////////////////
	///			PSO
	///////////////////////////////////////////////////////////////////////////////////////////////

public:

	///////////////////////////////////////////////////////////////////////////////////////////
	///			publicメンバ関数
	///////////////////////////////////////////////////////////////////////////////////////////

	PSO() = default;
	~PSO();

	void CompileVertexShader(DXC* dxc_, const std::wstring& filePath);

	void CompilePixelShader(DXC* dxc_, const std::wstring& filePath);

	void CompileComputeShader(DXC* dxc_, const std::wstring& filePath);

	ShaderResourceMap LoadShaderResourceInfo(const std::vector<ComPtr<IDxcBlob>>& shaderBlobs);

	ComPtr<ID3D12RootSignature> CreateRootSignature(ID3D12Device* device, ShaderResourceMap resourceMap);

	void ExtractInputLayout(ID3D12ShaderReflection* shaderReflection);

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
	void CreateGraphicPSO(
		ID3D12Device* device,
		D3D12_FILL_MODE fillMode,
		D3D12_DEPTH_WRITE_MASK depthWriteMask,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE topologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE
	);

	void CreateComputePSO(ID3D12Device* device);

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

	void SetGraphicPipelineStateDesc(D3D12_PRIMITIVE_TOPOLOGY_TYPE type);
	void SetComputePipelineStateDesc();

	///////////////////////////////////////////////////////////////////////////////////////////
	///			privateメンバ変数
	///////////////////////////////////////////////////////////////////////////////////////////

	//rootSignature
	D3D12_ROOT_PARAMETER rootParametersForParticle_[3] = {};

	std::vector<std::string> semanticName_;

	ComPtr<ID3D10Blob> signatureBlob_;
	ComPtr<ID3D10Blob> errorBlob_;
	ComPtr<ID3D12RootSignature> graphicRootSignature_;
	ComPtr<ID3D12RootSignature> computeRootSignature_;

	D3D12_STATIC_SAMPLER_DESC staticSamplers_[1] = {};
	//InputLayout
	std::vector<D3D12_INPUT_ELEMENT_DESC> inputElementDescs_ = {};
	std::array<D3D12_INPUT_ELEMENT_DESC,2> inputElementDescsForSkyBox_ = {};
	std::array < D3D12_INPUT_ELEMENT_DESC,3> inputElementDescsForSkinningObject_ = {};
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc_{};
	D3D12_BLEND_DESC blendDesc_{};
	D3D12_RASTERIZER_DESC rasterizerDesc_{};

	//shaderBlob
	GraphicShaderData graphicShaderData_;
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