#pragma once
#include "engine/PostEffect/PostEffect.h"
class DepthBasedOutline : public PostEffect {
public:

	DepthBasedOutline() = default;
	~DepthBasedOutline() = default;
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(DirectXCommon* dxCommon, SrvManager* srvManager, const std::wstring& CSFilePath,
		ComPtr<ID3D12Resource> inputResource, uint32_t inputSrvIdx, ComPtr<ID3D12Resource> outputResource) override;
	void UpdateImGui() override;
	/// <summary>
	/// 更新処理
	/// </summary>
	void DisPatch() override;

private:

	// 深度ベースのアウトライン情報
	struct DepthBasedOutlineInfo {
		float weight = 1.0f; // 輪郭の強さ
		bool isActive = true; // アウトラインの有効無効
		float padding; // パディング
	};

	DepthBasedOutlineInfo* outlineInfoData_ = nullptr; // アウトライン情報データ
	ComPtr<ID3D12Resource> outlineInfoResource_; // アウトライン情報リソース
	uint32_t depthTextureSrvIndex_ = 0; // 深度テクスチャのSRVインデックス
	ComPtr<ID3D12Resource> depthTextureResource_; // 深度テクスチャリソース
};

