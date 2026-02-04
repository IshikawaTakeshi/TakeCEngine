#pragma once
#include "engine/3d/Particle/EffectGroupConfig.h"
#include "engine/math/Vector3.h"
#include "engine/Animation/Skeleton.h"
#include <string>
//============================================================================
// EffectGroup class
//============================================================================

namespace TakeC {

	class EffectGroup {
	public:

		EffectGroup() = default;
		~EffectGroup() = default;

		//======================================================================
		// 基本機能
		//======================================================================

		/// <summary>
		/// JSONファイルから初期化
		/// </summary>
		void Initialize(const std::string& configFilePath);

		/// <summary>
		/// 設定から初期化
		/// </summary>
		void Initialize(const EffectGroupConfig& config);

		/// <summary>
		/// 更新処理
		/// </summary>
		void Update();

		/// <summary>
		/// ImGui更新処理
		/// </summary>
		void UpdateImGui();

		//======================================================================
		// エフェクト制御
		//======================================================================

		/// <summary>
		/// エフェクトを再生（ワンショット）
		/// </summary>
		void Play(const Vector3& position);

		/// <summary>
		/// エフェクトを停止
		/// </summary>
		void Stop();

		/// <summary>
		/// エフェクトを一時停止
		/// </summary>
		void Pause();

		/// <summary>
		/// エフェクトを再開
		/// </summary>
		void Resume();

		/// <summary>
		/// エフェクトをリセット（最初から）
		/// </summary>
		void Reset();

		//======================================================================
		// トランスフォーム設定
		//======================================================================

		/// <summary>
		/// エフェクト全体の位置を設定
		/// </summary>
		void SetPosition(const Vector3& position);

		/// <summary>
		/// エフェクト全体の回転を設定
		/// </summary>
		void SetRotation(const Vector3& rotation);

		/// <summary>
		/// エフェクト全体のスケールを設定
		/// </summary>
		void SetScale(const Vector3& scale);

		/// <summary>
		/// エフェクトの向きを設定（発生方向）
		/// </summary>
		void SetDirection(const Vector3& direction);

		/// <summary>
		/// スケルトンのジョイントにアタッチ
		/// </summary>
		void AttachToSkeletonJoint(Skeleton* skeleton, const std::string& jointName);

		//======================================================================
		// 状態取得
		//======================================================================

		/// <summary>
		/// エフェクトが再生中か
		/// </summary>
		bool IsPlaying() const { return isPlaying_; }

		/// <summary>
		/// エフェクトが終了したか
		/// </summary>
		bool IsFinished() const { return isFinished_; }

		/// <summary>
		/// エフェクトが一時停止中か
		/// </summary>
		bool IsPaused() const { return isPaused_; }

		/// <summary>
		/// ループ再生設定の取得
		/// </summary>
		bool IsLooping() const { return config_.isLooping; }

		//======================================================================
		// 詳細設定
		//======================================================================

		/// <summary>
		/// 特定のエミッターを有効/無効化
		/// </summary>
		void SetEmitterActive(const std::string& emitterName, bool isActive);

		/// <summary>
		/// 特定のエミッターの遅延時間を設定
		/// </summary>
		void SetEmitterDelay(const std::string& emitterName, float delay);

		/// <summary>
		/// ループ再生を設定
		/// </summary>
		void SetLooping(bool isLooping) { config_.isLooping = isLooping; }

		/// <summary>
		/// エミッター個別の位置オフセットを設定
		/// </summary>
		void SetEmitterOffset(const std::string& emitterName, const Vector3& offset);

		void SetEmitterRotation(const std::string& emitterName, const Vector3& rotation);

		//======================================================================
		// Getter
		//======================================================================

		const std::string& GetEffectName() const { return config_.effectName; }
		const Vector3& GetPosition() const { return position_; }
		const Vector3& GetRotation() const { return rotation_; }
		const Vector3& GetScale() const { return scale_; }
		float GetElapsedTime() const { return totalElapsedTime_; }

	private:

		//======================================================================
		// 内部処理
		//======================================================================

		/// <summary>
		/// 共通初期化処理
		/// </summary>
		void InitializeCommon();

		/// <summary>
		/// エミッターインスタンスを作成
		/// </summary>
		void CreateEmitterInstances();

		/// <summary>
		/// 各エミッターの位置を更新
		/// </summary>
		void UpdateEmitterPositions();

		/// <summary>
		/// ジョイント追従の更新
		/// </summary>
		void UpdateJointAttachment();

		/// <summary>
		/// エミッターの開始判定
		/// </summary>
		void CheckEmitterStart(EmitterInstance& instance);

		/// <summary>
		/// エミッターの終了判定
		/// </summary>
		void CheckEmitterEnd(EmitterInstance& instance);

		//======================================================================
		// メンバ変数
		//======================================================================

		// 設定
		EffectGroupConfig config_;

		// エミッターインスタンス
		std::vector<EmitterInstance> emitterInstances_;

		// トランスフォーム
		Vector3 position_ = {0.0f, 0.0f, 0.0f};
		Vector3 rotation_ = {0.0f, 0.0f, 0.0f};
		Vector3 scale_ = {1.0f, 1.0f, 1.0f};
		Vector3 direction_ = {0.0f, 1.0f, 0.0f}; // デフォルトは上向き

		// 状態
		bool isPlaying_ = false;
		bool isFinished_ = false;
		bool isPaused_ = false;
		float totalElapsedTime_ = 0.0f;

		// ジョイント追従
		Skeleton* attachedSkeleton_ = nullptr;
		std::string attachedJointName_;

		// デルタタイム
		float deltaTime_;
	};
}


