#pragma once
enum class GameCharacterBehavior {
	NONE = -1, // 行動なし
	IDLE,             // 待機状態
	RUNNING,          // 移動状態
	JUMP,             // ジャンプ状態
	DASH,             // ダッシュ状態
	CHARGESHOOT_STUN, // チャージショット後の硬直状態
	HEAVYDAMAGE,	  // 大ダメージによる硬直状態
	STEPBOOST,        // ステップブースト
	FLOATING,         // 浮遊状態
	DEAD,             // 死亡状態
};