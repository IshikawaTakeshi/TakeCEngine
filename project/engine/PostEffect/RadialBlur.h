#pragma once
#include "PostEffect/PostEffect.h"
#include "math/Vector2.h"

//RadialBlur情報構造体
struct RadialBlurInfo {
	Vector2 center; // 中心UV座標
	float blurWidth; // ブラーの幅
	bool enable; // ブラーの有効フラグ
};

//============================================================================
//	RadialBlur class
//============================================================================
class RadialBlur : public PostEffect {
public:

	//=======================================================================
	/// functions
	//=======================================================================

	/// <summary>
	/// コンストラクタ・デストラクタ
	/// </summary>
	RadialBlur() = default;
	~RadialBlur() = default;

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

	void SetBlurWidth(float width) { radialBlurInfo_->blurWidth = width; }

	void SetIsActive(bool isActive) override {
		if (radialBlurInfo_) {
			radialBlurInfo_->enable = isActive;
		}
	}

private:

	//ブラーの情報
	RadialBlurInfo* radialBlurInfo_;
	//ブラーの情報を格納するバッファ
	ComPtr<ID3D12Resource> blurInfoResource_ = nullptr;

};