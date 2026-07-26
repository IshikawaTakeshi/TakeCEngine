#pragma once
#include <Windows.h>
#include <d3d12.h>
#include <packages/Microsoft.ML.OnnxRuntime.DirectML.1.24.4/build/native/include/onnxruntime_cxx_api.h>
#include <packages/Microsoft.ML.OnnxRuntime.DirectML.1.24.4/build/native/include/dml_provider_factory.h>
#include <packages/Microsoft.AI.DirectML.1.15.4/include/DirectML.h>
#include <vector>
#include <string>
#include <mutex>
#include <memory>

namespace TakeC {

	//====================================================================
	// 	OnnxModel class
	//====================================================================
	/// <summary>
	/// ONNXモデルのセッションを保持し、入力テンソルから推論を実行するクラスです。
	/// </summary>
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
		bool Initialize(Ort::Env& env,const std::wstring& modelPath,IDMLDevice* dmlDevice,ID3D12CommandQueue* commandQueue);

		/// <summary>
		/// 実行処理
		/// </summary>
		/// <param name="inputData"></param>
		/// <param name="inputShape"></param>
		/// <param name="outputData"></param>
		/// <param name="outputShape"></param>
		/// <param name="inputIndex"></param>
		/// <param name="outputIndex"></param>
		/// <returns></returns>
		bool Run(
			const std::vector<float>& inputData,
			const std::vector<int64_t>& inputShape,
			std::vector<float>& outputData,
			std::vector<int64_t>& outputShape,
			size_t inputIndex = 0,
			size_t outputIndex = 0);

		/// <summary>
		/// 複数入力・複数出力の実行処理
		/// </summary>
		/// <param name="inputDataList"></param>
		/// <param name="inputShapeList"></param>
		/// <param name="outputDataList"></param>
		/// <param name="outputShapeList"></param>
		/// <returns></returns>
		bool Run(
			const std::vector<std::vector<float>>& inputDataList,
			const std::vector<std::vector<int64_t>>& inputShapeList,
			std::vector<std::vector<float>>& outputDataList,
			std::vector<std::vector<int64_t>>& outputShapeList);

		bool Run();

		//================================================================
		// getters
		//================================================================

		// 入力・出力の名前を取得
		const std::vector<std::string>& GetInputNames() const { return inputNames_; }
		const std::vector<std::string>& GetOutputNames() const { return outputNames_; }
		// モデルファイルに定義されているshape。dynamic(-1)を含む可能性があり、モデル情報表示用として扱う。
		const std::vector<std::vector<int64_t>>& GetInputShapes() const { return inputShapes_; }
		const std::vector<std::vector<int64_t>>& GetOutputShapes() const { return outputShapes_; }
		// 実際の推論で使用する入力データと、推論後に得られた出力データ。
		const std::vector<std::vector<float>>& GetInputDataList() const { return inputDataList_; }
		const std::vector<std::vector<float>>& GetOutputDataList() const { return outputDataList_; }
		// 実際の推論に使う具体化済みshape。dynamic(-1)はRun前に具体値へ置き換える。
		const std::vector<std::vector<int64_t>>& GetRuntimeInputShapes() const { return runtimeInputShapes_; }
		// 実際の推論結果としてONNX Runtimeから返されたshape。
		const std::vector<std::vector<int64_t>>& GetRuntimeOutputShapes() const { return runtimeOutputShapes_; }
		std::vector<float>* GetInputData(size_t inputIndex = 0);
		const std::vector<int64_t>* GetRuntimeInputShape(size_t inputIndex = 0) const;
		bool ResizeInputBuffer(size_t inputIndex, const std::vector<int64_t>& inputShape);

	private:

		Ort::SessionOptions sessionOptions_;
		std::unique_ptr<Ort::Session> session_;

		std::vector<std::string> inputNames_;
		std::vector<std::string> outputNames_;
		// モデルファイルから読み取ったshape。メタ情報なのでRunで上書きしない。
		std::vector<std::vector<int64_t>> inputShapes_;
		std::vector<std::vector<int64_t>> outputShapes_;
		// 実際の推論で使う/得られるshape。dynamic shapeモデルではこちらが具体値を持つ。
		std::vector<std::vector<int64_t>> runtimeInputShapes_;
		std::vector<std::vector<int64_t>> runtimeOutputShapes_;
		std::vector<std::vector<float>> inputDataList_;
		std::vector<std::vector<float>> outputDataList_;
		std::vector<const char*> inputNamesPtrs_;
		std::vector<const char*> outputNamesPtrs_;

		// DirectML EPでは同一Sessionの並列Runを避ける
		std::mutex runMutex_;

		static size_t CalculateElementCount(const std::vector<int64_t>& shape);

	};

}
