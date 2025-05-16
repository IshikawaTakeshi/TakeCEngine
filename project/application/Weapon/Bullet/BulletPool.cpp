#include "BulletPool.h"
#include <cassert>

void BulletPool::Initialize(size_t size) {

	pool_.resize(size);
	for (int i = 0; i < size; i++) {
		pool_.emplace_back(std::make_unique<Bullet>());
	}
}

void BulletPool::Finalize() {

	pool_.clear();
}

Bullet* BulletPool::GetBullet() {
	for (const auto& bullet : pool_) {
		if (!bullet->GetIsActive()) {
			return bullet.get();
		}
	}

	return nullptr;
}

void BulletPool::UpdateAllBullet() {

	for (const auto& bullet : pool_) {
		if (bullet->GetIsActive()) {
			bullet->Update();
		}
	}
}

void BulletPool::DrawAllBullet() {

	for (const auto& bullet : pool_) {
		if (bullet->GetIsActive()) {
			bullet->Draw();;
		}
	}
}
