#pragma once

#include "engine/GameObject/GameObjectManager.h"
#include "engine/GameObject/TransformComponent.h"
#include "engine/GameObject/AIMovementComponent.h"
#include "engine/GameObject/BehaviorTreeComponent.h"
#include "engine/Math/Quaternion.h"
#include <nlohmann/json.hpp>
#include <cmath>
#include <fstream>
#include <filesystem>
#include <functional>
#include <initializer_list>
#include <limits>
#include <stdexcept>
#include <string_view>
#include <unordered_map>
#include <unordered_set>

namespace TakeC {

// 単一GameObject用の生成レシピ。実行時状態は保存・共有しない。
// 同一スレッドで使用する。登録関数は生成中にRegistryを変更しないこと。
class PrefabFactory final {
public:
    using Json = nlohmann::json;
    using ComponentBuilder = std::function<void(GameObject&, const Json&)>;
    // 呼び出す度に新しいツリーを返す。パス解決/独自ノードRegistryはゲーム側で担当。
    using TreeLoader = std::function<std::unique_ptr<BehaviorNode>(const std::string&)>;
    using ActionSet = std::function<void(BehaviorTreeComponent&)>;
    using BrainBuilder = std::function<std::unique_ptr<IAgentBrain>(const Json&)>;

    explicit PrefabFactory(TreeLoader treeLoader = {}) : treeLoader_(std::move(treeLoader)) {}
    void RegisterComponent(std::string type, ComponentBuilder builder) {
        if (type.empty() || IsBuiltin(type) || !builder ||
            !components_.emplace(std::move(type), std::move(builder)).second) {
            throw std::invalid_argument("Prefab: invalid/duplicate component registration");
        }
    }
    void RegisterActionSet(std::string name, ActionSet binder) {
        if (name.empty() || !binder || !actions_.emplace(std::move(name), std::move(binder)).second) {
            throw std::invalid_argument("Prefab: invalid/duplicate action set");
        }
    }
    void RegisterBrain(std::string name, BrainBuilder builder) {
        if (name.empty() || !builder || !brains_.emplace(std::move(name), std::move(builder)).second) {
            throw std::invalid_argument("Prefab: invalid/duplicate brain");
        }
    }

    std::unique_ptr<GameObject> Build(const Json& recipe) const {
        Fields(recipe, {"version", "name", "active", "transform", "components"});
        if (!recipe.at("version").is_number_integer() || recipe.at("version") != 1) {
            throw std::invalid_argument("Prefab.version: expected 1");
        }
        const auto& list = recipe.at("components");
        if (!list.is_array()) { throw std::invalid_argument("Prefab.components: expected array"); }
        // 不明な型や重複は、副作用を持つ生成処理に入る前に検出する。
        std::unordered_set<std::string> types;
        bool hasAI = false;
        for (const auto& entry : list) {
            if (!entry.is_object()) { throw std::invalid_argument("Prefab component: expected object"); }
            const auto type = entry.at("type").get<std::string>();
            if ((!IsBuiltin(type) && !components_.contains(type)) || !types.insert(type).second) {
                throw std::invalid_argument("Prefab component: unknown/duplicate type " + type);
            }
            if (type == "BehaviorTree" || type == "AIController") {
                if (hasAI) { throw std::invalid_argument("Prefab: only one AI controller is allowed"); }
                hasAI = true;
            }
        }
        auto object = std::make_unique<GameObject>(recipe.value("name", std::string("GameObject")));
        object->SetActive(false);
        if (recipe.contains("transform")) {
            const auto& t = recipe.at("transform");
            Fields(t, {"position", "rotation", "scale"});
            if (t.contains("position")) { object->GetTransform().SetLocalPosition(Vector(t.at("position"))); }
            if (t.contains("rotation")) {
                object->GetTransform().SetLocalRotation(QuaternionMath::FromEuler(Vector(t.at("rotation"))));
            }
            if (t.contains("scale")) { object->GetTransform().SetLocalScale(Vector(t.at("scale"))); }
        }
        for (const auto& entry : list) {
            const auto type = entry.at("type").get<std::string>();
            try {
                BuildComponent(*object, type, entry);
            } catch (const std::exception& error) {
                throw std::runtime_error("Prefab.components[" + type + "]: " + error.what());
            }
        }
        if (object->GetComponents<AIControllerComponent>().size() > 1 || object->IsDestroyRequested()) {
            throw std::invalid_argument("Prefab: invalid object produced by a component builder");
        }
        object->SetActive(recipe.value("active", true));
        return object;
    }

    GameObject& Instantiate(GameObjectManager& world, const Json& recipe) const {
        return world.AdoptObject(Build(recipe));
    }
    GameObject& Load(GameObjectManager& world, const std::filesystem::path& path) const {
        std::ifstream input(path);
        if (!input) { throw std::runtime_error("Cannot open prefab: " + path.string()); }
        return Instantiate(world, Json::parse(input));
    }

    static void Fields(const Json& value, std::initializer_list<std::string_view> allowed) {
        if (!value.is_object()) { throw std::invalid_argument("expected object"); }
        for (const auto& [key, unused] : value.items()) {
            (void)unused;
            bool found = false;
            for (const auto field : allowed) { if (field == key) { found = true; break; } }
            if (!found) { throw std::invalid_argument("unknown field: " + key); }
        }
    }
    static float Number(const Json& value) {
        if (!value.is_number()) { throw std::invalid_argument("expected finite number"); }
        const double number = value.get<double>();
        if (!std::isfinite(number) || std::abs(number) > std::numeric_limits<float>::max()) {
            throw std::invalid_argument("number is outside finite float range");
        }
        return static_cast<float>(number);
    }
    static Vector3 Vector(const Json& value) {
        if (!value.is_array() || value.size() != 3) { throw std::invalid_argument("expected [x,y,z]"); }
        return {Number(value[0]), Number(value[1]), Number(value[2])};
    }

private:
    static bool IsBuiltin(const std::string& type) {
        return type == "AIMovement" || type == "BehaviorTree" || type == "AIController";
    }
    static void ConfigureAI(AIControllerComponent& ai, const Json& entry) {
        ai.SetTickInterval(Number(entry.value("tickInterval", Json(0.0))));
        if (entry.contains("blackboard")) {
            const auto& values = entry.at("blackboard");
            if (!values.is_object()) { throw std::invalid_argument("blackboard: expected object"); }
            for (const auto& [key, value] : values.items()) {
                if (key.empty()) { throw std::invalid_argument("blackboard key is empty"); }
                auto& bb = ai.GetBlackboard();
                if (value.is_boolean()) { bb.Set(key, value.get<bool>()); }
                else if (value.is_string()) { bb.Set(key, value.get<std::string>()); }
                else if (value.is_number_integer()) {
                    const double n = value.get<double>();
                    if (n < INT32_MIN || n > INT32_MAX) { throw std::invalid_argument("blackboard int32 overflow"); }
                    bb.Set(key, value.get<std::int32_t>());
                } else if (value.is_number_float()) { bb.Set(key, Number(value)); }
                else { throw std::invalid_argument("unsupported blackboard value: " + key); }
            }
        }
    }
    void BuildComponent(GameObject& object, const std::string& type, const Json& entry) const {
        if (type == "AIMovement") {
            Fields(entry, {"type", "speed"});
            object.AddComponent<AIMovementComponent>().SetSpeed(Number(entry.at("speed")));
        } else if (type == "BehaviorTree") {
            Fields(entry, {"type", "tree", "actionSet", "repeat", "tickInterval", "blackboard"});
            if (!treeLoader_) { throw std::invalid_argument("tree loader not registered"); }
            auto tree = treeLoader_(entry.at("tree").get<std::string>());
            if (!tree) { throw std::invalid_argument("tree loader returned null"); }
            auto& ai = object.AddComponent<BehaviorTreeComponent>();
            ai.SetTree(std::move(tree));
            ai.SetRepeat(entry.value("repeat", true));
            if (entry.contains("actionSet")) {
                const auto name = entry.at("actionSet").get<std::string>();
                const auto binder = actions_.find(name);
                if (binder == actions_.end()) { throw std::invalid_argument("unknown actionSet: " + name); }
                binder->second(ai);
            }
            ConfigureAI(ai, entry);
        } else if (type == "AIController") {
            Fields(entry, {"type", "brain", "settings", "tickInterval", "blackboard"});
            const auto name = entry.at("brain").get<std::string>();
            const auto builder = brains_.find(name);
            if (builder == brains_.end()) { throw std::invalid_argument("unknown brain: " + name); }
            auto brain = builder->second(entry.value("settings", Json::object()));
            if (!brain) { throw std::invalid_argument("brain factory returned null"); }
            auto& ai = object.AddComponent<AIControllerComponent>();
            ai.SetBrain(std::move(brain));
            ConfigureAI(ai, entry);
        } else { components_.at(type)(object, entry); }
    }
    TreeLoader treeLoader_;
    std::unordered_map<std::string, ComponentBuilder> components_;
    std::unordered_map<std::string, ActionSet> actions_;
    std::unordered_map<std::string, BrainBuilder> brains_;
};
} // namespace TakeC
