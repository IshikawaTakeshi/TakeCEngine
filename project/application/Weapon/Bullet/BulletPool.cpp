#include "BulletPool.h"
#include "3d/Object3dCommon.h"
#include "Base/TakeCFrameWork.h"
#include <cassert>

void BulletPool::OnInitializeObject(Bullet& object,const std::string& modelFilePath,const BulletEffectConfig& effectConfig) {

	effectConfig_ = effectConfig;

	//初期化処理
	object.Initialize(objectCommon_, modelFilePath);
	object.InitializeEffect(effectConfig_);
}