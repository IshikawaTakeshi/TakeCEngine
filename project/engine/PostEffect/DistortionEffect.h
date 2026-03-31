#pragma once
#include "PostEffect/PostEffect.h"
#include "math/Vector2.h"

//=============================================================================
// DistortionEffect パラメータ構造体
//=============================================================================

// ノイズベースの歪み情報 (16バイトアライメント)
struct DistortionInfo {
	float strength;
	float timer;
	Vector2 scrollSpeed;
	Vector2 noiseScale;
	Vector2 offset;
	bool enable;
	float padding[3];
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(DistortionInfo,
	strength,
	timer,
	scrollSpeed,
	noiseScale,
	offset,
	enable
)

//=============================================================================
// DistortionEffect class
//=============================================================================

/**
 * @brief ノイズテクスチャベースの画面歪みエフェクト
 */
	class DistortionEffect : public PostEffect {
	public:

		//=======================================================================
		/// functions
		//=======================================================================

		DistortionEffect() = default;
		~DistortionEffect() = default;

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
		/// 強度を設定する（PlayEffect の onTick で呼ばれる）
		/// </summary>
		/// <param name="intensity"></param>
		void SetIntensity(float intensity) override;

	public:
		//=========================================================
		// accessors
		//=========================================================

		void SetStrength(float strength) { distortionInfo_->strength = strength; }

		void SetIsActive(bool isActive) override {
				distortionInfo_->enable = isActive;
		}

	private:

		// 歪み情報データ（Mappedポインタ）
		DistortionInfo* distortionInfo_ = nullptr;
		// 歪み情報の定数バッファ
		ComPtr<ID3D12Resource> distortionInfoResource_ = nullptr;
		// ノイズテクスチャのファイルパス
		std::string noiseTextureFilePath_;
};