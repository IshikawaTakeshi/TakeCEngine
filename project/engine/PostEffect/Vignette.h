#pragma once
#include "Posteffect/PostEffect.h"

//Vignette情報構造体
struct VignetteInfo {
	float vignetteScale = 0.0f; //中心から外側への減衰の速さ
	float vignettePower = 0.0f; //Vignetteの強さ
	bool isActive = false; //Vignetteの有効無効
};

//============================================================================
// Vignette class
//============================================================================
class Vignette : public PostEffect {
public:

	//=======================================================================
	/// functions
	//=======================================================================

	/// <summary>
	/// コンストラクタ・デストラクタ
	/// </summary>
	Vignette() = default;
	~Vignette() = default;
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(TakeC::DirectXCommon* dxCommon, TakeC::SrvManager* srvManager, const std::wstring& CSFilePath,
		ComPtr<ID3D12Resource> inputResource, uint32_t inputSrvIdx, ComPtr<ID3D12Resource> outputResource) override;
	
	/// <summary>
	/// ImGui更新処理
	/// </summary>
	void UpdateImGui() override;
	/// <summary>
	/// 更新処理
	/// </summary>
	void Dispatch() override;

	/**
	 * @brief 強度を設定する
	 * @param intensity 0.0〜1.0 (1.0でプリセットの最大強度)
	 */
	void SetIntensity(float intensity) override;

public:
	//=========================================================
	// accessors
	//=========================================================

	void SetVignetteScale(float scale) { vignetteInfoData_->vignetteScale = scale; }
	void SetVignettePower(float power) { vignetteInfoData_->vignettePower = power; }

	void SetIsActive(bool isActive) override {
		if (vignetteInfoData_) {
			vignetteInfoData_->isActive = isActive;
		}
	}

private:

	//  Vignette情報データ
	VignetteInfo* vignetteInfoData_ = nullptr;
	//  Vignette情報リソース
	ComPtr<ID3D12Resource> vignetteInfoResource_;
};