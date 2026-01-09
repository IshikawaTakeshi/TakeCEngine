#pragma once
#include "engine/math/Vector2.h"
#include "engine/math/Vector3.h"
#include "engine/Utility/Timer.h"
#include "engine/Animation/SpriteSheetSttings.h"
#include <cstdint>
#include <vector>

class Material;

namespace TakeC {

	//============================================================================
	// テクスチャアニメーションの種類
	//============================================================================
	enum class TextureAnimationType {
		None,           // アニメーションなし
		UVScroll,       // UVスクロール
		SpriteSheet,    // スプライトシート
	};

	

	//============================================================================
	// UVスクロール設定
	//============================================================================
	struct UVScrollSettings {
		Vector2 scrollSpeed = { 0.0f, 0.0f }; // スクロール速度（UV単位/秒）
		bool wrapU = true;  // U座標をラップするか
		bool wrapV = true;  // V座標をラップするか
	};

	//============================================================================
	// UVTextureAnimation class
	//============================================================================

	class UVTextureAnimation {
		public:
		//========================================================================
		// functions
		//========================================================================
		/// <summary>
		/// コンストラクタ・デストラクタ
		/// </summary>
		UVTextureAnimation() = default;
		~UVTextureAnimation() = default;

		/// <summary>
		/// 初期化
		/// </summary>
		void Initialize(Material* material);

		/// <summary>
		/// 更新処理
		/// </summary>
		/// <param name="deltaTime">経過時間（秒）</param>
		void Update(float deltaTime);

		void UpdateImGui(const std::string& label);

		/// <summary>
		/// アニメーションをリセット
		/// </summary>
		void Reset();

		void Restart();

		/// <summary>
		/// アニメーションを開始
		/// </summary>
		void Play();

		/// <summary>
		/// アニメーションを一時停止
		/// </summary>
		void Pause();

		/// <summary>
		/// アニメーションを停止
		/// </summary>
		void Stop();

		//========================================================================
		// UVスクロール設定
		//========================================================================

		/// <summary>
		/// UVスクロールアニメーションを設定
		/// </summary>
		void SetUVScrollAnimation(const Vector2& scrollSpeed);
		void SetUVScrollAnimation(const UVScrollSettings& settings);

		//========================================================================
		// スプライトシート設定
		//========================================================================

		/// <summary>
		/// スプライトシートアニメーションを設定
		/// </summary>
		void SetSpriteSheetAnimation(uint32_t columns, uint32_t rows, 
			uint32_t totalFrames, float frameDuration, bool loop = true);
		void SetSpriteSheetAnimation(const SpriteSheetSettings& settings);

		//========================================================================
		// accessors
		//========================================================================

		// アニメーションタイプの取得
		TextureAnimationType GetAnimationType() const { return animationType_; }

		// 再生中かどうか
		bool IsPlaying() const;

		// アニメーションが完了したか（ループしない場合）
		bool IsFinished() const;

		// 現在のUVオフセットを取得
		Vector2 GetCurrentUVOffset() const { return currentUVOffset_; }

		// 現在のUVスケールを取得
		Vector2 GetCurrentUVScale() const { return currentUVScale_; }

	private: 

		//========================================================================
		// private functions
		//========================================================================

		/// <summary>
		/// UVスクロールの更新
		/// </summary>
		void UpdateUVScroll();

		/// <summary>
		/// スプライトシートの更新
		/// </summary>
		void UpdateSpriteSheet();

		/// <summary>
		/// マテリアルにUV変換を適用
		/// </summary>
		void ApplyToMaterial();

	private:

		//========================================================================
		// member variables
		//========================================================================

		// 対象のマテリアル
		Material* material_ = nullptr;

		// アニメーションタイプ
		TextureAnimationType animationType_ = TextureAnimationType:: None;

		// フレーム管理用タイマー
		Timer frameTimer_;

		// UVスクロール用の累積時間タイマー（ループ用）
		Timer scrollTimer_;

		// 現在のUV状態
		Vector2 currentUVOffset_ = { 0.0f, 0.0f };
		Vector2 currentUVScale_ = { 1.0f, 1.0f };

		// UVスクロール設定
		UVScrollSettings uvScrollSettings_;

		// スプライトシート設定
		SpriteSheetSettings spriteSheetSettings_;

		// スプライトシート用タイマー
		Timer spriteSheetTimer_;
		uint32_t currentFrame_ = 0;
	};
}