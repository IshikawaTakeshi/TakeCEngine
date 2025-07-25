#pragma once
#include "PostEffect/PostEffect.h"
class GrayScale : public PostEffect {
public:
	GrayScale() = default;
	~GrayScale() = default;
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(DirectXCommon* dxCommon, SrvManager* srvManager, const std::wstring& CSFilePath,
		ComPtr<ID3D12Resource> inputResource, uint32_t inputSrvIdx,ComPtr<ID3D12Resource> outputResource) override;
	
	
	void UpdateImGui() override;
	/// <summary>
	/// 更新処理
	/// </summary>
	void DisPatch() override;

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

