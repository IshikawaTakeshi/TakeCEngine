#pragma once
#include "engine/PostEffect/PostEffect.h"
class BloomEffect : public PostEffect {
public:
	BloomEffect() = default;
	~BloomEffect() = default;
	void Initialize(
		DirectXCommon* dxCommon, SrvManager* srvManager, const std::wstring& CSFilePath,
		ComPtr<ID3D12Resource> inputResource, uint32_t inputSrvIdx, ComPtr<ID3D12Resource> outputResource) override;
	void UpdateImGui() override;
	void Dispatch() override;

private:

	struct BloomEffectInfo {
		bool isActive = false; // エフェクトの有効無効
		float threshold = 0.8f; // 輝度の閾値
		float strength = 1.0f; // 輝度の強さ
		float sigma = 2.0f; // ブラー強度
	};

	BloomEffectInfo* effectInfoData_ = nullptr; // エフェクト情報データ

	ComPtr<ID3D12Resource> effectInfoResource_; // エフェクト情報リソース
};

