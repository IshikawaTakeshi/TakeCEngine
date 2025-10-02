#pragma once
#include <string>
#include "engine/math/Vector3.h"
#include "engine/math/Transform.h"



struct StepBoostInfo {
	Vector3 direction;    // ステップブーストの方向
	float speed;    // ステップブーストの速度
	float duration; // ステップブーストの持続時間
	float boostTimer;     // ステップブーストの持続タイマー
	float useEnergy;      // ステップブーストに必要なエネルギー
	//インターバル用
	float interval = 0.2f; // ステップブーストのインターバル
	float intervalTimer  = 0.0f; // ステップブーストのインターバルタイマー
};

struct JumpInfo {
	float speed = 50.0f;        // ジャンプの速度
	float jumpTimer = 0.0f;               // ジャンプのタイマー
	float maxJumpTime = 0.5f;       // ジャンプの最大時間
	float deceleration = 40.0f; // ジャンプ中の減速率
	float useEnergy = 150.0f;         // ジャンプに必要なエネルギー
	bool isJumping = false; // ジャンプするかどうか
};

struct ChargeAttackStunInfo {
	float stunTimer = 0.0f;    //チャージ攻撃後の硬直時間
	float stunDuration = 0.5f; // チャージ攻撃後の硬直時間
	bool isStunned = false; // チャージ攻撃後に硬直しているかどうか
};

struct EnergyInfo {
	float energy = 0.0f;               // 現在のエネルギー
	float maxEnergy = 1000.0f;         // 最大エネルギー
	float recoveryRate = 200.0f;    // エネルギーの回復速度
	const float energyCooldown = 1.0f; // エネルギー使用後のクールダウン時間
	bool isEnergyDepleted = false; // エネルギーが枯渇しているかどうか
};
struct OverHeatInfo {
	float overheatTimer = 0.0f;          // オーバーヒートのタイマー
	float overheatDuration = 3.0f; // オーバーヒートの持続時間
	bool isOverheated = false;           // オーバーヒート中かどうか
};

struct GameCharacterContext {
	QuaternionTransform transform; // 位置、回転、スケール
	Vector3 velocity;              // 速度
	Vector3 moveDirection;        //移動方向
	float deceleration = 1.1f;    //減速率
	float moveSpeed = 200.0f;     //移動速度
	float kMaxMoveSpeed = 120.0f; //移動速度の最大値
	float fallSpeed = 40.0f;      //落下速度
	float health = 0.0f;          // 現在の体力
	float maxHealth = 10000.0f;   // 最大体力
	bool isAlive = true;          //生存しているかどうか
	bool onGround = true; // 地面にいるかどうか
	bool isChargeShooting = false; // チャージショット中かどうか
	bool isDamaged = false; // ダメージを受けたかどうか

	StepBoostInfo stepBoostInfo;   // ステップブースト情報
	JumpInfo jumpInfo;             // ジャンプ情報
	ChargeAttackStunInfo chargeAttackStunInfo; // チャージ攻撃後の硬直情報
	EnergyInfo energyInfo;         // エネルギー情報
	OverHeatInfo overHeatInfo;     // オーバーヒート情報

	std::string name;              // エンティティの名前
};