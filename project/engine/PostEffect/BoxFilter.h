#pragma once
#include "PostEffect/PostEffect.h"

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

private:

	// フィルター情報構造体
	struct BoxFilterInfo {
		bool isActive = true; // フィルターの有効無効
	};
	BoxFilterInfo* filterInfoData_ = nullptr; // フィルター情報データ

	ComPtr<ID3D12Resource> filterInfoResource_; // フィルター情報リソース

};