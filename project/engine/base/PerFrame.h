#pragma once

//=============================================
// PerFrame structure
//=============================================

//フレーム情報
struct PerFrame {
	//ゲームを開始してからの時間
	float gameTime;
	//フレームの経過時間
	float deltaTime;
};