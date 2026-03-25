#pragma once
#include "PostEffect/PostEffect.h"
#include <wrl.h>

//==============================================================================
// ScanlineEffect class
//==============================================================================
class ScanlineEffect : public PostEffect {
public:
	struct ScanlineParam {
		float intensity = 0.35f;   // 0~1
		float frequency = 420.0f;  // 縞密度
		float speed = 2.0f;    // 流れる速度
		float thickness = 0.35f;   // 線の太さ(0~1)
		float time = 0.0f;
		bool isActive = false; // エフェクトの有効無効
	};

	//=========================================================================
	// functions
	//=========================================================================

	/// <summary>
	/// コンストラクタ・デストラクタ
	/// </summary>
	ScanlineEffect() = default;
	~ScanlineEffect() = default;

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(TakeC::DirectXCommon* dxCommon, TakeC::SrvManager* srvManager,
		const std::wstring& CSFilePath,
		ComPtr<ID3D12Resource> inputResource, uint32_t inputSrvIdx, ComPtr<ID3D12Resource> outputResource) override;

	/// <summary>
	/// ImGui更新処理
	/// </summary>
	void UpdateImGui() override;

	/// <summary>
	/// ディスパッチ
	/// </summary>
	void Dispatch() override;

	// Active状態を設定する
	void SetIsActive(bool isActive) override { param_->isActive = isActive; }
	// 強度を設定する（PlayEffect の onTick で呼ばれる）
	void SetIntensity(float intensity) override { param_->intensity = intensity; }

private:

	/// <summary>
	/// エフェクトパラメータ用のリソースを生成する
	/// </summary>
	void CreateParamResource();

private:
	
	ScanlineParam* param_{};
	Microsoft::WRL::ComPtr<ID3D12Resource> paramResource_;
};