#pragma once
#include <packages/Microsoft.ML.OnnxRuntime.DirectML.1.24.4/build/native/include/onnxruntime_cxx_api.h>
#include <packages/Microsoft.ML.OnnxRuntime.DirectML.1.24.4/build/native/include/dml_provider_factory.h>
#include <vector>
#include <string>
#include <mutex>
#include <memory>

namespace TakeC {

	//====================================================================
	// 	OnnxModel class
	//====================================================================
	class OnnxModel {
	public:

		//================================================================
		// functions
		//================================================================

		/// <summary>
		/// コンストラクタ・デストラクタ
		/// </summary>
		OnnxModel() = default;
		~OnnxModel() = default;

		/// <summary>
		/// 初期化処理
		/// </summary>
		/// <param name="env"></param>
		/// <param name="modelPath"></param>
		/// <param name="dmlDeviceId"></param>
		/// <returns></returns>
		bool Initialize(Ort::Env& env,const std::wstring& modelPath,int dmlDeviceId = 0);

		/// <summary>
		/// 実行処理
		/// </summary>
		/// <param name="inputData"></param>
		/// <param name="inputShapes"></param>
		void Run(const std::vector<float>& inputData, std::vector<int64_t>& inputShapes);

		//================================================================
		// getters
		//================================================================

		// 入力・出力の名前を取得
		const std::vector<std::string>& GetInputNames() const { return inputNames_; }
		const std::vector<std::string>& GetOutputNames() const { return outputNames_; }

	private:

		Ort::SessionOptions sessionOptions_;
		std::unique_ptr<Ort::Session> session_;

		std::vector<std::string> inputNames_;
		std::vector<std::string> outputNames_;
		std::vector<const char*> inputNamesPtrs_;
		std::vector<const char*> outputNamesPtrs_;

		// DirectML EPでは同一Sessionの並列Runを避ける
		std::mutex runMutex_;

	};

}