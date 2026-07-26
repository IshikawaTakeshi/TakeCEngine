#pragma once
#include <json.hpp>
#include "engine/Utility/JsonDirectoryPathData.h"

//============================================================================
//		BurstShotInfo.h
//============================================================================

// 連射情報構造体
/// <summary>
/// BurstShotInfoに関するデータを保持する構造体です。
/// </summary>
struct BurstShotInfo {
	float kInterval = 0.1f; // 連射時の攻撃間隔
	int kMaxBurstCount = 3; // 連射の最大カウント
	
	void EditConfigImGui();
};

// 実行中の連射状態構造体
/// <summary>
/// BurstShotStateに関するデータを保持する構造体です。
/// </summary>
struct BurstShotState {
	int count = 0;    // 現在の連射カウント
	float intervalTimer = 0.0f; // 連射間隔タイマー
	bool isActive = false; // 連射中かどうか
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(BurstShotInfo, kInterval, kMaxBurstCount)