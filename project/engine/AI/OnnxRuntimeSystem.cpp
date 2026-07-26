#include "OnnxRuntimeSystem.h"
#include <cassert>
#include <exception>
#include "engine/Utility/Logger.h"
#include "engine/Utility/ResourcePath.h"

using namespace TakeC;

//====================================================================
//	初期化処理
//====================================================================
void TakeC::OnnxRuntimeSystem::Initialize(DirectXCommon* dxCommon) {
	assert(dxCommon);
	assert(dxCommon->GetDevice());
	assert(dxCommon->GetCommandQueue());

	if (initialized_) {
		return;
	}

	dxCommon_ = dxCommon;
	env_ = std::make_unique<Ort::Env>(ORT_LOGGING_LEVEL_WARNING, "TakeCOnnxRuntime");
	cpuMemoryInfo_ = std::make_unique<Ort::MemoryInfo>(
		Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault));

	// DirectMLデバイスの作成
	HRESULT hr = DMLCreateDevice1(dxCommon->GetDevice(), DML_CREATE_DEVICE_FLAG_NONE,DML_FEATURE_LEVEL_5_0, IID_PPV_ARGS(&dmlDevice_));
	assert(SUCCEEDED(hr));

	// DirectMLデバイスの作成に失敗した場合のエラーハンドリング
	if (FAILED(hr)) {
		Logger::Log("\n OnnxRuntimeSystem DirectML device creation failed.\n");
		env_.reset();
		cpuMemoryInfo_.reset();
		dxCommon_ = nullptr;
		return;
	}

	initialized_ = true;
	Logger::Log("\n OnnxRuntimeSystem initialized successfully!\n");
}

//====================================================================
//	終了・開放処理
//====================================================================
void TakeC::OnnxRuntimeSystem::Finalize() {
	models_.clear();
	cpuMemoryInfo_.reset();
	env_.reset();
	dmlDevice_.Reset();
	dxCommon_ = nullptr;
	initialized_ = false;
	Logger::Log("\n OnnxRuntimeSystem finalized.\n");
}

//====================================================================
//	onnxモデルのロード
//====================================================================
OnnxModel* TakeC::OnnxRuntimeSystem::LoadModel(const std::string& name, const std::wstring& modelPath) {

	if (!initialized_ || !env_ || !dxCommon_) {
		Logger::Log("\n OnnxRuntimeSystem is not initialized. LoadModel failed.\n");
		return nullptr;
	}

	// すでにロード済みのモデルがある場合はそれを返す
	if (OnnxModel* loadedModel = FindModel(name)) {
		return loadedModel;
	}

	std::filesystem::path resolvedModelPath(modelPath);
	if (!resolvedModelPath.is_absolute()) {
		resolvedModelPath = ResourcePath::Game(resolvedModelPath);
	}

	// 新しいモデルをロード
	auto model = std::make_unique<OnnxModel>();
	try {
		if (!model->Initialize(
			*env_,
			resolvedModelPath.wstring(),
			dmlDevice_.Get(),
			dxCommon_->GetCommandQueue())) {
			Logger::Log("\n OnnxModel load failed.\n");
			return nullptr;
		}
	} catch (const Ort::Exception& e) {
		Logger::Log(std::string("\n OnnxModel load failed: ") + e.what() + "\n");
		return nullptr;
	} catch (const std::exception& e) {
		Logger::Log(std::string("\n OnnxModel load failed: ") + e.what() + "\n");
		return nullptr;
	}

	// モデルをマップに追加
	OnnxModel* modelPtr = model.get();
	models_.emplace(name, std::move(model));
	return modelPtr;
}

//====================================================================
//	onnxモデルの取得
//====================================================================
OnnxModel* TakeC::OnnxRuntimeSystem::FindModel(const std::string& name) {
	auto it = models_.find(name);
	if (it == models_.end()) {
		return nullptr;
	}

	return it->second.get();
}
