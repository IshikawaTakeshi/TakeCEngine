#pragma once
#include "application/Weapon/Bullet/VerticalMissile.h"
#include <cstdint>
#include <vector>
#include <memory>

class MissilePool {
public:
	MissilePool() = default;
	~MissilePool() = default;
	void Initialize(size_t size);
	void Finalize();
	VerticalMissile* GetMissile();
	void UpdateAllMissiles();
	void DrawAllMissiles();
	void DrawAllCollider();

	std::vector<VerticalMissile*> GetPool();


private:

	std::vector<std::unique_ptr<VerticalMissile>> pool_;
};

