#pragma once
#include <string>
#include <vector>

//=============================================================================================
/// BulletEffectConfig struct
//=============================================================================================

/// <summary>
/// BulletEffectConfigで使用する設定値を保持する構造体です。
/// </summary>
struct BulletEffectConfig {
	//トレイルのエフェクトファイルパス
	std::string lightEffectFilePath;
	// 爆発のエフェクトファイルパス
	std::string explosionEffectFilePath;
};