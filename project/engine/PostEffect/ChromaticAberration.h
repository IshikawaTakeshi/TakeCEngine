#pragma once
#include "Posteffect/PostEffect.h"
#include "engine/math/Vector2.h"
#include <json.hpp>

// 色収差情報構造体
struct ChromaticAberrationInfo {
	Vector2 center = { 0.5f, 0.5f }; // 中心位置(UV)
	float intensity = 1.0f;                 // エフェクト強度
	float redScale = 1.0f;                  // 赤の放射状ズレ倍率
	float greenScale = 1.0f;                // 緑の放射状ズレ倍率
	float blueScale = 1.0f;                 // 青の放射状ズレ倍率
	int32_t isActive = 0;                   // 有効無効フラグ (hlslでboolとして扱う)
	float padding;                          // 16バイトアライメント用 (32 bytes total)
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(
	ChromaticAberrationInfo,
	center,
	intensity,
	redScale,
	greenScale,
	blueScale,
	isActive)


//============================================================================
// ChromaticAberration class
//============================================================================
class ChromaticAberration : public PostEffect {
public:

	//=======================================================================
	/// functions
	//=======================================================================

	/// <summary>
	/// コンストラクタ・デストラクタ
	/// </summary>
	ChromaticAberration() = default;
	~ChromaticAberration() = default;

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

	
	// 強度を設定する
	void SetIntensity(float intensity) override;

	/// <summary>
	/// エフェクト固有のパラメータを適用する
	/// </summary>
	void ApplySpecificParams(const nlohmann::json& params) override;

	/// <summary>
	/// エフェクト固有のパラメータを取得する
	/// </summary>
	nlohmann::json GetSpecificParams() const override;

public:
	//=========================================================
	// accessors
	//=========================================================

	void SetCenter(const Vector2& center) { chromaticAberrationInfoData_->center = center; }
	void SetRedScale(float scale) { chromaticAberrationInfoData_->redScale = scale; }
	void SetGreenScale(float scale) { chromaticAberrationInfoData_->greenScale = scale; }
	void SetBlueScale(float scale) { chromaticAberrationInfoData_->blueScale = scale; }

	void SetIsActive(bool isActive) override {
		if (chromaticAberrationInfoData_) {
			chromaticAberrationInfoData_->isActive = isActive ? 1 : 0;
		}
	}

private:

	// 情報データ
	ChromaticAberrationInfo* chromaticAberrationInfoData_ = nullptr;
	// 情報リソース
	ComPtr<ID3D12Resource> chromaticAberrationInfoResource_;
};
