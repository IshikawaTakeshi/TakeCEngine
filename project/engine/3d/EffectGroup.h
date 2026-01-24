#pragma once
#include <map>
#include <string>
#include <memory>
#include "engine/Utility/Timer.h"
#include "engine/3d/EffectGroupConfig.h"

class ParticleEmitter;

namespace TakeC {

	//====================================================================
	// particleエフェクト群クラス
	//====================================================================
	class EffectGroup {

	public:
		EffectGroup() = default;
		~EffectGroup() = default;
		// 初期化
		void Initialize();
		// 終了処理
		void Finalize();
		// 更新処理
		void Update();
		// 描画処理
		void Draw();

		void RegisterParticlePreset(const std::string& presetName);

	private:

		// particleエミッタ群
		std::map<std::string, std::unique_ptr<ParticleEmitter>> particleEmitters_;

		EffectGroupConfig effectConfig_;  // エフェクトグループ設定

		// エフェクト再生中フラグ
		bool isActive_ = false;       // 実行中フラグ
		bool isCompleted_ = false;    // 完了フラグ
		Timer effectDelayTimer_;    // エフェクト遅延タイマー
	};
}