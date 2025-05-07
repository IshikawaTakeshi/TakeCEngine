#pragma once
#include "base/DirectXCommon.h"
#include "base/SrvManager.h"
#include "base/RtvManager.h"
#include "base/PipelineStateObject.h"
#include <string>



class PostEffect {
public:
	PostEffect() = default;
	~PostEffect();

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(DirectXCommon* dxCommon, SrvManager* srvManager, const std::wstring& CSFilePath, ComPtr<ID3D12Resource> inputResource, uint32_t srvIndex);

	void UpdateImGui();
	/// <summary>
	/// 更新処理
	/// </summary>
	void DisPatch();

public:

	//inputRenderTextureのSRVインデックスを取得
	uint32_t GetInputTextureSrvIndex() const { return inputTexSrvIndex_; }
	//outputRenderTextureのUAVインデックスを取得
	uint32_t GetOutputTextureUavIndex() const { return outputTexUavIndex_; }
	//inputRenderTextureのポインタを取得
	ComPtr<ID3D12Resource> GetInputTextureResource() const { return inputResource_; }
	//outputRenderTextureのポインタを取得
	ComPtr<ID3D12Resource> GetOutputTextureResource() const { return outputResource_; }

private:

	DirectXCommon* dxCommon_ = nullptr; //DirectXCommonのポインタ
	SrvManager* srvManager_ = nullptr; //SrvManagerのポインタ

	//RenderTextureリソース
	ComPtr<ID3D12Resource> inputResource_;
	ComPtr<ID3D12Resource> outputResource_;
	ComPtr<ID3D12Resource> grayScaleTypeResource_;
	int32_t* grayScaleType_;
	uint32_t inputTexSrvIndex_ = 0;
	uint32_t outputTexUavIndex_ = 0;

	//computeパイプライン
	std::unique_ptr<PSO> computePSO_ = nullptr;
	ComPtr<ID3D12RootSignature> rootSignature_ = nullptr;

	std::wstring csFilePath_ = L"";
};