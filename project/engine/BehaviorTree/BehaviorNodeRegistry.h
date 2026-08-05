#pragma once

#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

class BehaviorNode;
struct BehaviorNodeData;

namespace TakeC {

/// <summary>
/// BehaviorNodeの生成・保存処理とEditor表示用メタデータをまとめた登録情報です。
/// </summary>
struct BehaviorNodeRegistration final {
	using Creator = std::function<std::unique_ptr<BehaviorNode>(const BehaviorNodeData&)>;
	using Serializer = std::function<void(const BehaviorNode&, BehaviorNodeData&)>;

	std::string typeId;
	std::string displayName;
	std::string category;
	Creator creator;
	Serializer serializer;
};

/// <summary>
/// 型IDからBehaviorNodeを生成し、ゲーム側から独自ノードを登録できるレジストリです。
/// </summary>
class BehaviorNodeRegistry final {
public:
	/// <summary>
	/// ノード型を登録します。replaceExistingがfalseで同名登録がある場合は失敗します。
	/// </summary>
	bool Register(BehaviorNodeRegistration registration, bool replaceExisting = false);

	/// <summary>
	/// 登録された型IDからノードを生成します。
	/// </summary>
	[[nodiscard]] std::unique_ptr<BehaviorNode> Create(const BehaviorNodeData& data) const;

	/// <summary>
	/// ノード固有パラメーターを保存用データへ書き出します。
	/// </summary>
	bool SerializeParameters(const BehaviorNode& node, BehaviorNodeData& data) const;

	[[nodiscard]] bool Contains(std::string_view typeId) const;
	bool Unregister(std::string_view typeId);
	void Clear();

	/// <summary>
	/// 登録情報を表示名順で取得します。
	/// </summary>
	[[nodiscard]] std::vector<const BehaviorNodeRegistration*> Registrations() const;

private:
	std::unordered_map<std::string, BehaviorNodeRegistration> registrations_;
};

/// <summary>
/// TakeCEngineが標準提供するBehaviorNodeをレジストリへ登録します。
/// </summary>
void RegisterBuiltInBehaviorNodes(BehaviorNodeRegistry& registry);

} // namespace TakeC
