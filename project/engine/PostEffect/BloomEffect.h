#pragma once
#include "engine/PostEffect/PostEffect.h"

//============================================================
//	BloomEffect class
//============================================================
class BloomEffect : public PostEffect {
public:

	//=========================================================
	// functions
	//=========================================================

	/// <summary>
	/// コンストラクタ・デストラクタ
	/// </summary>
	BloomEffect() = default;
	~BloomEffect() = default;

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
		DirectXCommon* dxCommon, SrvManager* srvManager, const std::wstring& CSFilePath,
		ComPtr<ID3D12Resource> inputResource, uint32_t inputSrvIdx, ComPtr<ID3D12Resource> outputResource) override;

	/// <summary>
	/// ImGui更新処理
	/// </summary>
	void UpdateImGui() override;
	
	/// <summary>
	/// ディスパッチ
	/// </summary>
	void Dispatch() override;

private:

	// エフェクト情報構造体
	struct BloomEffectInfo {
		bool isActive = false; // エフェクトの有効無効
		float threshold = 0.8f; // 輝度の閾値
		float strength = 1.0f; // 輝度の強さ
		float sigma = 2.0f; // ブラー強度
	};

	BloomEffectInfo* effectInfoData_ = nullptr; // エフェクト情報データ

	ComPtr<ID3D12Resource> effectInfoResource_; // エフェクト情報リソース
};