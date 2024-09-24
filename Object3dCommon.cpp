#include "Object3dCommon.h"

Object3dCommon* Object3dCommon::instance_ = nullptr;

Object3dCommon* Object3dCommon::GetInstance() {
	if (instance_ == nullptr) {
		instance_ = new Object3dCommon();
	}
	return instance_;
	
}

void Object3dCommon::Initialize() {
}

void Object3dCommon::Finalize() {
	delete instance_;
	instance_ = nullptr;
}

void Object3dCommon::PreDraw() {
}
