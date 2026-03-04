#pragma once
#include "application/Scene/SceneState/BaseSceneState.h"
#include <memory>
#include <unordered_map>

// 前方宣言
class BaseScene;

//============================================================================
// SceneStateManager class
// シーン非依存のステートマネージャー
//============================================================================
class SceneStateManager {
public:
  SceneStateManager() = default;
  ~SceneStateManager() = default;

  /// <summary>
  /// 状態を登録
  /// </summary>
  /// <param name="type">状態の種類</param>
  /// <param name="state">状態の実体</param>
  void RegisterState(SceneState type, std::unique_ptr<BaseSceneState> state);

  /// <summary>
  /// 初期状態を設定して初期化
  /// </summary>
  /// <param name="initialState">初期状態</param>
  /// <param name="scene">シーンポインタ</param>
  void Initialize(SceneState initialState, BaseScene *scene);

  /// <summary>
  /// 更新処理（遷移チェック＋現在ステートの更新）
  /// </summary>
  /// <param name="scene">シーンポインタ</param>
  void Update(BaseScene *scene);

  //----- getter ---------------------------

  /// <summary>
  /// 現在の状態タイプを取得
  /// </summary>
  SceneState GetCurrentStateType() const { return currentStateType_; }

private:
  /// <summary>
  /// 指定した状態へ遷移
  /// </summary>
  void TransitionTo(SceneState nextState, BaseScene *scene);

private:
  // 状態マップ
  std::unordered_map<SceneState, std::unique_ptr<BaseSceneState>> states_;
  // 現在の状態
  BaseSceneState *currentState_ = nullptr;
  // 現在の状態タイプ
  SceneState currentStateType_ = SceneState::GAMESTART;
};
