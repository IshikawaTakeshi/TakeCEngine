#pragma once
#include "engine/base/DirectXCommon.h"
#include "engine/base/SrvManager.h"
#include "engine/base/RtvManager.h"
#include "engine/base/PipelineStateObject.h"
#include <string>

//============================================================
//	PostEffect class
//============================================================
class PostEffect {
public:

	//=========================================================
	// functions
	//=========================================================

	/// <summary>
	/// コンストラクタ
	/// </summary>
	PostEffect() = default;

	/// <summary>
	/// デストラクタ(仮想デストラクタ)
	/// </summary>
	virtual ~PostEffect() = default;

	/// <summary>
	/// 初期化
	/// </summary>
	virtual void Initialize(TakeC::DirectXCommon* dxCommon, TakeC::SrvManager* srvManager,const std::wstring& CSFilePath,
		ComPtr<ID3D12Resource> inputResource, uint32_t inputSrvIdx,ComPtr<ID3D12Resource> outputResource);

	/// <summary>
	/// ImGui更新処理
	/// </summary>
	virtual void UpdateImGui() = 0;
	/// <summary>
	/// 更新処理
	/// </summary>
	virtual void Dispatch() = 0;

	virtual void SetIsActive(bool isActive) = 0;

public:
	//=========================================================
	// accessors
	//=========================================================
	
	//----- getter ----------------------------
	
	//inputRenderTextureのSRVインデックスを取得
	uint32_t GetInputTextureSrvIndex() const { return inputTexSrvIndex_; }
	//outputRenderTextureのUAVインデックスを取得
	uint32_t GetOutputTextureUavIndex() const { return outputTexUavIndex_; }
	//outputRenderTextureのSRVインデックスを取得
	uint32_t GetOutputTextureSrvIndex() const { return outputTexSrvIndex_; }
	//inputRenderTextureのポインタを取得
	ComPtr<ID3D12Resource> GetInputTextureResource() const { return inputResource_; }
	//outputRenderTextureのポインタを取得
	ComPtr<ID3D12Resource> GetOutputTextureResource() const { return outputResource_; }

	//----- setter ----------------------------

	//深度テクスチャリソースとSRVインデックスを設定
	void SetDepthTextureResource(ComPtr<ID3D12Resource> depthTextureResource, uint32_t depthTextureSrvIndex) {
		depthTextureResource_ = depthTextureResource;
		depthTextureSrvIndex_ = depthTextureSrvIndex;
	}
	//ライトカメラ深度テクスチャリソースとSRVインデックスを設定
	void SetLightCameraDepthTextureResource(ComPtr<ID3D12Resource> lightCameraDepthTextureResource, uint32_t lightCameraDepthTextureSrvIndex) {
		lightCameraDepthTextureResource_ = lightCameraDepthTextureResource;
		lightCameraDepthTextureSrvIndex_ = lightCameraDepthTextureSrvIndex;
	}
protected:

	TakeC::DirectXCommon* dxCommon_ = nullptr; //DirectXCommonのポインタ
	TakeC::SrvManager* srvManager_ = nullptr; //SrvManagerのポインタ

	//RenderTextureリソース
	ComPtr<ID3D12Resource> inputResource_;
	ComPtr<ID3D12Resource> outputResource_;
	uint32_t inputTexSrvIndex_ = 0;
	uint32_t outputTexUavIndex_ = 0;
	uint32_t outputTexSrvIndex_ = 0;

	//computeパイプライン
	std::unique_ptr<PSO> computePSO_ = nullptr;
	ComPtr<ID3D12RootSignature> rootSignature_ = nullptr;

	//CSファイルパス
	std::wstring csFilePath_ = L"";

	ComPtr<ID3D12Resource> depthTextureResource_; // 深度テクスチャリソース
	uint32_t depthTextureSrvIndex_ = 0;           // 深度テクスチャのDSVインデックス

	ComPtr<ID3D12Resource> lightCameraDepthTextureResource_; // ライトカメラ深度テクスチャリソース
	uint32_t lightCameraDepthTextureSrvIndex_ = 0;           // ライトカメラ深度テクスチャのDSVインデックス
};