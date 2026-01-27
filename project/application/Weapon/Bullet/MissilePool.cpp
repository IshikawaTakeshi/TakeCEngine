#include "MissilePool.h"
#include "engine/Base/TakeCFrameWork.h"

//=============================================================================================
// 型ごとの初期化処理
//=============================================================================================

void MissilePool::OnInitializeObject(VerticalMissile& object,const std::string& modelFilePath,const BulletEffectConfig& effectConfig) {

	effectConfig_ = effectConfig;

	//初期化処理
	object.Initialize(objectCommon_, modelFilePath);
	object.InitializeEffect(effectConfig_);
}