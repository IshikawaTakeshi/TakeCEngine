#pragma once
#include "base/DirectXCommon.h"
#include "base/SrvManager.h"
#include "base/RtvManager.h"
#include "base/PipelineStateObject.h"
#include <string>



class PostEffect {
public:
	PostEffect() = default;
	~PostEffect() = default;

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(DirectXCommon* dxCommon,SrvManager* srvManager,const std::wstring& CSFilePath,const Vector4& clearColor);

	/// <summary>
	/// 更新処理
	/// </summary>
	void DisPatch(uint32_t outputTextureUavIndex);

public:

	uint32_t GetInputTextureSrvIndex() const { return inputTexSrvIndex_; } //inputRenderTextureのSRVインデックスを取得

	uint32_t GetOutputTextureUavIndex() const { return outputTexUavIndex_; } //outputRenderTextureのUAVインデックスを取得


private:

	DirectXCommon* dxCommon_ = nullptr; //DirectXCommonのポインタ
	SrvManager* srvManager_ = nullptr; //SrvManagerのポインタ

	//RenderTextureリソース
	ComPtr<ID3D12Resource> inputResource_;
	ComPtr<ID3D12Resource> outputResource_;
	uint32_t inputTexSrvIndex_ = 0;
	uint32_t outputTexUavIndex_ = 0;

	//computeパイプライン
	std::unique_ptr<PSO> computePSO_ = nullptr;
	ComPtr<ID3D12RootSignature> rootSignature_ = nullptr;

	std::wstring csFilePath_ = L"";
};