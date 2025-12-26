#include "MissilePool.h"
#include "engine/Base/TakeCFrameWork.h"

//===================================================================================
//　初期化
//===================================================================================
void MissilePool::Initialize(size_t size) {
	for (size_t i = 0; i < size; i++) {
		pool_.emplace_back(std::make_unique<VerticalMissile>());
		pool_[i]->Initialize(&Object3dCommon::GetInstance(), "ICOBall.gltf");
	}
}

//===================================================================================
//　終了処理
//===================================================================================
void MissilePool::Finalize() {
	pool_.clear();
}

//===================================================================================
//　ミサイルの取得
//===================================================================================
VerticalMissile* MissilePool::GetMissile() {
	for (const auto& missile : pool_) {
		if (!missile->GetIsActive()) {
			missile->SetIsActive(true);
			return missile.get();
		}
	}
	return nullptr;
}

//===================================================================================
//　全ミサイルの更新
//===================================================================================
void MissilePool::UpdateAllMissiles() {
	for (const auto& missile : pool_) {
		missile->Update();
	}
}

//===================================================================================
//　全ミサイルの描画
//===================================================================================
void MissilePool::DrawAllMissiles() {
	for (const auto& missile : pool_) {
		if (missile->GetIsActive()) {
			missile->Draw();
		}
	}
}

//===================================================================================
//　全ミサイルのコライダー描画
//===================================================================================
void MissilePool::DrawAllCollider() {
	for (const auto& missile : pool_) {
		if (missile->GetIsActive()) {
			missile->DrawCollider();
		}
	}
}
//===================================================================================
//　ミサイルプールの取得
//===================================================================================
std::vector<VerticalMissile*> MissilePool::GetPool() {
	std::vector<VerticalMissile*> missiles;
	for (const auto& missile : pool_) {
		missiles.push_back(missile.get());
	}
	return missiles;
}