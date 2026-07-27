#include "GameLevel.h"

#include "engine/3d/Object3dCommon.h"
#include "engine/Utility/ResourcePath.h"

#include <cassert>
#include <fstream>
#include <json.hpp>
#include <type_traits>
#include <utility>

using namespace TakeC;

namespace {
Vector3 ReadVector3(const nlohmann::json& values) {
	return {
		values[0].get<float>(),
		values[2].get<float>(),
		values[1].get<float>()
	};
}
}

bool GameLevel::Load(const std::string& levelName) {
	auto levelData = LoadLevelData(levelName);
	if (!levelData) {
		return false;
	}

	std::map<std::string, std::unique_ptr<LevelObject>> objects;
	for (const auto& objectData : levelData->objects) {
		auto object = std::make_unique<LevelObject>();
		object->Initialize(&Object3dCommon::GetInstance(), objectData.file_name);
		object->SetName(objectData.name);

		if (objectData.collider.isValid) {
			std::visit([&object](const auto& colliderData) {
				object->CollisionInitialize(colliderData);
			}, objectData.collider.colliderData);
		}

		object->SetTranslate(objectData.translation);
		object->SetRotate(objectData.rotation);
		object->SetScale(objectData.scale);
		object->GetObject3d()->GetModel()->GetMesh()->GetMaterial()->SetMaterialColor({ 0.2f, 0.2f, 0.2f, 1.0f });
		object->GetObject3d()->GetModel()->GetMesh()->GetMaterial()->SetEnvCoefficient(0.0f);
		object->GetObject3d()->GetModel()->GetMesh()->GetMaterial()->SetShininess(20.0f);

		objects.insert_or_assign(objectData.name, std::move(object));
	}

	levelData_ = std::move(levelData);
	objects_ = std::move(objects);
	return true;
}

void GameLevel::Clear() {
	objects_.clear();
	levelData_.reset();
}

std::unique_ptr<LevelData> GameLevel::LoadLevelData(const std::string& levelName) {
	const std::filesystem::path filePath = ResourcePath::Game("JsonLoader") / (levelName + ".json");
	std::ifstream input(filePath);
	if (!input) {
		return nullptr;
	}

	nlohmann::json source;
	input >> source;
	if (!source.is_object() || source.value("name", "") != "scene" || !source.contains("objects")) {
		return nullptr;
	}

	auto levelData = std::make_unique<LevelData>();
	for (const auto& sourceObject : source["objects"]) {
		if (sourceObject.value("type", "") != "MESH") {
			continue;
		}

		auto& objectData = levelData->objects.emplace_back();
		objectData.name = sourceObject.value("name", "");
		objectData.type = "MESH";
		objectData.file_name = sourceObject.value("file_name", "");

		if (sourceObject.contains("transform")) {
			const auto& transform = sourceObject["transform"];
			objectData.translation = ReadVector3(transform["translation"]);
			objectData.rotation = ReadVector3(transform["rotation"]) * -1.0f;
			objectData.scale = ReadVector3(transform["scaling"]);
		}

		if (!sourceObject.contains("collider")) {
			continue;
		}

		const auto& collider = sourceObject["collider"];
		objectData.collider.type = collider.value("type", "");
		objectData.collider.isValid = true;
		if (collider.contains("center")) {
			objectData.collider.center = ReadVector3(collider["center"]);
		}

		if (objectData.collider.type == "BOX" && collider.contains("size")) {
			objectData.collider.colliderData = LevelData::BoxCollider{
				ReadVector3(collider["size"]) * objectData.scale
			};
		} else if (objectData.collider.type == "SPHERE" && collider.contains("radius")) {
			objectData.collider.colliderData = LevelData::SphereCollider{
				collider["radius"].get<float>() * objectData.scale.x
			};
		} else {
			objectData.collider.isValid = false;
		}
	}

	return levelData;
}
