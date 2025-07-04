#pragma once
#include <memory>
#include <string>

class ICommand;
class Enemy;
class Player;

/// <summary>
/// AI状態の基底クラス
/// </summary>
class AIState {
public:
    virtual ~AIState() = default;
    
    /// <summary>
    /// 状態に入る時の初期化処理
    /// </summary>
    /// <param name="enemy">状態を持つ敵</param>
    virtual void Enter(Enemy* enemy) = 0;
    
    /// <summary>
    /// 状態の更新処理
    /// </summary>
    /// <param name="enemy">状態を持つ敵</param>
    /// <param name="player">プレイヤー</param>
    /// <param name="deltaTime">フレーム時間</param>
    /// <returns>次の状態名（状態変更がない場合は空文字列）</returns>
    virtual std::string Update(Enemy* enemy, Player* player, float deltaTime) = 0;
    
    /// <summary>
    /// 状態から出る時の終了処理
    /// </summary>
    /// <param name="enemy">状態を持つ敵</param>
    virtual void Exit(Enemy* enemy) = 0;

protected:
    std::unique_ptr<ICommand> currentCommand_;
};