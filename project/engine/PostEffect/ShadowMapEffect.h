#pragma once
#include "engine/PostEffect/PostEffect.h"

// シャドウマップエフェクト情報構造体
struct ShadowMapEffectInfo {
	bool isActive = false; // エフェクトの有効無効
	float bias = 0.005f; // シャドウのバイアス値
	float pcfRange = 1.0f; // テクセル半径	
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ShadowMapEffectInfo, isActive, bias, pcfRange)

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

	/// <summary>
	/// エフェクト固有のパラメータを適用する
	/// </summary>
	/// <param name="params"></param>
	void ApplySpecificParams(const nlohmann::json& params) override;

	/// <summary>
	/// エフェクト固有のパラメータを取得する
	/// </summary>
	/// <returns></returns>
	nlohmann::json GetSpecificParams() const override;

	void SetIsActive(bool isActive) override {
		if (shadowMapEffectInfo_) {
			shadowMapEffectInfo_->isActive = isActive;
		}
	}

private:

	ShadowMapEffectInfo* shadowMapEffectInfo_ = nullptr; // シャドウマップエフェクト情報データ
	ComPtr<ID3D12Resource> shadowMapEffectInfoResource_; // シャドウマップエフェクト情報リソース
	ComPtr<ID3D12Resource> lightCameraInfoResource_; // ライトカメラ情報リソース

};