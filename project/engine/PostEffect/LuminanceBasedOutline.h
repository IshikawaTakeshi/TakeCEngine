#pragma once
#include "PostEffect/PostEffect.h"
#include "engine/math/Vector4.h"

//============================================================================
// LuminanceBasedOutline class
//============================================================================
class LuminanceBasedOutline : public PostEffect {
public:

	//=======================================================================
	// functions
	//=======================================================================

	/// <summary>
	/// コンストラクタ・デストラクタ
	/// </summary>
	LuminanceBasedOutline() = default;
	~LuminanceBasedOutline() = default;

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

	void SetIsActive(bool isActive) override {
		if (outlineInfoData_) {
			outlineInfoData_->isActive = isActive;
		}
	}

private:

	// 輝度ベースのアウトライン情報
	struct LuminanceBasedOutlineInfo {
		Vector4 color = { 1.0f, 1.0f, 1.0f, 1.0f }; // アウトラインの色
		float weight = 6.0f; // 輪郭の強さ
		bool isActive = true; // アウトラインの有効無効
	};

	LuminanceBasedOutlineInfo* outlineInfoData_ = nullptr; // アウトライン情報データ
	ComPtr<ID3D12Resource> outlineInfoResource_; // アウトライン情報リソース

};