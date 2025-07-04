#include "AIStateMachine.h"
#include "State/AIState.h"

void AIStateMachine::Update(Enemy* enemy, Player* player, float deltaTime) {
    if (!enemy || !currentState_) return;
    
    // 現在の状態を更新し、次の状態名を取得
    std::string nextStateName = currentState_->Update(enemy, player, deltaTime);
    
    // 状態遷移が必要な場合
    if (!nextStateName.empty() && nextStateName != currentStateName_) {
        ChangeState(nextStateName);
    }
    
    lastEnemy_ = enemy;
}

void AIStateMachine::ChangeState(const std::string& stateName) {
    // 次の状態が存在するかチェック
    auto it = states_.find(stateName);
    if (it == states_.end()) {
        return; // 存在しない状態名の場合は何もしない
    }
    
    // 現在の状態から退出
    if (currentState_ && lastEnemy_) {
        currentState_->Exit(lastEnemy_);
    }
    
    // 新しい状態に遷移
    currentState_ = it->second.get();
    currentStateName_ = stateName;
    
    // 新しい状態に入る
    if (currentState_ && lastEnemy_) {
        currentState_->Enter(lastEnemy_);
    }
}

void AIStateMachine::AddState(const std::string& name, std::unique_ptr<AIState> state) {
    if (state) {
        states_[name] = std::move(state);
        
        // 最初の状態を現在の状態として設定
        if (!currentState_) {
            currentState_ = states_[name].get();
            currentStateName_ = name;
        }
    }
}