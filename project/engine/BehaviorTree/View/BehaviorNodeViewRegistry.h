#pragma once

#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

struct BehaviorNodeData;
class BehaviorNodeView;
struct ImVec2;

namespace ImFlow {
class ImNodeFlow;
}

namespace TakeC {

/// <summary>
/// BehaviorTreeEditorへ登録するノードViewの生成処理と表示情報です。
/// </summary>
struct BehaviorNodeViewRegistration final {
	using CreateAtCallback = std::function<std::shared_ptr<BehaviorNodeView>(
		ImFlow::ImNodeFlow&, const ImVec2&, const BehaviorNodeData&)>;
	using PlaceCallback = std::function<void(ImFlow::ImNodeFlow&)>;

	std::string typeId;
	std::string displayName;
	std::string category;
	CreateAtCallback createAt;
	PlaceCallback place;
};

/// <summary>
/// BehaviorTreeEditorのノードViewを型IDから生成するレジストリです。
/// </summary>
class BehaviorNodeViewRegistry final {
public:
	bool Register(BehaviorNodeViewRegistration registration, bool replaceExisting = false);

	[[nodiscard]] std::shared_ptr<BehaviorNodeView> CreateAt(
		std::string_view typeId,
		ImFlow::ImNodeFlow& editor,
		const ImVec2& position,
		const BehaviorNodeData& data) const;

	bool Place(std::string_view typeId, ImFlow::ImNodeFlow& editor) const;
	[[nodiscard]] bool Contains(std::string_view typeId) const;
	bool Unregister(std::string_view typeId);
	void Clear();

	[[nodiscard]] std::vector<const BehaviorNodeViewRegistration*> Registrations() const;

private:
	std::unordered_map<std::string, BehaviorNodeViewRegistration> registrations_;
};

} // namespace TakeC
