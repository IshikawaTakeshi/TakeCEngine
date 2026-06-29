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

	inputNames_.clear();
	outputNames_.clear();
	inputShapes_.clear();
	outputShapes_.clear();
	inputNamesPtrs_.clear();
	outputNamesPtrs_.clear();

	Ort::AllocatorWithDefaultOptions allocator;

	const size_t inputCount = session_->GetInputCount();
	inputNames_.reserve(inputCount);
	inputShapes_.reserve(inputCount);
	for (size_t i = 0; i < inputCount; ++i) {
		Ort::AllocatedStringPtr name = session_->GetInputNameAllocated(i, allocator);
		inputNames_.emplace_back(name.get());

		Ort::TypeInfo typeInfo = session_->GetInputTypeInfo(i);
		Ort::ConstTensorTypeAndShapeInfo tensorInfo = typeInfo.GetTensorTypeAndShapeInfo();
		inputShapes_.emplace_back(tensorInfo.GetShape());
	}

	const size_t outputCount = session_->GetOutputCount();
	outputNames_.reserve(outputCount);
	outputShapes_.reserve(outputCount);
	for (size_t i = 0; i < outputCount; ++i) {
		Ort::AllocatedStringPtr name = session_->GetOutputNameAllocated(i, allocator);
		outputNames_.emplace_back(name.get());

		Ort::TypeInfo typeInfo = session_->GetOutputTypeInfo(i);
		Ort::ConstTensorTypeAndShapeInfo tensorInfo = typeInfo.GetTensorTypeAndShapeInfo();
		outputShapes_.emplace_back(tensorInfo.GetShape());
	}

	for (const std::string& name : inputNames_) {
		inputNamesPtrs_.push_back(name.c_str());
	}
	for (const std::string& name : outputNames_) {
		outputNamesPtrs_.push_back(name.c_str());
	}

	return true;
}

//void TakeC::OnnxModel::Run(const std::vector<float>& inputData, std::vector<int64_t>& inputShapes) {
//}
