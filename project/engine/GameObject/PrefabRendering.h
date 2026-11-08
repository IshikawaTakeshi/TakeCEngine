#pragma once
#include "engine/GameObject/PrefabFactory.h"
#include "engine/3d/Object3d.h"
#include "engine/GameObject/ModelRendererComponent.h"
#include "engine/GameObject/CollisionComponent.h"
#include "engine/Collision/BoxCollider.h"
#include "engine/Collision/SphereCollider.h"
#include "engine/base/ModelManager.h"

namespace TakeC {
// 描画サービスの寿命はFactoryより長く保つ。ヘッドレス利用時は登録不要。
inline void RegisterPrefabRendering(PrefabFactory& factory, Object3dCommon& common) {
    factory.RegisterComponent("ModelRenderer", [&common](GameObject& object, const PrefabFactory::Json& j) {
        PrefabFactory::Fields(j, {"type", "model"});
        const auto path = j.at("model").get<std::string>();
        if (path.empty()) { throw std::invalid_argument("model path is empty"); }
        ModelManager::GetInstance().LoadModel(path);
        if (!ModelManager::GetInstance().FindModel(path)) {
            throw std::runtime_error("model could not be loaded: " + path);
        }
        object.AddComponent<ModelRendererComponent>().Initialize(&common, path);
    });
    factory.RegisterComponent("Collision", [](GameObject& object, const PrefabFactory::Json& j) {
        const auto shape = j.at("shape").get<std::string>();
        std::unique_ptr<Collider> collider;
        if (shape == "Box") {
            PrefabFactory::Fields(j, {"type", "shape", "halfExtents", "center", "layer", "mask", "trigger"});
            const auto size = PrefabFactory::Vector(j.at("halfExtents"));
            if (size.x <= 0 || size.y <= 0 || size.z <= 0) { throw std::invalid_argument("halfExtents must be positive"); }
            auto box = std::make_unique<BoxCollider>();
            box->SetHalfSize(size);
            collider = std::move(box);
        } else if (shape == "Sphere") {
            PrefabFactory::Fields(j, {"type", "shape", "radius", "center", "layer", "mask", "trigger"});
            const auto radius = PrefabFactory::Number(j.at("radius"));
            if (radius <= 0) { throw std::invalid_argument("radius must be positive"); }
            auto sphere = std::make_unique<SphereCollider>();
            sphere->SetRadius(radius);
            collider = std::move(sphere);
        } else { throw std::invalid_argument("unknown collision shape"); }
        const auto bits = [](const PrefabFactory::Json& value) {
            if (!value.is_number_integer() || value.get<double>() < 0 ||
                value.get<double>() > UINT32_MAX) { throw std::invalid_argument("expected uint32 mask/layer"); }
            return value.get<std::uint32_t>();
        };
        auto& collision = object.AddComponent<CollisionComponent>();
        collision.SetCollisionLayer(bits(j.value("layer", PrefabFactory::Json(0))));
        collision.SetCollisionMask(bits(j.value("mask", PrefabFactory::Json(UINT32_MAX))));
        collision.SetTrigger(j.value("trigger", false));
        if (j.contains("center")) { collision.SetLocalOffset(PrefabFactory::Vector(j.at("center"))); }
        collision.Initialize(std::move(collider));
    });
}
} // namespace TakeC
