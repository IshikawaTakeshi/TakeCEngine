#pragma once
#include "Object3d.h"
#include "Object3dCommon.h"
#include <cstdint>
#include <string>
#include <memory>

//武器インターフェース
class IWeapon {
public:

	//武器の初期化
	virtual void Initialize(Object3dCommon* object3dCommon, const std::string& filePath) = 0;
	//武器の更新
	virtual void Update() = 0;
	//武器の描画
	virtual void Draw() = 0;
	//武器の破棄
	virtual void Finalize() = 0;
	//武器の攻撃
	virtual void Attack() = 0;
	//武器タイプの取得
	virtual int32_t GetWeaponType() const = 0;
	//所有者の設定
	virtual void SetOwner(int32_t owner) = 0;

protected:

	//武器の3Dオブジェクト
	std::unique_ptr<Object3d> object3d_ = nullptr; 

	//武器の所有者
	int32_t owner_ = -1;
	//武器の種類
	int32_t weaponType_ = -1;
	//武器の攻撃力
	int32_t attackPower_ = 0;
	//攻撃間隔
	float attackInterval_ = 0.0f;
	//弾数
	int32_t bulletCount_ = 0;
	//弾薬の最大数
	int32_t maxBulletCount_;
};