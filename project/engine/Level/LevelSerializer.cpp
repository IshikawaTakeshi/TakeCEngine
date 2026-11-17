#include "engine/Level/LevelSerializer.h"

#include <fstream>
#include <initializer_list>
#include <stdexcept>
#include <string_view>
#include <system_error>
#include <utility>

#if defined(_WIN32)
#include <Windows.h>
#endif

namespace TakeC::Level {
namespace {

using Json = nlohmann::json;

void RequireFields(const Json& value, std::initializer_list<std::string_view> allowed) {
	for (const auto& [key, unused] : value.items()) {
		(void)unused;
		bool found = false;
		for (const std::string_view field : allowed) {
			if (field == key) {
				found = true;
				break;
			}
		}
		if (!found) {
			throw std::invalid_argument("Unknown TakeCLevel field: " + key);
		}
	}
}

void RejectInvalid(const LevelDocument& document) {
	const std::vector<LevelDiagnostic> errors = ValidateLevelDocument(document);
	if (!errors.empty()) {
		throw std::invalid_argument("Invalid level document" +
			(errors.front().objectId.empty() ? std::string{} : " [" + errors.front().objectId + "]") +
			": " + errors.front().message);
	}
}

bool ReplaceFile(const std::filesystem::path& temporary, const std::filesystem::path& target) {
#if defined(_WIN32)
	return MoveFileExW(
		temporary.c_str(), target.c_str(),
		MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) != FALSE;
#else
	std::error_code error;
	std::filesystem::rename(temporary, target, error);
	return !error;
#endif
}

} // namespace

LevelDocument ParseLevelDocument(const nlohmann::json& source) {
	if (!source.is_object() || source.value("format", std::string{}) != "TakeCLevel") {
		throw std::invalid_argument("Expected TakeCLevel document.");
	}
	RequireFields(source, { "format", "version", "name", "objects" });
	if (!source.contains("version") || !source.at("version").is_number_integer() ||
		source.at("version") != LevelDocument::kCurrentVersion) {
		throw std::invalid_argument("Unsupported TakeCLevel version.");
	}
	if (!source.contains("objects") || !source.at("objects").is_array()) {
		throw std::invalid_argument("TakeCLevel.objects must be an array.");
	}
	LevelDocument document;
	if (source.contains("name")) {
		document.name = source.at("name").get<std::string>();
	}
	for (const Json& item : source.at("objects")) {
		if (!item.is_object()) {
			throw std::invalid_argument("Each TakeCLevel object must be an object.");
		}
		RequireFields(item, { "id", "parentId", "gameObject" });
		GameObjectDefinition definition;
		definition.id = item.at("id").get<std::string>();
		if (item.contains("parentId") && !item.at("parentId").is_null()) {
			definition.parentId = item.at("parentId").get<std::string>();
		}
		definition.gameObject = item.at("gameObject");
		document.objects.push_back(std::move(definition));
	}
	RejectInvalid(document);
	return document;
}

nlohmann::json SerializeLevelDocument(const LevelDocument& document) {
	RejectInvalid(document);
	Json objects = Json::array();
	for (const GameObjectDefinition& definition : document.objects) {
		Json item = {
			{ "id", definition.id },
			{ "gameObject", definition.gameObject },
		};
		if (definition.parentId) {
			item["parentId"] = *definition.parentId;
		}
		objects.push_back(std::move(item));
	}
	return {
		{ "format", "TakeCLevel" },
		{ "version", LevelDocument::kCurrentVersion },
		{ "name", document.name },
		{ "objects", std::move(objects) },
	};
}

LevelDocument LoadLevelDocument(const std::filesystem::path& path) {
	std::ifstream input(path);
	if (!input) {
		throw std::runtime_error("Cannot open level file: " + path.string());
	}
	Json source;
	input >> source;
	return ParseLevelDocument(source);
}

void SaveLevelDocument(const std::filesystem::path& path, const LevelDocument& document) {
	if (path.empty()) {
		throw std::invalid_argument("Level file path must not be empty.");
	}
	const Json source = SerializeLevelDocument(document);
	if (!path.parent_path().empty()) {
		std::filesystem::create_directories(path.parent_path());
	}
	std::filesystem::path temporary = path;
	temporary += ".tmp";
	{
		std::ofstream output(temporary, std::ios::trunc);
		if (!output) {
			throw std::runtime_error("Cannot open temporary level file: " + temporary.string());
		}
		output << source.dump(2) << '\n';
		if (!output) {
			throw std::runtime_error("Cannot write temporary level file: " + temporary.string());
		}
	}
	if (!ReplaceFile(temporary, path)) {
		throw std::runtime_error("Cannot replace level file: " + path.string());
	}
}

} // namespace TakeC::Level
