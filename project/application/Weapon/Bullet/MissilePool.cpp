#include "MissilePool.h"


void MissilePool::Initialize(size_t size) {
	for (size_t i = 0; i < size; i++) {
		pool_.emplace_back(std::make_unique<VerticalMissile>());
	}
}

void MissilePool::Finalize() {
	pool_.clear();
}

VerticalMissile* MissilePool::GetMissile() {
	for (const auto& missile : pool_) {
		if (!missile->GetIsActive()) {
			missile->SetIsActive(true);
			return missile.get();
		}
	}
	return nullptr;
}

void MissilePool::UpdateAllMissiles() {
	for (const auto& missile : pool_) {
		if (missile->GetIsActive()) {
			missile->Update();
		}
	}
}

void MissilePool::DrawAllMissiles() {
	for (const auto& missile : pool_) {
		if (missile->GetIsActive()) {
			missile->Draw();
		}
	}
}

void MissilePool::DrawAllCollider() {
	for (const auto& missile : pool_) {
		if (missile->GetIsActive()) {
			missile->DrawCollider();
		}
	}
}

std::vector<VerticalMissile*> MissilePool::GetPool() {
	std::vector<VerticalMissile*> missiles;
	for (const auto& missile : pool_) {
		missiles.push_back(missile.get());
	}
	return missiles;
}