#include "OnnxModel.h"

//====================================================================
// 初期化処理
//====================================================================
bool TakeC::OnnxModel::Initialize(Ort::Env& env, const std::wstring& modelPath, IDMLDevice* dmlDevice, ID3D12CommandQueue* commandQueue) {
    
	// セッションオプションの設定
	sessionOptions_.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);
	// DirectML EPではメモリパターンを無効化
	sessionOptions_.DisableMemPattern();
	// DirectML EPでは同一Sessionの並列Runを避けるため、実行モードをシーケンシャルに設定
	sessionOptions_.SetExecutionMode(ExecutionMode::ORT_SEQUENTIAL);

	// DirectML EPをセッションオプションに追加
	const OrtStatus* status = OrtSessionOptionsAppendExecutionProviderEx_DML(sessionOptions_, dmlDevice, commandQueue);

	// エラーチェック
	if (status != nullptr) {
		Ort::GetApi().ReleaseStatus(const_cast<OrtStatus*>(status));
		return false;
	}

	// モデルのロード
	session_ = std::make_unique<Ort::Session>(env, modelPath.c_str(), sessionOptions_);
	return true;
}

//void TakeC::OnnxModel::Run(const std::vector<float>& inputData, std::vector<int64_t>& inputShapes) {
//}
