#include "BulletPool.h"
#include "3d/Object3dCommon.h"
#include "Base/TakeCFrameWork.h"
#include <cassert>

void BulletPool::OnInitializeObject(Bullet& object) {

	// 共通の初期化処理
	object.Initialize(objectCommon_, "Bullet.gltf");
}
