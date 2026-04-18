#pragma once
#include "application/Weapon/Bullet/VerticalMissile.h"
#include "application/Weapon/Bullet/ObjectPool.h"
#include "Weapon/Bullet/BulletEffectConfig.h"
#include <cstdint>
#include <vector>
#include <memory>

//============================================================================
// MissilePool class
//============================================================================
class MissilePool : public ObjectPool<VerticalMissile> {
public:
	MissilePool() = default;
	~MissilePool() = default;

	//========================================================================
	// function
	//========================================================================

protected:

	//型ごとの初期化処理
	void OnInitializeObject(VerticalMissile& object,const std::string& modelFilePath,const BulletEffectConfig& effectConfig) override;

};