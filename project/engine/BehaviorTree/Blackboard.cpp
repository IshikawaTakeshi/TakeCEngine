#include "Blackboard.h"

namespace TakeC {

bool Blackboard::Contains(std::string_view key) const {
	return data_.contains(std::string(key));
}

bool Blackboard::Remove(std::string_view key) {
	if (data_.erase(std::string(key)) == 0) {
		return false;
	}
	++revision_;
	return true;
}

bool Blackboard::Rename(std::string_view oldKey, std::string_view newKey) {
	if (oldKey.empty() || newKey.empty() || oldKey == newKey || Contains(newKey)) {
		return false;
	}

	auto node = data_.extract(std::string(oldKey));
	if (node.empty()) {
		return false;
	}

	node.key() = std::string(newKey);
	data_.insert(std::move(node));
	++revision_;
	return true;
}

void Blackboard::Clear() {
	if (data_.empty()) {
		return;
	}
	data_.clear();
	++revision_;
}

bool Blackboard::SetOrReplace(std::string_view key, BlackboardValue value) {
	if (key.empty()) {
		return false;
	}

	auto [it, inserted] = data_.try_emplace(std::string(key), std::move(value));
	if (inserted) {
		++revision_;
		return true;
	}

	if (it->second != value) {
		it->second = std::move(value);
		++revision_;
	}
	return true;
}

std::optional<float> Blackboard::TryGetNumberAsFloat(std::string_view key) const {
	auto it = data_.find(std::string(key));
	if (it == data_.end()) {
		return std::nullopt;
	}

	return std::visit([](const auto& value) -> std::optional<float> {
		using T = std::decay_t<decltype(value)>;
		if constexpr (std::is_same_v<T, std::int32_t>) {
			return static_cast<float>(value);
		} else if constexpr (std::is_same_v<T, float>) {
			return value;
		} else {
			return std::nullopt;
		}
	}, it->second);
}

} // namespace TakeC
