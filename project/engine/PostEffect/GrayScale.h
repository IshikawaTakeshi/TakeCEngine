#pragma once
#include "PostEffect/PostEffect.h"

struct GrayScaleInfo {
	int32_t grayScaleType; // グレースケールの種類
	bool isActive = true; // グレースケールの有効無効
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(GrayScaleInfo, grayScaleType, isActive)

//============================================================================
//	GrayScale class
//============================================================================
class GrayScale : public PostEffect {
public:

	//=======================================================================
	/// functions
	//=======================================================================

	/// <summary>
	/// コンストラクタ・デストラクタ
	/// </summary>
	GrayScale() = default;
	~GrayScale() = default;
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(TakeC::DirectXCommon* dxCommon, TakeC::SrvManager* srvManager, const std::wstring& CSFilePath,
		ComPtr<ID3D12Resource> inputResource, uint32_t inputSrvIdx,ComPtr<ID3D12Resource> outputResource) override;
	
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

	/// <summary>
	/// アクティブ状態を設定する
	/// </summary>
	/// <param name="isActive"></param>
	void SetIsActive(bool isActive) override {
		if (grayScaleInfoData_) {
			grayScaleInfoData_->isActive = isActive;
		}
	}

private:

	//グレースケールの種類
	enum class GrayScaleType {
		GRAYSCALE,
		SEPIA,
		NONE,
	};
	
	GrayScaleInfo* grayScaleInfoData_ = nullptr; // グレースケール情報データ
	ComPtr<ID3D12Resource> grayScaleTypeResource_;

};