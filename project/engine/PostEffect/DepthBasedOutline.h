#pragma once
#include "engine/PostEffect/PostEffect.h"
#include "engine/camera/CameraForGPU.h"

//=============================================================================
//	DepthBasedOutline class
//=============================================================================
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
	void Dispatch() override;

private:

	// 深度ベースのアウトライン情報
	struct DepthBasedOutlineInfo {
		Vector4 color = { 0.0f, 0.0f, 0.0f, 1.0f }; // アウトラインの色
		float weight = 1.0f;                        // 輪郭の強さ
		float distantSensitivity;                   //遠方オブジェクトの感度調整係数
		float distantStart;                         //遠方補正を始めるviewZ
		float distantEnd;                           //補正を最大にするviewZ
		bool isActive = true;                       // アウトラインの有効無効
	};

	DepthBasedOutlineInfo* outlineInfoData_ = nullptr; // アウトライン情報データ
	ComPtr<ID3D12Resource> outlineInfoResource_;       // アウトライン情報リソース

	uint32_t depthTextureSrvIndex_ = 0;           // 深度テクスチャのSRVインデックス
	ComPtr<ID3D12Resource> depthTextureResource_; // 深度テクスチャリソース
};

