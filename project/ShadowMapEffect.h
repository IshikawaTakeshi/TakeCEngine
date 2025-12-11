#pragma once
#include "engine/PostEffect/PostEffect.h"

//============================================================================
// ShadowMapEffect class
//============================================================================
class ShadowMapEffect : public PostEffect {
	public:
		//=========================================================
		// functions
		//=========================================================

		ShadowMapEffect() = default;
		~ShadowMapEffect() override = default;

		void Initialize(TakeC::DirectXCommon* dxCommon, TakeC::SrvManager* srvManager,
			const std::wstring& CSFilePath,
			ComPtr<ID3D12Resource> inputResource, uint32_t inputSrvIdx,
			ComPtr<ID3D12Resource> outputResource) override;


		/// <summary>
		/// ImGui更新処理
		/// </summary>
		void UpdateImGui() override;
		/// <summary>
		/// 更新処理
		/// </summary>
		void Dispatch() override;

private:

	// シャドウマップエフェクト情報構造体
		struct ShadowMapEffectInfo {
			bool isActive = false; // エフェクトの有効無効
			float bias = 0.005f; // シャドウのバイアス値
			float normalBias = 0.05f; // シャドウのノーマルバイアス値
			float pcfRange = 1.0f; // テクセル半径	
		};
		ShadowMapEffectInfo* shadowMapEffectInfo_ = nullptr; // シャドウマップエフェクト情報データ
		ComPtr<ID3D12Resource> shadowMapEffectInfoResource_; // シャドウマップエフェクト情報リソース

		uint32_t depthTextureSrvIndex_ = 0;           // 深度テクスチャのSRVインデックス
		ComPtr<ID3D12Resource> depthTextureResource_; // 深度テクスチャリソース
};

