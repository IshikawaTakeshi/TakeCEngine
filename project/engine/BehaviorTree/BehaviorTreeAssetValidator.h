#pragma once

#include "BehaviorNodeRegistry.h"
#include "BehaviorTreeUtil.h"

#include <cstddef>
#include <string>
#include <vector>

namespace TakeC {

/// <summary>BehaviorTreeAsset検証結果の重要度を表します。</summary>
enum class BehaviorTreeIssueSeverity {
	Warning,
	Error,
};

/// <summary>BehaviorTreeAsset内で検出された1件の問題を保持します。</summary>
struct BehaviorTreeValidationIssue final {
	BehaviorTreeIssueSeverity severity = BehaviorTreeIssueSeverity::Error;
	std::string code;
	std::string message;
	std::string nodePath;
	int nodeUID = -1;
};

/// <summary>BehaviorTreeAssetの検証結果と問題一覧を保持します。</summary>
class BehaviorTreeValidationResult final {
public:
	[[nodiscard]] bool IsValid() const noexcept { return errorCount_ == 0; }
	[[nodiscard]] std::size_t ErrorCount() const noexcept { return errorCount_; }
	[[nodiscard]] const std::vector<BehaviorTreeValidationIssue>& Issues() const noexcept { return issues_; }

	void AddError(std::string code, std::string message, std::string nodePath = {}, int nodeUID = -1);
	void AddWarning(std::string code, std::string message, std::string nodePath = {}, int nodeUID = -1);

private:
	std::vector<BehaviorTreeValidationIssue> issues_;
	std::size_t errorCount_ = 0;
};

/// <summary>保存AssetをRuntimeへ反映する前に構造と登録ノード型を検証します。</summary>
class BehaviorTreeAssetValidator final {
public:
	[[nodiscard]] static BehaviorTreeValidationResult Validate(
		const BehaviorTreeAsset& asset,
		const BehaviorNodeRegistry& registry);
};

} // namespace TakeC

