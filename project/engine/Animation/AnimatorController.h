#pragma once
#include "engine/Animation/AnimationState.h"
#include "engine/Animation/Skeleton.h"
#include <vector>
#include <string>

//============================================================================
// AnimatorController class
//============================================================================
class AnimatorController {
public:

	// レイヤー情報構造体
	struct Layer {
		std::string name;
		AnimationState currentState; // 現在のアニメーション
		AnimationState nextState;    // 遷移先のアニメーション
		float blendTimer = 0.0f;
		float blendDuration = 0.0f;
		bool isBlending = false;
		float weight = 1.0f;         // このレイヤーの影響度
		AnimationBlendMode blendMode = AnimationBlendMode::Override; // 合成モード
	};

	//========================================================================
	// functions
	//========================================================================

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="skeleton">スケルトンへのポインタ</param>
	void Initialize(Skeleton* skeleton);

	/// <summary>
	/// レイヤーの追加
	/// </summary>
	void AddLayer(const std::string& name, AnimationBlendMode mode, float weight = 1.0f);

	/// <summary>
	/// レイヤーのウェイト設定
	/// </summary>
	void SetLayerWeight(const std::string& name, float weight);

	/// <summary>
	/// アニメーション遷移の開始（デフォルトで "Base" レイヤーを対象）
	/// </summary>
	void TransitionTo(Animation* animation, float blendDuration, bool isLoop = true);

	/// <summary>
	/// 指定レイヤーのアニメーション遷移
	/// </summary>
	void TransitionTo(const std::string& layerName, Animation* animation, float blendDuration, bool isLoop = true);

	/// <summary>
	/// 更新（スケルトンに全レイヤーを順次適用）
	/// </summary>
	/// <param name="dt">デルタタイム</param>
	void Update(float dt);

private:

	Skeleton* skeleton_ = nullptr;     //スケルトンへの非所有ポインタ
	std::vector<Layer> layers_;        //アニメーションレイヤーのリスト

};
