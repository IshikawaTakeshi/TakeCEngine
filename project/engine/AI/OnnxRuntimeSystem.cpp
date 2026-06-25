#include "OnnxRuntimeSystem.h"
#include <cassert>

using namespace TakeC;

//====================================================================
//	インスタンスの取得
//====================================================================
OnnxRuntimeSystem& TakeC::OnnxRuntimeSystem::GetInstance() {
	static OnnxRuntimeSystem instance;
	return instance;
}

//====================================================================
//	初期化処理
//====================================================================
void TakeC::OnnxRuntimeSystem::Initialize(DirectXCommon* dxCommon) {

	// DirectMLデバイスの作成
	HRESULT hr = DMLCreateDevice1(dxCommon->GetDevice(), DML_CREATE_DEVICE_FLAG_NONE,DML_FEATURE_LEVEL_5_0, IID_PPV_ARGS(&dmlDevice_));
	assert(SUCCEEDED(hr));
}

void TakeC::OnnxRuntimeSystem::Finalize() {
	dmlDevice_.Reset();
}
