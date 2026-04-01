#pragma once
#include "PostEffect/PostEffect.h"

// フィルター情報構造体
struct BoxFilterInfo {
	bool isActive = true; // フィルターの有効無効
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(BoxFilterInfo, isActive)

//============================================================================
// BoxFilter class
//============================================================================
class BoxFilter : public PostEffect {
public:

	//=======================================================================
	/// functions
	//=======================================================================

	/// <summary>
	/// コンストラクタ・デストラクタ
	/// </summary>
	BoxFilter() = default;
	~BoxFilter() = default;

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="dxCommon"></param>
	/// <param name="srvManager"></param>
	/// <param name="CSFilePath"></param>
	/// <param name="inputResource"></param>
	/// <param name="inputSrvIdx"></param>
	/// <param name="outputResource"></param>
	void Initialize(
		TakeC::DirectXCommon* dxCommon, TakeC::SrvManager* srvManager, const std::wstring& CSFilePath,
		ComPtr<ID3D12Resource> inputResource, uint32_t inputSrvIdx,ComPtr<ID3D12Resource> outputResource) override;
	
	/// <summary>
	/// ImGui更新処理
	/// </summary>
	void UpdateImGui() override;
	
	/// <summary>
	/// ディスパッチ
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
		if (filterInfoData_) {
			filterInfoData_->isActive = isActive;
		}
	}

private:

	

	BoxFilterInfo* filterInfoData_ = nullptr; // フィルター情報データ

	ComPtr<ID3D12Resource> filterInfoResource_; // フィルター情報リソース

};