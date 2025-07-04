#include "AttackCommand.h"
#include "Entity/GameCharacter.h"
#include "Entity/Enemy/Enemy.h"
#include "Weapon/BaseWeapon.h"

AttackCommand::AttackCommand()
    : isComplete_(false), hasAttacked_(false) {
}

void AttackCommand::Execute(GameCharacter* character, float deltaTime) {
    if (!character || hasAttacked_) {
        isComplete_ = true;
        return;
    }
    
    // 敵の場合、武器での攻撃を実行
    if (Enemy* enemy = dynamic_cast<Enemy*>(character)) {
        BaseWeapon* weapon = enemy->GetWeapon();
        if (weapon) {
            weapon->Attack();
            hasAttacked_ = true;
        }
        isComplete_ = true;
    }
}

bool AttackCommand::IsComplete() const {
    return isComplete_;
}

void AttackCommand::Reset() {
    isComplete_ = false;
    hasAttacked_ = false;
}