#include "MissilePool.h"
#include "engine/Base/TakeCFrameWork.h"

//=============================================================================================
// 型ごとの初期化処理
//=============================================================================================

void MissilePool::OnInitializeObject(VerticalMissile& object) {
	object.Initialize(objectCommon_, "ICOBall.gltf");
}
