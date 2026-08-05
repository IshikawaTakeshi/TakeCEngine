#include "BlackboardPanel.h"

#include <imgui.h>

#include <algorithm>
#include <array>
#include <cstdio>
#include <cstdint>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace TakeC {
namespace {

constexpr std::array<const char*, 4> kTypeNames = {
	"Int",
	"Float",
	"Bool",
	"String",
};

BlackboardValue CreateDefaultValue(int typeIndex) {
	switch (typeIndex) {
	case 0: return std::int32_t{0};
	case 1: return 0.0f;
	case 2: return false;
	case 3: return std::string{};
	default: return std::int32_t{0};
	}
}

int GetTypeIndex(const BlackboardValue& value) {
	return static_cast<int>(value.index());
}

} // namespace

void BlackboardPanel::Draw(Blackboard& blackboard) {
#if defined(_DEBUG) || defined(_DEVELOP)
	ImGui::SeparatorText("Blackboard Editor");

	ImGui::PushItemWidth(150.0f);
	ImGui::InputText("##BlackboardNewKey", newKeyBuffer_.data(), newKeyBuffer_.size());
	ImGui::PopItemWidth();
	ImGui::SameLine();

	ImGui::PushItemWidth(90.0f);
	ImGui::Combo(
		"##BlackboardNewType",
		&newValueType_,
		kTypeNames.data(),
		static_cast<int>(kTypeNames.size()));
	ImGui::PopItemWidth();
	ImGui::SameLine();

	if (ImGui::Button("Add")) {
		const std::string newKey = newKeyBuffer_.data();
		if (!newKey.empty() && !blackboard.Contains(newKey)) {
			blackboard.SetOrReplace(newKey, CreateDefaultValue(newValueType_));
			newKeyBuffer_.fill('\0');
		}
	}

	ImGui::Separator();

	std::vector<std::string> sortedKeys;
	sortedKeys.reserve(blackboard.Entries().size());
	for (const auto& [key, value] : blackboard.Entries()) {
		(void)value;
		sortedKeys.push_back(key);
	}
	std::sort(sortedKeys.begin(), sortedKeys.end());

	std::string keyToRemove;
	std::pair<std::string, std::string> renameRequest;

	for (const std::string& key : sortedKeys) {
		auto valueIt = blackboard.Entries().find(key);
		if (valueIt == blackboard.Entries().end()) {
			continue;
		}

		const BlackboardValue currentValue = valueIt->second;
		ImGui::PushID(key.c_str());

		if (ImGui::Button("X")) {
			keyToRemove = key;
		}
		ImGui::SameLine();

		int currentType = GetTypeIndex(currentValue);
		ImGui::PushItemWidth(90.0f);
		if (ImGui::Combo("##Type", &currentType, kTypeNames.data(), static_cast<int>(kTypeNames.size()))) {
			blackboard.SetOrReplace(key, CreateDefaultValue(currentType));
		}
		ImGui::PopItemWidth();
		ImGui::SameLine();

		auto [renameIt, inserted] = renameBuffers_.try_emplace(key);
		if (inserted) {
			std::snprintf(renameIt->second.data(), renameIt->second.size(), "%s", key.c_str());
		}

		ImGui::PushItemWidth(140.0f);
		if (ImGui::InputText(
			"##Key",
			renameIt->second.data(),
			renameIt->second.size(),
			ImGuiInputTextFlags_EnterReturnsTrue)) {
			const std::string newName = renameIt->second.data();
			if (!newName.empty() && newName != key && !blackboard.Contains(newName)) {
				renameRequest = {key, newName};
			}
		}
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushItemWidth(180.0f);
		std::visit([&blackboard, &key](const auto& value) {
			using T = std::decay_t<decltype(value)>;
			if constexpr (std::is_same_v<T, std::int32_t>) {
				int editedValue = value;
				if (ImGui::DragInt("##Value", &editedValue)) {
					blackboard.Set(key, static_cast<std::int32_t>(editedValue));
				}
			} else if constexpr (std::is_same_v<T, float>) {
				float editedValue = value;
				if (ImGui::DragFloat("##Value", &editedValue, 0.1f)) {
					blackboard.Set(key, editedValue);
				}
			} else if constexpr (std::is_same_v<T, bool>) {
				bool editedValue = value;
				if (ImGui::Checkbox("##Value", &editedValue)) {
					blackboard.Set(key, editedValue);
				}
			} else if constexpr (std::is_same_v<T, std::string>) {
				std::array<char, 256> buffer{};
				std::snprintf(buffer.data(), buffer.size(), "%s", value.c_str());
				if (ImGui::InputText("##Value", buffer.data(), buffer.size())) {
					blackboard.Set(key, std::string(buffer.data()));
				}
			}
		}, currentValue);
		ImGui::PopItemWidth();

		ImGui::PopID();
	}

	if (!keyToRemove.empty()) {
		blackboard.Remove(keyToRemove);
		renameBuffers_.erase(keyToRemove);
	}
	if (!renameRequest.first.empty() && blackboard.Rename(renameRequest.first, renameRequest.second)) {
		auto buffer = renameBuffers_.extract(renameRequest.first);
		if (!buffer.empty()) {
			buffer.key() = renameRequest.second;
			renameBuffers_.insert(std::move(buffer));
		}
	}
#else
	(void)blackboard;
#endif
}

} // namespace TakeC
