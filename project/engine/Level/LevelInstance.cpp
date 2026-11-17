#include "engine/Level/LevelInstance.h"

#include "engine/GameObject/PrefabFactory.h"
#include "engine/GameObject/TransformComponent.h"

#include <memory>
#include <stdexcept>
#include <unordered_map>

namespace TakeC::Level {

LevelInstance::~LevelInstance() {
	Clear();
}

void LevelInstance::Load(const LevelDocument& document) {
	const std::vector<LevelDiagnostic> errors = ValidateLevelDocument(document);
	if (!errors.empty()) {
		throw std::invalid_argument("Invalid level object [" + errors.front().objectId +
			"]: " + errors.front().message);
	}

	// 未登録のまま全て構築し、失敗時は既存ワールドを変えません。
	std::vector<std::pair<std::string, std::unique_ptr<GameObject>>> staged;
	staged.reserve(document.objects.size());
	for (const GameObjectDefinition& definition : document.objects) {
		try {
			staged.emplace_back(definition.id, factory_.Build(definition.gameObject));
		} catch (const std::exception& error) {
			throw std::runtime_error("Cannot build level object [" + definition.id + "]: " + error.what());
		}
	}

	std::unordered_map<std::string, GameObject*> byId;
	for (auto& [id, object] : staged) {
		byId.emplace(id, object.get());
	}
	for (const GameObjectDefinition& definition : document.objects) {
		if (definition.parentId) {
			byId.at(definition.id)->GetTransform().SetParent(
				&byId.at(*definition.parentId)->GetTransform());
		}
	}

	std::vector<std::pair<std::string, GameObjectId>> adopted;
	adopted.reserve(staged.size());
	try {
		for (auto& [id, object] : staged) {
			GameObject& registered = world_.AdoptObject(std::move(object));
			adopted.emplace_back(id, registered.GetId());
		}
	} catch (...) {
		for (const auto& entry : adopted) {
			world_.DestroyObject(entry.second);
		}
		world_.FlushPendingChanges();
		throw;
	}

	Clear();
	objects_ = std::move(adopted);
}

void LevelInstance::Clear() {
	for (const auto& entry : objects_) {
		world_.DestroyObject(entry.second);
	}
	objects_.clear();
	world_.FlushPendingChanges();
}

GameObjectId LevelInstance::FindObjectId(std::string_view persistentId) const {
	for (const auto& [id, runtimeId] : objects_) {
		if (id == persistentId) {
			return runtimeId;
		}
	}
	return kInvalidGameObjectId;
}

} // namespace TakeC::Level
