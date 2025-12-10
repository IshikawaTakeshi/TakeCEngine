#pragma once
#include "engine/PostEffect/PostEffect.h"

class GlareEffect : public PostEffect {
public:
	//========================================================================
	/// functions
	//========================================================================
	/// <summary>
	/// コンストラクタ・デストラクタ
	/// </summary>
	GlareEffect() = default;
	~GlareEffect() = default;
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

private:

	//========================================================================
	/// variables
	//========================================================================
	
	struct GlareEffectInfo {
		float intensity = 1.0f; // グレアの強さ
		float threshold = 1.0f; // グレアの閾値
		int32_t sampleCount = 8; // サンプル数
		float padding; // パディング
	};

	GlareEffectInfo* glareEffectInfoData_ = nullptr; // グレアエフェクト情報データ
	ComPtr<ID3D12Resource> glareEffectInfoResource_; // グレアエフェクト情報リソース
};

