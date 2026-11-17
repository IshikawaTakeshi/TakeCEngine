#include "engine/Level/LevelDocument.h"

#include <cmath>
#include <functional>
#include <limits>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>

namespace TakeC::Level {
namespace {

using Json = nlohmann::json;

void AddError(std::vector<LevelDiagnostic>& errors, std::string id, std::string message) {
	errors.push_back({ std::move(id), std::move(message) });
}

bool IsVector3(const Json& value) {
	if (!value.is_array() || value.size() != 3) {
		return false;
	}
	for (const Json& coordinate : value) {
		if (!coordinate.is_number() || !std::isfinite(coordinate.get<double>()) ||
			std::abs(coordinate.get<double>()) > std::numeric_limits<float>::max()) {
			return false;
		}
	}
	return true;
}

void ValidateRecipe(const GameObjectDefinition& definition, std::vector<LevelDiagnostic>& errors) {
	const Json& recipe = definition.gameObject;
	if (!recipe.is_object()) {
		AddError(errors, definition.id, "gameObject must be an object.");
		return;
	}
	if (!recipe.contains("version") || !recipe.at("version").is_number_integer() ||
		recipe.at("version") != 1) {
		AddError(errors, definition.id, "gameObject.version must be 1.");
	}
	if (!recipe.contains("components") || !recipe.at("components").is_array()) {
		AddError(errors, definition.id, "gameObject.components must be an array.");
	} else {
		std::unordered_set<std::string> componentTypes;
		for (const Json& component : recipe.at("components")) {
			if (!component.is_object() || !component.contains("type") ||
				!component.at("type").is_string() ||
				component.at("type").get<std::string>().empty()) {
				AddError(errors, definition.id, "Each component needs a non-empty string type.");
			} else if (!componentTypes.insert(component.at("type").get<std::string>()).second) {
				AddError(errors, definition.id, "Duplicate component type.");
			}
		}
	}
	if (recipe.contains("name") && !recipe.at("name").is_string()) {
		AddError(errors, definition.id, "gameObject.name must be a string.");
	}
	if (recipe.contains("active") && !recipe.at("active").is_boolean()) {
		AddError(errors, definition.id, "gameObject.active must be a boolean.");
	}
	if (!recipe.contains("transform")) {
		return;
	}
	const Json& transform = recipe.at("transform");
	if (!transform.is_object()) {
		AddError(errors, definition.id, "gameObject.transform must be an object.");
		return;
	}
	for (const std::string_view field : { "position", "rotation", "scale" }) {
		const std::string key(field);
		if (transform.contains(key) && !IsVector3(transform.at(key))) {
			AddError(errors, definition.id, "transform." + key + " must contain three finite numbers.");
		}
	}
}

} // namespace

std::vector<LevelDiagnostic> ValidateLevelDocument(const LevelDocument& document) {
	std::vector<LevelDiagnostic> errors;
	if (document.version != LevelDocument::kCurrentVersion) {
		AddError(errors, {}, "Unsupported level version.");
	}
	std::unordered_map<std::string, std::size_t> indices;
	for (std::size_t index = 0; index < document.objects.size(); ++index) {
		const GameObjectDefinition& definition = document.objects[index];
		if (definition.id.empty()) {
			AddError(errors, {}, "GameObject ID must not be empty.");
		} else if (!indices.emplace(definition.id, index).second) {
			AddError(errors, definition.id, "Duplicate GameObject ID.");
		}
		ValidateRecipe(definition, errors);
	}
	for (const GameObjectDefinition& definition : document.objects) {
		if (definition.parentId && !indices.contains(*definition.parentId)) {
			AddError(errors, definition.id, "Parent GameObject ID was not found.");
		}
	}

	// 0:未訪問、1:探索中、2:探索済み。親参照の閉路を検出します。
	std::vector<unsigned char> state(document.objects.size(), 0);
	std::function<void(std::size_t)> visit = [&](std::size_t index) {
		if (state[index] == 2) {
			return;
		}
		if (state[index] == 1) {
			AddError(errors, document.objects[index].id, "GameObject parent cycle detected.");
			return;
		}
		state[index] = 1;
		if (const auto& parentId = document.objects[index].parentId) {
			if (const auto parent = indices.find(*parentId); parent != indices.end()) {
				visit(parent->second);
			}
		}
		state[index] = 2;
	};
	for (std::size_t index = 0; index < document.objects.size(); ++index) {
		visit(index);
	}
	return errors;
}

} // namespace TakeC::Level
