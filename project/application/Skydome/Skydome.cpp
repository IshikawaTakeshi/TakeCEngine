#define NOMINMAX
#include "Input.h"
#include "MatrixMath.h"
#include "Object3dCommon.h"
#include "Vector3Math.h"
#include "Skydome.h"

#include "Sprite.h"
#include "TextureManager.h"
#include "WinApp.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <memory>
#include <numbers>
#ifdef _DEBUG
#include "ImGuiManager.h"
#endif // DEBUG

Skydome::~Skydome() {}

void Skydome::Initialize(Object3dCommon* object3dCommon, const std::string& filePath) {

	//
	Object3d::Initialize(object3dCommon, filePath);
}

void Skydome::Update() {

	//
	Object3d::Update();
}

void Skydome::Draw() {

	// モデル描画
	Object3d::Draw();
}