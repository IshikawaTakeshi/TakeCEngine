#pragma once
#include "engine/GameObject/PrefabRendering.h"
#include "engine/BehaviorTree/ComboFactory.h"

namespace TakeC::Examples {
// rootに置くBTのパスはrootからの相対パス。Factoryはこの関数内でのみ使う。
inline GameObject& SpawnPrefab(
    GameObjectManager& world, Object3dCommon& rendering,
    const std::filesystem::path& root, const std::filesystem::path& prefabPath,
    PrefabFactory::ActionSet enemyActions) {
    BehaviorTreeFactory trees;
    PrefabFactory prefabs([&](const std::string& path) {
        std::ifstream input(root / path);
        if (!input) { throw std::runtime_error("Cannot open behavior tree: " + path); }
        return trees.BuildBehaviorTree(nlohmann::json::parse(input).get<BehaviorTreeAsset>());
    });
    RegisterPrefabRendering(prefabs, rendering);
    prefabs.RegisterActionSet("EnemyActions", std::move(enemyActions));
    return prefabs.Load(world, root / prefabPath);
}
} // namespace TakeC::Examples
