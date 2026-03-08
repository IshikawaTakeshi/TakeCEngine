#pragma once
#include "application/Entity/GameCharacterInfo.h"

///============================================================================
/// BreakGaugeUtil namespace
///============================================================================

namespace BreakGaugeUtil {
	/// <summary>
	/// ブレイクゲージの合計値を計算する
	/// </summary>
	/// <param name="bg"></param>
	float ComputeBreakGaugeTotal(const BreakGaugeInfo& bg);

	/// <summary>
	/// ブレイクゲージのエントリーを更新する
	/// </summary>
	void UpdateBreakGaugeEntries(BreakGaugeInfo& bg);

	/// <summary>
	/// ブレイクゲージのエントリーを追加する
	/// </summary>
	/// <param name="amount"></param>
	/// <param name="decayDelay"></param>
	/// <returns></returns>
	BreakGaugeEntry MakeBreakGaugeEntry(float amount, float decayDelay);
}