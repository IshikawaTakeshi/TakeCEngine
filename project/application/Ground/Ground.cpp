#define NOMINMAX
#include "Ground.h"
#include "Input.h"
#include "MatrixMath.h"
#include "Object3dCommon.h"
#include "Vector3Math.h"
#include "Model.h"
#include "Sprite.h"
#include "TextureManager.h"
#include "WinApp.h"
#include "TakeCFrameWork.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <memory>
#include <numbers>
#ifdef _DEBUG
#include "ImGuiManager.h"
#endif // DEBUG

Ground::~Ground() {}

void Ground::Initialize(Object3dCommon* object3dCommon, const std::string& filePath) {
	object3dCommon;
	filePath;
}

void Ground::Update() {

}

void Ground::Draw() {


}

void Ground::ImGuiDebug() {
#ifdef _DEBUG


#endif // DEBUG
}