#include "BehaviorDead.h"
#include "engine/io/Input.h"
#include "engine/math/Vector3Math.h"
#include "application/Provider/BaseInputProvider.h"
#include "engine/base/TakeCFrameWork.h"

//===================================================================================
//　コンストラクタ
//===================================================================================
BehaviorDead::BehaviorDead(baseInputProvider* provider) {
	inputProvider_ = provider;
	deltaTime_ = TakeCFrameWork::GetDeltaTime();
}

//===================================================================================
//　初期化
//===================================================================================
void BehaviorDead::Initialize([[maybe_unused]]GameCharacterContext& characterInfo) {

}

//===================================================================================
//　更新
//===================================================================================
void BehaviorDead::Update(GameCharacterContext& characterInfo) {
	if(characterInfo.onGround == false){
		//重力の影響を受ける
		characterInfo.velocity.y -= (gravity_ + characterInfo.fallSpeed) * deltaTime_;
		//位置の更新
		characterInfo.transform.translate += characterInfo.velocity * deltaTime_;

	}
}
