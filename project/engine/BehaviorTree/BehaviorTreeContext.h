#pragma once

#include "engine/AI/AIContext.h"

namespace TakeC {

/// 既存ノードのソース互換用の別名。実際の実行Contextは共通AIContextです。
using BehaviorTreeContext = AIContext;

} // namespace TakeC
