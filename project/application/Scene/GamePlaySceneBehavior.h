#pragma once

//GamePlayシーンの状態
enum class SceneBehavior {
	GAMESTART, // ゲーム開始状態
	GAMEPLAY, // ゲームプレイ状態
	ENEMYDESTROYED, // 敵撃破状態
	GAMEOVER, // ゲームオーバー状態
	GAMECLEAR, // ゲームクリア状態
	PAUSE,  // ポーズ状態
};
