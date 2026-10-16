#pragma once
#include "engine/GameObject/PrefabFactory.h"
#include "engine/GameObject/PrefabRendering.h"
#include "engine/BehaviorTree/WaitNode.h"
#include "engine/BehaviorTree/ComboFactory.h"
#include <cassert>

inline void RunPrefabTests() {
    using namespace TakeC;
    using Json = PrefabFactory::Json;
    PrefabFactory factory([](const std::string&) { return std::make_unique<WaitNode>(); });
    factory.RegisterActionSet("EnemyActions", [](BehaviorTreeComponent&) {});
    const auto recipe = Json::parse(R"({
        "version":1, "name":"PrefabEnemy",
        "transform":{"position":[1,2,3]},
        "components":[
            {"type":"AIMovement","speed":3},
            {"type":"BehaviorTree","tree":"Patrol.json","actionSet":"EnemyActions",
             "blackboard":{"visible":false,"count":2,"time":0.5}}
        ]
    })");
    GameObjectManager world;
    auto& a = factory.Instantiate(world, recipe);
    auto& b = factory.Instantiate(world, recipe);
    assert(a.GetWorld() == &world && world.GetObjectCount() == 2);
    assert(a.GetId() != b.GetId());
    assert(a.GetTransform().GetLocalPosition().x == 1);
    assert(a.GetComponent<AIMovementComponent>()->GetSpeed() == 3);
    auto* ai = a.GetComponent<BehaviorTreeComponent>();
    auto* bi = b.GetComponent<BehaviorTreeComponent>();
    assert(ai->GetTree() != bi->GetTree());
    ai->GetBlackboard().Set("visible", true);
    assert(!bi->GetBlackboard().GetOr("visible", true));
    const auto rejects = [&](Json invalid) {
        bool failed = false;
        try { factory.Instantiate(world, invalid); } catch (const std::exception&) { failed = true; }
        assert(failed && world.GetObjectCount() == 2);
    };
    auto invalid = recipe; invalid["components"][0]["speed"] = -1; rejects(invalid);
    invalid = recipe; invalid["version"] = 2; rejects(invalid);
    invalid = recipe; invalid["components"][1]["actionSet"] = "Missing"; rejects(invalid);
    invalid = recipe; invalid["components"].push_back(recipe["components"][0]); rejects(invalid);
    invalid = recipe; invalid["transform"]["position"] = Json::array({1, 2}); rejects(invalid);
    invalid = recipe; invalid["components"][0]["speeed"] = 2; rejects(invalid);
    invalid = recipe; invalid["components"][1]["blackboard"]["count"] = 4294967296ULL; rejects(invalid);
    invalid = recipe; invalid["components"][0]["type"] = "Missing"; rejects(invalid);
    invalid = recipe; invalid["components"][1]["tickInterval"] = -0.1; rejects(invalid);
    invalid = recipe; invalid["components"][0]["speed"] = std::numeric_limits<double>::infinity(); rejects(invalid);
    struct TestBrain final : IAgentBrain {
        void Tick(AIContext& context) override { context.blackboard.Set("ticked", true); }
    };
    factory.RegisterBrain("Custom", [](const Json&) { return std::make_unique<TestBrain>(); });
    auto custom = Json::parse(R"({"version":1,"components":[{"type":"AIController","brain":"Custom"}]})");
    auto detached = factory.Build(custom);
    assert(detached->GetWorld() == nullptr);
    auto* address = detached.get();
    auto& adopted = world.AdoptObject(std::move(detached));
    assert(&adopted == address && adopted.GetWorld() == &world);
    world.Update(0.1f);
    assert(adopted.GetComponent<AIControllerComponent>()->GetBlackboard().GetOr("ticked", false));
    world.DestroyObject(adopted.GetId());
    world.FlushPendingChanges();
    int alive = 0;
    struct LifetimeProbe final : Component {
        explicit LifetimeProbe(int& count) : count_(count) { ++count_; }
        ~LifetimeProbe() override { --count_; }
        int& count_;
    };
    factory.RegisterComponent("Fails", [&](GameObject& object, const Json&) {
        object.AddComponent<LifetimeProbe>(alive);
        throw std::runtime_error("test");
    });
    invalid = recipe; invalid["components"].push_back({{"type","Fails"}}); rejects(invalid);
    assert(world.GetObjectCount() == 2);
    assert(alive == 0);

    // 添付JSONを実際に読み込み、ActionSetを個体別のBTへ紐付ける。
    const auto root = std::filesystem::path(__FILE__).parent_path().parent_path() / "examples" / "Prefabs";
    BehaviorTreeFactory trees;
    PrefabFactory assets([&](const std::string& path) {
        std::ifstream input(root / path);
        return trees.BuildBehaviorTree(Json::parse(input).get<BehaviorTreeAsset>());
    });
    assets.RegisterActionSet("EnemyActions", [](BehaviorTreeComponent& component) {
        component.BindAction("Patrol", [](const AIContext& context) {
            context.blackboard.Set("patrolled", true);
            return BehaviorStatus::Success;
        });
    });
    auto& spawned = assets.Load(world, root / "PatrolEnemy.json");
    world.Update(0.1f);
    assert(spawned.GetComponent<BehaviorTreeComponent>()->GetBlackboard().GetOr("patrolled", false));
}
