#pragma once
#include "PostEffect/PostEffect.h"


class LuminanceBasedOutline : public PostEffect {
public:

	LuminanceBasedOutline() = default;
	~LuminanceBasedOutline() = default;

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

	struct LuminanceBasedOutlineInfo {
		Vector4 color = { 1.0f, 1.0f, 1.0f, 1.0f }; // アウトラインの色
		float weight = 6.0f; // 輪郭の強さ
		float padding[2]; // パディング（16バイト境界のため）
		bool isActive = true; // アウトラインの有効無効
	};

	LuminanceBasedOutlineInfo* outlineInfoData_ = nullptr; // アウトライン情報データ
	ComPtr<ID3D12Resource> outlineInfoResource_; // アウトライン情報リソース

};

