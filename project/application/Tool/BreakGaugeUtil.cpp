#include "BreakGaugeUtil.h"
#include "engine/base/TakeCFrameWork.h"

///============================================================================
/// ブレイクゲージの合計値の計算関数
///============================================================================
float BreakGaugeUtil::ComputeBreakGaugeTotal(const BreakGaugeInfo& bg) {

    float total = 0.0f;
    for (const auto& e : bg.entries) {
        total += e.amount;
    }
    return total;
}

///============================================================================
/// ブレイクゲージのエントリーを更新する関数
///============================================================================

void BreakGaugeUtil::UpdateBreakGaugeEntries(BreakGaugeInfo& bg) {
    for (auto& e : bg.entries) {
        // delayTimer がまだ終わっていない間は減衰しない
        if (!e.delayTimer.IsFinished()) {
            e.delayTimer.Update();
            continue;
        }

        // 減衰開始（delay終了後）
        e.amount -= bg.decayRate * TakeCFrameWork::GetDeltaTime();
        if (e.amount < 0.0f) e.amount = 0.0f;
    }

    // amountが0になったエントリを削除
    bg.entries.erase(
        std::remove_if(bg.entries.begin(), bg.entries.end(),
            [](const BreakGaugeEntry& e) { return e.amount <= 0.0f; }),
        bg.entries.end()
    );

    // breakGaugeを「合計」として更新したいならここで更新
    bg.breakGauge = ComputeBreakGaugeTotal(bg);
}

///============================================================================
/// ブレイクゲージのエントリーを追加する関数
///============================================================================
BreakGaugeEntry BreakGaugeUtil::MakeBreakGaugeEntry(float amount, float decayDelay) {
	BreakGaugeEntry entry;
	entry.amount = amount;
	entry.decayDelay = decayDelay;
	entry.delayTimer.Initialize(decayDelay, 0.0f);
	return entry;
}