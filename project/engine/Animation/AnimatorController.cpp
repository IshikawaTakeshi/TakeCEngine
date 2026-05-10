#include "AnimatorController.h"
#include <algorithm>

//====================================================================
// 初期化
//====================================================================
void AnimatorController::Initialize(Skeleton* skeleton) {
	skeleton_ = skeleton;
	layers_.clear();
	// デフォルトレイヤーを追加
	AddLayer("Base", AnimationBlendMode::Override, 1.0f);
}

//====================================================================
// レイヤーの追加
//====================================================================
void AnimatorController::AddLayer(const std::string& name, AnimationBlendMode mode, float weight) {
	Layer layer;
	layer.name = name;
	layer.blendMode = mode;
	layer.weight = weight;
	layers_.push_back(layer);
}

//====================================================================
// レイヤーのウェイト設定
//====================================================================
void AnimatorController::SetLayerWeight(const std::string& name, float weight) {
	for (auto& layer : layers_) {
		if (layer.name == name) {
			layer.weight = weight;
			return;
		}
	}
}

//====================================================================
// アニメーション遷移の開始（デフォルト）
//====================================================================
void AnimatorController::TransitionTo(Animation* animation, float blendDuration, bool isLoop) {
	TransitionTo("Base", animation, blendDuration, isLoop);
}

//====================================================================
// 指定レイヤーのアニメーション遷移
//====================================================================
void AnimatorController::TransitionTo(const std::string& layerName, Animation* animation, float blendDuration, bool isLoop) {
	if (!animation) return;

	for (auto& layer : layers_) {
		if (layer.name == layerName) {
			if (!layer.currentState.IsValid()) {
				layer.currentState.animation = animation;
				layer.currentState.isLoop = isLoop;
				layer.currentState.Reset();
				layer.isBlending = false;
			} else {
				layer.nextState.animation = animation;
				layer.nextState.isLoop = isLoop;
				layer.nextState.Reset();
				layer.blendDuration = std::max(blendDuration, 0.0001f);
				layer.blendTimer = 0.0f;
				layer.isBlending = true;
			}
			return;
		}
	}
}

//====================================================================
// 更新
//====================================================================
void AnimatorController::Update(float dt) {
	if (!skeleton_) return;

	// スケルトンの状態をリセット
	skeleton_->ClearTransform();

	// 全レイヤーを順番に適用
	for (auto& layer : layers_) {
		if (!layer.currentState.IsValid()) continue;

		layer.currentState.Advance(dt);

		if (layer.isBlending) {
			layer.nextState.Advance(dt);
			layer.blendTimer += dt;
			float t = std::clamp(layer.blendTimer / layer.blendDuration, 0.0f, 1.0f);

			// レイヤー内部でのクロスフェード
			// 現在の状態を反映（weightを考慮）
			skeleton_->ApplyLayeredAnimation(layer.currentState.animation, layer.currentState.time, layer.weight * (1.0f - t), layer.blendMode);
			// 次の状態を反映
			skeleton_->ApplyLayeredAnimation(layer.nextState.animation, layer.nextState.time, layer.weight * t, layer.blendMode);

			if (t >= 1.0f) {
				layer.currentState = layer.nextState;
				layer.nextState = {};
				layer.isBlending = false;
			}
		} else {
			// 単一アニメーションの適用
			skeleton_->ApplyLayeredAnimation(layer.currentState.animation, layer.currentState.time, layer.weight, layer.blendMode);
		}
	}

	// スケルトンの行列計算を更新
	skeleton_->Update();
}
