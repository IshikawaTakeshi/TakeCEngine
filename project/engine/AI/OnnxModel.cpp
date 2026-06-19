#include "OnnxModel.h"

//====================================================================
// 初期化処理
//====================================================================
bool TakeC::OnnxModel::Initialize(Ort::Env& env, const std::wstring& modelPath, int dmlDeviceId) {
    
	// セッションオプションの設定
	sessionOptions_.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);
	// DirectML EPではメモリパターンを無効化
	sessionOptions_.DisableMemPattern();
	// DirectML EPでは同一Sessionの並列Runを避けるため、実行モードをシーケンシャルに設定
	sessionOptions_.SetExecutionMode(ExecutionMode::ORT_SEQUENTIAL);

	// DirectML EPをセッションオプションに追加
	OrtSessionOptionsAppendExecutionProvider_DML(sessionOptions_, dmlDeviceId);
	// モデルのロード
	session_ = std::make_unique<Ort::Session>(env, modelPath.c_str(), sessionOptions_);
	return true;
}
