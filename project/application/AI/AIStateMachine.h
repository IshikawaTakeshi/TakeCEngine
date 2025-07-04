#pragma once
#include <map>
#include <memory>
#include <string>

class AIState;
class Enemy;
class Player;

/// <summary>
/// AI状態管理システム
/// 各状態の管理と遷移を行う
/// </summary>
class AIStateMachine {
public:
    AIStateMachine() = default;
    ~AIStateMachine() = default;
    
    /// <summary>
    /// 状態機械の更新
    /// </summary>
    /// <param name="enemy">対象の敵</param>
    /// <param name="player">プレイヤー</param>
    /// <param name="deltaTime">フレーム時間</param>
    void Update(Enemy* enemy, Player* player, float deltaTime);
    
    /// <summary>
    /// 状態を変更
    /// </summary>
    /// <param name="stateName">変更先の状態名</param>
    void ChangeState(const std::string& stateName);
    
    /// <summary>
    /// 状態を追加
    /// </summary>
    /// <param name="name">状態名</param>
    /// <param name="state">状態オブジェクト</param>
    void AddState(const std::string& name, std::unique_ptr<AIState> state);
    
    /// <summary>
    /// 現在の状態名を取得
    /// </summary>
    /// <returns>現在の状態名</returns>
    const std::string& GetCurrentStateName() const { return currentStateName_; }

private:
    std::map<std::string, std::unique_ptr<AIState>> states_;
    AIState* currentState_ = nullptr;
    std::string currentStateName_;
    Enemy* lastEnemy_ = nullptr; // 状態遷移時のEnemy参照保持用
};