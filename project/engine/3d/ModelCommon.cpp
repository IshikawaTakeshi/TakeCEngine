#include "ModelCommon.h"
#include "DirectXCommon.h"
#include "SrvManager.h"

// 静的メンバ変数の初期化
ModelCommon* ModelCommon::instance_ = nullptr;

//============================================================================
// インスタンスの取得
//============================================================================
ModelCommon* ModelCommon::GetInstance() {
	if(instance_ == nullptr) {
		instance_ = new ModelCommon();
	}
	return instance_;
}

//============================================================================
// 初期化
//============================================================================
void ModelCommon::Initialize(DirectXCommon* dxCommon,SrvManager* srvManager) {
	dxCommon_ = dxCommon;
	srvManager_ = srvManager;
}

//============================================================================
// 終了処理
//============================================================================
void ModelCommon::Finalize() {
	dxCommon_ = nullptr;
	delete instance_;
	instance_ = nullptr;
}
