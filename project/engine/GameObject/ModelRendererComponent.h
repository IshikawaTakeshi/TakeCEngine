#pragma once

#include "engine/GameObject/Component.h"

#include <memory>
#include <string>

class Camera;
struct LightCameraInfo;

namespace TakeC {

class Object3d;
class Object3dCommon;

/// <summary>
/// GameObjectのTransformを既存のObject3dへ同期し、3Dモデルの更新と描画を提供するコンポーネントです。
/// </summary>
class ModelRendererComponent final : public Component {
public:
	ModelRendererComponent() = default;
	~ModelRendererComponent() override;

	void Initialize(Object3dCommon* object3dCommon, const std::string& modelPath);
	void BindLegacyObject(Object3d& object3d);
	void SynchronizeTransform();

	bool IsInitialized() const { return object3d_ != nullptr; }
	Object3d* GetObject3d() { return object3d_; }
	const Object3d* GetObject3d() const { return object3d_; }

	void SetCamera(Camera* camera);
	void Draw();
	void DrawShadow(const LightCameraInfo& lightCamera);
	void Dispatch();

protected:
	void Update(const ComponentUpdateContext& context) override;

private:
	std::unique_ptr<Object3d> ownedObject3d_;
	Object3d* object3d_ = nullptr;
};

} // namespace TakeC
