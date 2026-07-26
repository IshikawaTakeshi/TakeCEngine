#pragma once
#include "FaceDetectionContext.h"
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

//==============================================================================================
//		ScrfdDecoder class
//==============================================================================================
/// <summary>
/// モデルの出力を解析し、Scrfd結果へ変換するクラスです。
/// </summary>
class ScrfdDecoder {
public:

	/// <summary>
	/// デコード設定
	/// </summary>
	struct Config {
		int inputWidth = 640;
		int inputHeight = 640;
		std::vector<int> strides = { 8, 16, 32 };
		float scoreThreshold = 0.5f;
		float nmsThreshold = 0.4f;
		int maxDetections = 100;
		bool multiplyByStride = true;
		float anchorCenterOffset = 0.0f;
		float decodeOffsetX = 0.0f;
		float decodeOffsetY = 0.0f;
	};

private:

	/// <summary>
	/// 出力テンソル情報
	/// </summary>
	struct OutputTensor {
		const std::vector<float>* data = nullptr;
		std::vector<int64_t> shape;
		int elementCount = 0;
		int channels = 0;
	};

	/// <summary>
	/// 出力セット情報
	/// </summary>
	struct OutputSet {
		OutputTensor scores;
		OutputTensor boxes;
		OutputTensor landmarks;
		int stride = 0;
		int featureWidth = 0;
		int featureHeight = 0;
		int anchorsPerCell = 1;
	};

public:

	//==========================================================================================
	// functions
	//==========================================================================================

	/// <summary>
	/// コンストラクタ・デストラクタ
	/// </summary>
	ScrfdDecoder() = default;
	~ScrfdDecoder() = default;

	/// <summary>
	/// デコード
	/// </summary>
	/// <param name="scores"></param>
	/// <param name="bboxes"></param>
	/// <param name="landmarks"></param>
	/// <param name="numAnchors"></param>
	/// <param name="config"></param>
	/// <returns></returns>
	std::vector <FaceDetectionResult> Decode(
		const std::vector<std::string>& outputNames,
		const std::vector<std::vector<float>>& outputDataList,
		const std::vector<std::vector<int64_t>>& outputShapeList,
		const Config& config);

private:

	/// <summary>
	/// 出力セットの構築
	/// </summary>
	bool BuildOutputSets(
		const std::vector<std::string>& outputNames,
		const std::vector<std::vector<float>>& outputDataList,
		const std::vector<std::vector<int64_t>>& outputShapeList,
		const Config& config,
		std::vector<OutputSet>& outputSets);

	/// <summary>
	/// テンソルレイアウトの解決
	/// </summary>
	/// <returns></returns>
	static bool ResolveTensorLayout(const std::vector<int64_t>& shape, size_t dataSize, int preferredChannels, int& elementCount, int& channels);
	
	/// <summary>
	/// IoUの計算
	/// </summary>
	static float CalculateIoU(const FaceDetectionResult& a, const FaceDetectionResult& b);

	/// <summary>
	/// Non-Maximum Suppressionの適用
	/// </summary>
	static std::vector<FaceDetectionResult> ApplyNms(const std::vector<FaceDetectionResult>& candidates, float threshold, int maxDetections);

	/// <summary>
	/// 入力サイズにクランプ
	/// </summary>
	static void ClampToInputSize(FaceDetectionResult& result, int inputWidth, int inputHeight);
};
