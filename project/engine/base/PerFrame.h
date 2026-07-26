#pragma once

//=============================================
// PerFrame structure
//=============================================

//フレーム情報
/// <summary>
/// PerFrameに必要な値をまとめて保持する構造体です。
/// </summary>
struct PerFrame {
	//ゲームを開始してからの時間
	float gameTime;
	//フレームの経過時間
	float deltaTime;
};