#include "Mesh.h"
#include "DirectXCommon.h"
#include "TextureManager.h"

void Mesh::InitializeMesh(DirectXCommon* dxCommon,const std::string& filePath) {

	material_ = std::make_unique<Material>();
	material_->InitializeTexture(dxCommon, filePath);
}