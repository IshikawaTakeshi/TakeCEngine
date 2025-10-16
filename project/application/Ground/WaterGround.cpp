#include "WaterGround.h"

void WaterGround::Initialize(Object3dCommon* object3dCommon, const std::string& filePath) {
	Object3d::Initialize(object3dCommon, filePath);

	model_->GetMesh()->GetMaterial()->SetTextureFilePath("white1x1.png");
}

void WaterGround::Update() {
	Object3d::Update();
}

void WaterGround::Draw() {
	Object3d::Draw();
}

void WaterGround::ImGuiDebug() {
	Object3d::UpdateImGui("WaterGround");
}