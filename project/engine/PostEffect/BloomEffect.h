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

	void DispatchBrightPass(); // 輝度抽出ディスパッチ
	void DispatchHorizontalBlur(); // 水平ブラーディスパッチ
	void DispatchVerticalBlur(); // 垂直ブラーディスパッチ
	void DispatchComposite(); // 合成ディスパッチ

private:

	// エフェクト情報構造体
	struct BloomEffectInfo {
		bool isActive = false; // エフェクトの有効無効
		float threshold = 0.8f; // 輝度の閾値
		float strength = 1.0f; // 輝度の強さ
		float sigma = 2.0f; // ブラー強度
	};

	std::unique_ptr<PSO> brightPassPSO_; //輝度抽出PSO
	std::unique_ptr<PSO> verticalBlurPSO_; //垂直ブラーPSO
	std::unique_ptr<PSO> horizontalBlurPSO_; //水平ブラーPSO

	ComPtr<ID3D12Resource> brightPassResource_; // 輝度抽出リソース
	ComPtr<ID3D12Resource> verticalBlurResource_; // 垂直ブラーリソース
	ComPtr<ID3D12Resource> horizontalBlurResource_; // 水平ブラーリソース
	ComPtr<ID3D12RootSignature> rootSignatureForBloomTexture_ = nullptr; // ブルームテクスチャ用ルートシグネチャ

	uint32_t brightPassSrvIndex_ = 0; // 輝度抽出SRVインデックス
	uint32_t brightPassUavIndex_ = 0; // 輝度抽出UAVインデックス
	uint32_t verticalBlurSrvIndex_ = 0; // 垂直ブラーSRVインデックス
	uint32_t verticalBlurUavIndex_ = 0; // 垂直ブラーUAVインデックス
	uint32_t horizontalBlurSrvIndex_ = 0; // 水平ブラーSRVインデックス
	uint32_t horizontalBlurUavIndex_ = 0; // 水平ブラーUAVインデックス

	BloomEffectInfo* effectInfoData_ = nullptr; // エフェクト情報データ
	ComPtr<ID3D12Resource> effectInfoResource_; // エフェクト情報リソース

	uint32_t halfWidth_;
	uint32_t halfHeight_;
};