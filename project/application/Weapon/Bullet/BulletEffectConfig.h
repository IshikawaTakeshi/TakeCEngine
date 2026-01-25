#pragma once
#include <string>
#include <vector>

//=============================================================================================
/// BulletEffectConfig struct
//=============================================================================================

struct BulletEffectConfig {
	//トレイルのエフェクトファイルパス
	std::vector<std::string> trailEffectFilePath;
	// 爆発のエフェクトファイルパス
	std::vector<std::string> explosionEffectFilePath;
};