#include "ModelCommon.h"
#include "DirectXCommon.h"
#include "SrvManager.h"


//============================================================================
// インスタンスの取得
//============================================================================
ModelCommon& ModelCommon::GetInstance() {
	static ModelCommon instance_;
	return instance_;
}

//============================================================================
// 初期化
//============================================================================
void ModelCommon::Initialize(TakeC::DirectXCommon* dxCommon,TakeC::SrvManager* srvManager) {
	dxCommon_ = dxCommon;
	srvManager_ = srvManager;
}

//============================================================================
// 終了処理
//============================================================================
void ModelCommon::Finalize() {
	dxCommon_ = nullptr;
}
