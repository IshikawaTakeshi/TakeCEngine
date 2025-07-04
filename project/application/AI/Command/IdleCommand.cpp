#include "IdleCommand.h"
#include "Entity/GameCharacter.h"

void IdleCommand::Execute(GameCharacter* character, float deltaTime) {
    // 待機中は何もしない
    // 実際には、待機アニメーションや小さな動作を追加できる
    isComplete_ = false; // 待機は基本的に継続する
}

bool IdleCommand::IsComplete() const {
    return isComplete_;
}

void IdleCommand::Reset() {
    isComplete_ = false;
}