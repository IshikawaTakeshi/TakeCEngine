#pragma once

#include "engine/BehaviorTree/Blackboard.h"

#include <array>
#include <string>
#include <unordered_map>

namespace TakeC {

/// <summary>
/// Blackboardの内容をImGuiで表示・編集する開発用パネルです。
/// </summary>
class BlackboardPanel final {
public:
	/// <summary>
	/// 指定したBlackboardの編集UIを描画します。
	/// </summary>
	void Draw(Blackboard& blackboard);

private:
	using TextBuffer = std::array<char, 128>;

	TextBuffer newKeyBuffer_{};
	int newValueType_ = 0;
	std::unordered_map<std::string, TextBuffer> renameBuffers_;
};

} // namespace TakeC
