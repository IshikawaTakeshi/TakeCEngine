#pragma once

#include <nlohmann/json.hpp>

#include <optional>
#include <string>
#include <vector>

namespace TakeC::Level {

/// <summary>永続IDとPrefab互換のレシピでレベル内のGameObjectを定義します。</summary>
struct GameObjectDefinition final {
	std::string id;
	std::optional<std::string> parentId;
	nlohmann::json gameObject = {
		{ "version", 1 },
		{ "components", nlohmann::json::array() },
	};
};

/// <summary>GameObject定義の集合を保持するエンジン共通のレベル文書です。</summary>
struct LevelDocument final {
	static constexpr int kCurrentVersion = 1;
	int version = kCurrentVersion;
	std::string name = "Untitled";
	std::vector<GameObjectDefinition> objects;
};

/// <summary>レベル文書の構造エラーと対象GameObjectを表します。</summary>
struct LevelDiagnostic final {
	std::string objectId;
	std::string message;
};

/// <summary>文書・ID・親子関係・Prefab共通フィールドを副作用なく検証します。</summary>
std::vector<LevelDiagnostic> ValidateLevelDocument(const LevelDocument& document);

} // namespace TakeC::Level
