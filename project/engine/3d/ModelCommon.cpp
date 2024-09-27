#include "ModelCommon.h"

ModelCommon* ModelCommon::instance_ = nullptr;

ModelCommon* ModelCommon::GetInstance() {
	if(instance_ == nullptr) {
		instance_ = new ModelCommon();
	}
	return instance_;
}

void ModelCommon::Initialize(DirectXCommon* dxCommon) {
	dxCommon_ = dxCommon;

}

void ModelCommon::Finalize() {
	dxCommon_ = nullptr;
	delete instance_;
	instance_ = nullptr;
}
