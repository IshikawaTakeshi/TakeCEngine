#pragma once
#include "PostEffect/PostEffect.h"

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
	struct GrayScaleInfo {
		int32_t grayScaleType; // グレースケールの種類
		bool isActive = true; // グレースケールの有効無効
	};
	GrayScaleInfo* grayScaleInfoData_ = nullptr; // グレースケール情報データ
	ComPtr<ID3D12Resource> grayScaleTypeResource_;

};