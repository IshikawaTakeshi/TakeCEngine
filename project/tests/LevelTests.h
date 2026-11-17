#pragma once

#include "engine/GameObject/PrefabFactory.h"
#include "engine/Level/LevelEditorSession.h"
#include "engine/Level/LevelInstance.h"
#include "engine/Level/LevelSerializer.h"

#include <cassert>
#include <chrono>
#include <filesystem>
#include <stdexcept>
#include <string>

inline void RunLevelTests() {
	using namespace TakeC;
	using namespace TakeC::Level;
	using Json = nlohmann::json;

	LevelDocument source;
	source.name = "Test Level";
	GameObjectDefinition parent;
	parent.id = "root";
	parent.gameObject = Json::parse(R"({
		"version":1,"name":"Root","transform":{"position":[2,0,0]},"components":[]
	})");
	GameObjectDefinition child;
	child.id = "child";
	child.parentId = "root";
	child.gameObject = Json::parse(R"({
		"version":1,"name":"Child","transform":{"position":[1,0,0]},
		"components":[{"type":"AIMovement","speed":2}]
	})");
	source.objects = { parent, child };
	assert(ValidateLevelDocument(source).empty());
	const Json serialized = SerializeLevelDocument(source);
	assert(serialized.at("format") == "TakeCLevel");
	assert(serialized.at("objects").size() == 2);
	const LevelDocument parsed = ParseLevelDocument(serialized);
	assert(parsed.objects.at(1).parentId == "root");
	assert(parsed.objects.at(1).gameObject == child.gameObject);

	const auto uniqueName = "TakeCLevelTests-" + std::to_string(
		std::chrono::steady_clock::now().time_since_epoch().count());
	const std::filesystem::path directory = std::filesystem::temp_directory_path() / uniqueName;
	const std::filesystem::path file = directory / "sample.level.json";
	SaveLevelDocument(file, source);
	SaveLevelDocument(file, source); // 既存ファイルの置き換えも確認
	const LevelDocument loaded = LoadLevelDocument(file);
	assert(loaded.name == source.name && loaded.objects.size() == 2);
	std::filesystem::remove(file);
	std::filesystem::remove(directory);

	LevelDocument invalid = source;
	invalid.objects.at(1).id = "root";
	assert(!ValidateLevelDocument(invalid).empty());
	invalid = source;
	invalid.objects.at(1).parentId = "missing";
	assert(!ValidateLevelDocument(invalid).empty());
	invalid = source;
	invalid.objects.at(0).parentId = "child";
	assert(!ValidateLevelDocument(invalid).empty());
	invalid = source;
	invalid.objects.at(0).gameObject["transform"]["position"] = Json::array({ 1, 2 });
	assert(!ValidateLevelDocument(invalid).empty());
	invalid = source;
	invalid.objects.at(1).gameObject["components"].push_back(
		invalid.objects.at(1).gameObject["components"].at(0));
	assert(!ValidateLevelDocument(invalid).empty());
	Json extraField = serialized;
	extraField["unknown"] = 1;
	bool rejected = false;
	try {
		(void)ParseLevelDocument(extraField);
	} catch (const std::invalid_argument&) {
		rejected = true;
	}
	assert(rejected);
	rejected = false;
	try {
		(void)SerializeLevelDocument(invalid);
	} catch (const std::invalid_argument&) {
		rejected = true;
	}
	assert(rejected);

	// 不明Componentのデータを読み書きで消さず、生成時だけPrefabFactoryが拒否する。
	LevelDocument future = source;
	future.objects.at(0).gameObject["components"].push_back(
		Json{ { "type", "FutureComponent" }, { "setting", 7 } });
	assert(ParseLevelDocument(SerializeLevelDocument(future)).objects.at(0).gameObject ==
		future.objects.at(0).gameObject);

	PrefabFactory factory;
	GameObjectManager world;
	const GameObjectId externalId = world.CreateObject("External").GetId();
	LevelInstance instance(world, factory);
	instance.Load(loaded);
	assert(instance.GetObjectCount() == 2 && world.GetObjectCount() == 3);
	const GameObjectId oldRootId = instance.FindObjectId("root");
	const GameObjectId oldChildId = instance.FindObjectId("child");
	assert(oldRootId != kInvalidGameObjectId && oldChildId != kInvalidGameObjectId);
	const GameObject* runtimeChild = world.FindObject(oldChildId);
	assert(runtimeChild != nullptr);
	assert(runtimeChild->GetTransform().GetParent() == &world.FindObject(oldRootId)->GetTransform());
	assert(runtimeChild->GetTransform().GetWorldPosition().x == 3.0f);
	assert(runtimeChild->GetComponent<AIMovementComponent>()->GetSpeed() == 2.0f);

	rejected = false;
	try {
		instance.Load(future);
	} catch (const std::runtime_error&) {
		rejected = true;
	}
	assert(rejected && instance.FindObjectId("root") == oldRootId);
	assert(world.GetObjectCount() == 3);

	LevelDocument replacement = source;
	replacement.objects.pop_back();
	instance.Load(replacement);
	assert(instance.GetObjectCount() == 1 && world.GetObjectCount() == 2);
	assert(world.FindObject(oldRootId) == nullptr && world.FindObject(oldChildId) == nullptr);
	assert(world.FindObject(externalId) != nullptr);
	instance.Clear();
	assert(world.GetObjectCount() == 1 && world.FindObject(externalId) != nullptr);

	GameObjectManager editorWorld;
	LevelEditorSession editor(editorWorld, factory);
	assert(editor.SetLevelDocument(source));
	assert(editorWorld.GetObjectCount() == 2);
	assert(editor.Select("root"));
	assert(editor.ApplyToSelected([](GameObjectDefinition& object) {
		object.gameObject["name"] = "Renamed";
	}));
	assert(editor.GetSelectedObject()->gameObject.at("name") == "Renamed");
	assert(editor.CanUndo() && editor.Undo());
	assert(editor.GetSelectedObject()->gameObject.at("name") == "Root");
	assert(editor.CanRedo() && editor.Redo());
	assert(editor.DuplicateSelected());
	assert(editor.GetLevelDocument().objects.size() == 3);
	assert(editor.DeleteSelected());
	assert(editor.GetLevelDocument().objects.size() == 2);
	assert(editor.Select("root") && editor.DeleteSelected());
	assert(editor.GetLevelDocument().objects.empty()); // 子も一緒に削除
	assert(editor.Undo() && editor.GetLevelDocument().objects.size() == 2);
	assert(editorWorld.GetObjectCount() == 2);
	assert(editor.SetLevelDocument(future));
	assert(editor.GetLevelDocument().objects.size() == 2);
	assert(!editor.GetPreviewError().empty() && editorWorld.GetObjectCount() == 0);
	const std::filesystem::path editorDirectory =
		std::filesystem::temp_directory_path() / (uniqueName + "-editor");
	const std::filesystem::path editorFile = editorDirectory / "edited.level.json";
	assert(editor.Save(editorFile) && !editor.IsDirty());
	editor.NewLevelDocument();
	assert(editor.GetLevelDocument().objects.empty());
	assert(editor.Open(editorFile));
	assert(editor.GetLevelDocument().objects.size() == 2);
	assert(!editor.GetPreviewError().empty());
	std::filesystem::remove(editorFile);
	std::filesystem::remove(editorDirectory);
}
