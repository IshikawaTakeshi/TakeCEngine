#pragma once

#include "BehaviorNodeRegistry.h"
#include "BehaviorTreeAssetValidator.h"
#include "BehaviorTreeUtil.h"

#include <memory>
#include <filesystem>
#include <string>

class BehaviorNode;

namespace TakeC {

class JsonLoader;

/// <summary>
/// BehaviorNodeRegistryを使って保存データからBehaviorTreeを構築するクラスです。
/// </summary>
class ComboFactory final {
public:
	/// <summary>
	/// エンジン標準ノードを登録した内部レジストリを使用します。
	/// </summary>
	ComboFactory();

	/// <summary>
	/// ゲーム側で構成した外部レジストリを使用します。レジストリはFactoryより長く生存させてください。
	/// </summary>
	explicit ComboFactory(BehaviorNodeRegistry& registry);

	ComboFactory(const ComboFactory&) = delete;
	ComboFactory& operator=(const ComboFactory&) = delete;

	/// <summary>汎用BehaviorTreeAssetをJSONから読み込み、実行時ツリーを構築します。</summary>
	std::unique_ptr<BehaviorNode> LoadBehaviorTreeAsset(const std::string& assetFilePath);

	/// <summary>ゲーム側が指定したディレクトリから汎用Assetを読み込みます。</summary>
	std::unique_ptr<BehaviorNode> LoadBehaviorTreeAsset(
		const std::filesystem::path& directory,
		const std::string& assetFilePath);

	/// <summary>注入されたJsonLoaderとディレクトリから汎用Assetを読み込みます。</summary>
	std::unique_ptr<BehaviorNode> LoadBehaviorTreeAsset(
		JsonLoader& jsonLoader,
		const std::filesystem::path& directory,
		const std::string& assetFilePath);

	/// <summary>汎用BehaviorTreeAssetから実行時ツリーを構築します。</summary>
	std::unique_ptr<BehaviorNode> BuildBehaviorTree(const BehaviorTreeAsset& asset) const;

	/// <summary>現在のRegistry構成でAssetを実行可能か検証します。</summary>
	[[nodiscard]] BehaviorTreeValidationResult Validate(const BehaviorTreeAsset& asset) const;

	/// <summary>旧ComboSet JSON向けの互換APIです。</summary>
	std::unique_ptr<BehaviorNode> LoadComboSetData(const std::string& comboSetFilePath);

	/// <summary>旧ComboSetData向けの互換APIです。</summary>
	std::unique_ptr<BehaviorNode> BuildBehaviorTree(const ComboSetData& comboSetData) const;
	std::unique_ptr<BehaviorNode> BuildNode(const BehaviorNodeData& nodeData) const;

	BehaviorNodeRegistry& Registry() noexcept { return *registry_; }
	const BehaviorNodeRegistry& Registry() const noexcept { return *registry_; }

private:
	BehaviorNodeRegistry defaultRegistry_;
	BehaviorNodeRegistry* registry_ = nullptr;
};

/// <summary>汎用名で利用するためのComboFactory互換エイリアスです。</summary>
using BehaviorTreeFactory = ComboFactory;

} // namespace TakeC

using TakeC::ComboFactory;
using TakeC::BehaviorTreeFactory;
