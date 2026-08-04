#include "Blackboard.h"

void Blackboard::UpdateImGui() {

#if defined(_DEBUG) || defined(_DEVELOP)

	ImGui::SeparatorText("Blackboard Editor");

	//=================================================================
	// 新規キーの追加
	//=================================================================
	{
		static char newKeyBuf[128] = "";
		static int selectedType = 0;

		ImGui::PushItemWidth(150.0f);
		ImGui::InputText("##NewKey", newKeyBuf, sizeof(newKeyBuf));
		ImGui::PopItemWidth();

		ImGui::SameLine();
		ImGui::PushItemWidth(80.0f);
		const char* typeNames[] = { "Float", "Bool", "Int" };
		ImGui::Combo("##NewType", &selectedType, typeNames, IM_ARRAYSIZE(typeNames));
		ImGui::PopItemWidth();

		ImGui::SameLine();
		if (ImGui::Button("Add")) {
			std::string newKey = newKeyBuf;
			if (!newKey.empty() && !HasKey(newKey)) {
				data_[newKey] = CreateDefaultByIndex(selectedType);
				newKeyBuf[0] = '\0';
			}
		}
	}

	ImGui::Separator();

	//=================================================================
	// 既存キーの一覧
	//=================================================================
	std::string keyToRemove;
	std::string renameFrom;
	std::string renameTo;
	std::string typeChangeKey;
	int typeChangeTo = 0;

	// ソートして表示順を安定させる
	std::vector<std::string> sortedKeys;
	sortedKeys.reserve(data_.size());
	for (const auto& [key, val] : data_) {
		sortedKeys.push_back(key);
	}
	std::sort(sortedKeys.begin(), sortedKeys.end());

	for (const auto& key : sortedKeys) {
		auto& value = data_[key];

		ImGui::PushID(key.c_str());

		// --- 削除ボタン ---
		if (ImGui::Button("X")) {
			keyToRemove = key;
		}
		ImGui::SameLine();

		// --- 型変更コンボ（variant の index() から現在の型を取得）---
		int currentType = static_cast<int>(value.index());
		ImGui::PushItemWidth(80.0f);
		const char* typeNames[] = { "Float", "Bool", "Int" };
		if (ImGui::Combo("##Type", &currentType, typeNames, IM_ARRAYSIZE(typeNames))) {
			if (currentType != static_cast<int>(value.index())) {
				typeChangeKey = key;
				typeChangeTo = currentType;
			}
		}
		ImGui::PopItemWidth();
		ImGui::SameLine();

		// --- キー名（リネーム対応）---
		static std::unordered_map<std::string, std::string> renameBufs;
		if (renameBufs.find(key) == renameBufs.end()) {
			renameBufs[key] = key;
		}
		char nameBuf[128];
		strncpy_s(nameBuf, renameBufs[key].c_str(), sizeof(nameBuf) - 1);
		nameBuf[sizeof(nameBuf) - 1] = '\0';

		ImGui::PushItemWidth(120.0f);
		if (ImGui::InputText("##Key", nameBuf, sizeof(nameBuf),
			ImGuiInputTextFlags_EnterReturnsTrue)) {
			std::string newName = nameBuf;
			if (!newName.empty() && newName != key && !HasKey(newName)) {
				renameFrom = key;
				renameTo = newName;
				renameBufs[key] = newName;
			}
		}
		ImGui::PopItemWidth();
		ImGui::SameLine();

		// --- 値の編集（variant の型に応じて自動切替）---
		ImGui::PushItemWidth(150.0f);
		std::visit([](auto& val) {
			using T = std::decay_t<decltype(val)>;
			if constexpr (std::is_same_v<T, float>) {
				ImGui::DragFloat("##Value", &val, 0.1f);
			} else if constexpr (std::is_same_v<T, bool>) {
				ImGui::Checkbox("##Value", &val);
			} else if constexpr (std::is_same_v<T, int>) {
				ImGui::DragInt("##Value", &val);
			}
			}, value);
		ImGui::PopItemWidth();

		ImGui::PopID();
	}

	//=================================================================
	// 遅延処理
	//=================================================================
	if (!keyToRemove.empty()) {
		data_.erase(keyToRemove);
	}
	if (!renameFrom.empty()) {
		data_[renameTo] = std::move(data_[renameFrom]);
		data_.erase(renameFrom);
	}
	if (!typeChangeKey.empty()) {
		data_[typeChangeKey] = CreateDefaultByIndex(typeChangeTo);
	}

#endif // _DEBUG || _DEVELOP
}

bool Blackboard::HasKey(const std::string& key) const {
	return data_.contains(key);
}

void Blackboard::RemoveKey(const std::string& key) {
	data_.erase(key);
}

void Blackboard::Clear() {
	data_.clear();
}

float Blackboard::GetAsFloat(const std::string& key, float defaultValue) const {
	auto it = data_.find(key);
	if (it == data_.end()) return defaultValue;

	return std::visit([](const auto& v) -> float {
		using T = std::decay_t<decltype(v)>;
		if constexpr (std::is_same_v<T, float>) return v;
		else if constexpr (std::is_same_v<T, bool>) return v ? 1.0f : 0.0f;
		else if constexpr (std::is_same_v<T, int>) return static_cast<float>(v);
		else return 0.0f;
		}, it->second);
}

BlackboardValue Blackboard::CreateDefaultByIndex(int index) const {
	switch (index) {
	case 0: return int(0);
	case 1: return float(0.0f);
	case 2: return bool(false);
	case 3: return std::string("");
	default: assert(false && "Invalid type index"); return int(0);
	}
}
