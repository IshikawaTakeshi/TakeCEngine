#include "OnnxModel.h"
#include <algorithm>

#include "engine/Utility/Logger.h"

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
	runtimeInputShapes_.clear();
	runtimeOutputShapes_.clear();
	inputDataList_.clear();
	outputDataList_.clear();
	inputNamesPtrs_.clear();
	outputNamesPtrs_.clear();

	Ort::AllocatorWithDefaultOptions allocator;

	// 入力の名前と形状を取得
	const size_t inputCount = session_->GetInputCount();
	inputNames_.reserve(inputCount);
	inputShapes_.reserve(inputCount);
	for (size_t i = 0; i < inputCount; ++i) {
		// 入力名を取得
		Ort::AllocatedStringPtr name = session_->GetInputNameAllocated(i, allocator);
		inputNames_.emplace_back(name.get());

		Ort::TypeInfo typeInfo = session_->GetInputTypeInfo(i);
		Ort::ConstTensorTypeAndShapeInfo tensorInfo = typeInfo.GetTensorTypeAndShapeInfo();
		inputShapes_.emplace_back(tensorInfo.GetShape());
	}

	// 出力の名前と形状を取得
	const size_t outputCount = session_->GetOutputCount();
	outputNames_.reserve(outputCount);
	outputShapes_.reserve(outputCount);
	for (size_t i = 0; i < outputCount; ++i) {

		// 出力名を取得
		Ort::AllocatedStringPtr name = session_->GetOutputNameAllocated(i, allocator);
		outputNames_.emplace_back(name.get());


		// 出力の形状を取得
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

	// モデル情報として取得したinputShapes_とは別に、実際の推論で使うshapeを保持する。
	// dynamic shape(-1)を含むモデルでは、Run前にResizeInputBufferで具体値に置き換える。
	runtimeInputShapes_ = inputShapes_;
	runtimeOutputShapes_.resize(outputShapes_.size());
	inputDataList_.resize(inputShapes_.size());
	outputDataList_.resize(outputShapes_.size());

	return true;
}

//====================================================================
// 実行処理
//====================================================================
bool TakeC::OnnxModel::Run(
	const std::vector<float>& inputData,
	const std::vector<int64_t>& inputShape,
	std::vector<float>& outputData,
	std::vector<int64_t>& outputShape,
	size_t inputIndex,
	size_t outputIndex) {

	if (!session_) {
		return false;
	}

	if (inputIndex >= inputNamesPtrs_.size() || outputIndex >= outputNamesPtrs_.size()) {
		return false;
	}

	const size_t inputElementCount = CalculateElementCount(inputShape);
	if (inputElementCount == 0 || inputElementCount != inputData.size()) {
		return false;
	}

	Ort::MemoryInfo memoryInfo = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
	Ort::Value inputTensor = Ort::Value::CreateTensor<float>(
		memoryInfo,
		const_cast<float*>(inputData.data()),
		inputData.size(),
		inputShape.data(),
		inputShape.size());

	std::lock_guard<std::mutex> lock(runMutex_);
	std::vector<Ort::Value> outputTensors = session_->Run(
		Ort::RunOptions{ nullptr },
		&inputNamesPtrs_[inputIndex],
		&inputTensor,
		1,
		&outputNamesPtrs_[outputIndex],
		1);

	if (outputTensors.empty() || !outputTensors[0].IsTensor()) {
		return false;
	}

	Ort::TensorTypeAndShapeInfo outputInfo = outputTensors[0].GetTensorTypeAndShapeInfo();
	outputShape = outputInfo.GetShape();

	const size_t outputElementCount = outputInfo.GetElementCount();
	const float* outputRawData = outputTensors[0].GetTensorData<float>();
	outputData.assign(outputRawData, outputRawData + outputElementCount);

	return true;
}

//====================================================================
// 複数入力・複数出力の実行処理
//====================================================================
bool TakeC::OnnxModel::Run(
	const std::vector<std::vector<float>>& inputDataList,
	const std::vector<std::vector<int64_t>>& inputShapeList,
	std::vector<std::vector<float>>& outputDataList,
	std::vector<std::vector<int64_t>>& outputShapeList) {

	if (!session_) {
		return false;
	}

	if (inputDataList.empty() || inputDataList.size() != inputShapeList.size()) {
		return false;
	}

	if (inputDataList.size() > inputNamesPtrs_.size()) {
		return false;
	}

	std::vector<Ort::Value> inputTensors;
	inputTensors.reserve(inputDataList.size());

	Ort::MemoryInfo memoryInfo = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
	for (size_t i = 0; i < inputDataList.size(); ++i) {
		const size_t inputElementCount = CalculateElementCount(inputShapeList[i]);
		if (inputElementCount == 0 || inputElementCount != inputDataList[i].size()) {
			return false;
		}

		inputTensors.emplace_back(Ort::Value::CreateTensor<float>(
			memoryInfo,
			const_cast<float*>(inputDataList[i].data()),
			inputDataList[i].size(),
			inputShapeList[i].data(),
			inputShapeList[i].size()));
	}

	std::lock_guard<std::mutex> lock(runMutex_);
	std::vector<Ort::Value> outputTensors = session_->Run(
		Ort::RunOptions{ nullptr },
		inputNamesPtrs_.data(),
		inputTensors.data(),
		inputTensors.size(),
		outputNamesPtrs_.data(),
		outputNamesPtrs_.size());

	outputDataList.clear();
	outputShapeList.clear();
	outputDataList.reserve(outputTensors.size());
	outputShapeList.reserve(outputTensors.size());

	for (Ort::Value& outputTensor : outputTensors) {
		if (!outputTensor.IsTensor()) {
			return false;
		}

		Ort::TensorTypeAndShapeInfo outputInfo = outputTensor.GetTensorTypeAndShapeInfo();
		std::vector<int64_t> outputShape = outputInfo.GetShape();
		const size_t outputElementCount = outputInfo.GetElementCount();
		const float* outputRawData = outputTensor.GetTensorData<float>();

		outputShapeList.push_back(std::move(outputShape));
		outputDataList.emplace_back(outputRawData, outputRawData + outputElementCount);
	}

	return true;
}

bool TakeC::OnnxModel::Run() {
	if (!session_) {
		return false;
	}

	if (inputDataList_.empty() || inputDataList_.size() != runtimeInputShapes_.size()) {
		Logger::Log("Input data list and input shapes list must have the same size and cannot be empty.");
		return false;
	}

	if (inputDataList_.size() > inputNamesPtrs_.size()) {
		Logger::Log("Input data list size exceeds the number of input names.");
		return false;
	}

	std::vector<Ort::Value> inputTensors;
	inputTensors.reserve(inputDataList_.size());

	Ort::MemoryInfo memoryInfo = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
	for (size_t i = 0; i < inputDataList_.size(); ++i) {
		const size_t inputElementCount = CalculateElementCount(runtimeInputShapes_[i]);
		if (inputElementCount == 0 || inputElementCount != inputDataList_[i].size()) {
			Logger::Log("Input data size does not match the expected shape.");
			return false;
		}

		inputTensors.emplace_back(Ort::Value::CreateTensor<float>(
			memoryInfo,
			const_cast<float*>(inputDataList_[i].data()),
			inputDataList_[i].size(),
			runtimeInputShapes_[i].data(),
			runtimeInputShapes_[i].size()));
	}



	std::lock_guard<std::mutex> lock(runMutex_);
	std::vector<Ort::Value> outputTensors = session_->Run(
		Ort::RunOptions{ nullptr },
		inputNamesPtrs_.data(),
		inputTensors.data(),
		inputTensors.size(),
		outputNamesPtrs_.data(),
		outputNamesPtrs_.size());

	outputDataList_.clear();
	runtimeOutputShapes_.clear();
	outputDataList_.reserve(outputTensors.size());
	runtimeOutputShapes_.reserve(outputTensors.size());
	for (Ort::Value& outputTensor : outputTensors) {
		if (!outputTensor.IsTensor()) {
			Logger::Log("Output is not a tensor.");
			return false;
		}
		Ort::TensorTypeAndShapeInfo outputInfo = outputTensor.GetTensorTypeAndShapeInfo();
		std::vector<int64_t> outputShape = outputInfo.GetShape();
		const size_t outputElementCount = outputInfo.GetElementCount();
		const float* outputRawData = outputTensor.GetTensorData<float>();
		runtimeOutputShapes_.push_back(std::move(outputShape));
		outputDataList_.emplace_back(outputRawData, outputRawData + outputElementCount);
	}
	return true;
}

std::vector<float>* TakeC::OnnxModel::GetInputData(size_t inputIndex) {
	if (inputIndex >= inputDataList_.size()) {
		return nullptr;
	}

	return &inputDataList_[inputIndex];
}

const std::vector<int64_t>* TakeC::OnnxModel::GetRuntimeInputShape(size_t inputIndex) const {
	if (inputIndex >= runtimeInputShapes_.size()) {
		return nullptr;
	}

	return &runtimeInputShapes_[inputIndex];
}

bool TakeC::OnnxModel::ResizeInputBuffer(size_t inputIndex, const std::vector<int64_t>& inputShape) {
	if (inputIndex >= inputNamesPtrs_.size()) {
		return false;
	}

	const size_t inputElementCount = CalculateElementCount(inputShape);
	if (inputElementCount == 0) {
		return false;
	}

	if (runtimeInputShapes_.size() < inputNamesPtrs_.size()) {
		runtimeInputShapes_.resize(inputNamesPtrs_.size());
	}
	if (inputDataList_.size() < inputNamesPtrs_.size()) {
		inputDataList_.resize(inputNamesPtrs_.size());
	}

	// モデル定義のinputShapes_は変更せず、実際の推論に使うshapeだけを更新する。
	runtimeInputShapes_[inputIndex] = inputShape;
	inputDataList_[inputIndex].resize(inputElementCount);
	return true;
}

//====================================================================
// Shapeから要素数を計算
//====================================================================
size_t TakeC::OnnxModel::CalculateElementCount(const std::vector<int64_t>& shape) {
	if (shape.empty()) {
		return 0;
	}

	size_t elementCount = 1;
	for (int64_t dimension : shape) {
		if (dimension <= 0) {
			return 0;
		}

		elementCount *= static_cast<size_t>(dimension);
	}

	return elementCount;
}
