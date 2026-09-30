#include "engine/GameObject/ModelRendererComponent.h"

#include "engine/3d/Object3d.h"
#include "engine/GameObject/GameObject.h"
#include "engine/GameObject/TransformComponent.h"
#include "engine/Math/Quaternion.h"

namespace TakeC {

ModelRendererComponent::~ModelRendererComponent() = default;

void ModelRendererComponent::Initialize(
	Object3dCommon* object3dCommon,
	const std::string& modelPath) {
	ownedObject3d_ = std::make_unique<Object3d>();
	ownedObject3d_->Initialize(object3dCommon, modelPath);
	object3d_ = ownedObject3d_.get();
	SynchronizeTransform();
}

void ModelRendererComponent::BindLegacyObject(Object3d& object3d) {
	ownedObject3d_.reset();
	object3d_ = &object3d;
	SynchronizeTransform();
}

void ModelRendererComponent::SynchronizeTransform() {
	if (object3d_ == nullptr) {
		return;
	}

	const TransformComponent& transform = GetOwner().GetTransform();
	object3d_->SetTranslate(transform.GetLocalPosition());
	object3d_->SetRotate(QuaternionMath::ToEuler(transform.GetLocalRotation()));
	object3d_->SetScale(transform.GetLocalScale());
}

void ModelRendererComponent::SetCamera(Camera* camera) {
	if (object3d_ != nullptr) {
		object3d_->SetCamera(camera);
	}
}

void ModelRendererComponent::Draw() {
	if (object3d_ != nullptr) {
		object3d_->Draw();
	}
}

void ModelRendererComponent::DrawShadow(const LightCameraInfo& lightCamera) {
	if (object3d_ != nullptr) {
		object3d_->DrawShadow(lightCamera);
	}
}

void ModelRendererComponent::Dispatch() {
	if (object3d_ != nullptr) {
		object3d_->Dispatch();
	}
}

void ModelRendererComponent::Update([[maybe_unused]] const ComponentUpdateContext& context) {
	if (object3d_ == nullptr) {
		return;
	}

	SynchronizeTransform();
	object3d_->Update();
}

} // namespace TakeC
