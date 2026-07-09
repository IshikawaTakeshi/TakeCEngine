#include "ScrfdDecoder.h"

#include <algorithm>
#include <cmath>
#include <utility>

//================================================================
//		デコード
//================================================================
std::vector<FaceDetectionResult> ScrfdDecoder::Decode(
	const std::vector<std::string>& outputNames, 
	const std::vector<std::vector<float>>& outputDataList, 
	const std::vector<std::vector<int64_t>>& outputShapeList, 
	const Config& config) {

	std::vector<OutputSet> outputSets;

	// アウトプットセットの構築
	if (!BuildOutputSets(outputNames, outputDataList, outputShapeList, config, outputSets)) {
		return {};
	}

	// candidatesにスコアが閾値以上の検出結果を追加
	std::vector<FaceDetectionResult> candidates;

	// 各出力セットを処理
	for (const OutputSet& set : outputSets) {
		if (!set.scores.data || !set.boxes.data || !set.landmarks.data) {
			continue;
		}

		// 各アンカーを処理
		for (int anchorIndex = 0; anchorIndex < set.scores.elementCount; ++anchorIndex) {
			const int scoreOffset = anchorIndex * set.scores.channels;
			//スコアの取得
			const float score = (*set.scores.data)[scoreOffset + set.scores.channels - 1];

			// スコアが閾値未満の場合はスキップ
			if (score < config.scoreThreshold) {
				continue;
			}

			// アンカーの位置を計算
			const int cellIndex = anchorIndex / set.anchorsPerCell;
			const int gridX = cellIndex % set.featureWidth;
			const int gridY = cellIndex / set.featureWidth;
			const float centerX = (static_cast<float>(gridX) + config.anchorCenterOffset) * static_cast<float>(set.stride);
			const float centerY = (static_cast<float>(gridY) + config.anchorCenterOffset) * static_cast<float>(set.stride);

			// バウンディングボックスの座標を取得
			const int boxOffset = anchorIndex * set.boxes.channels;
			float left = (*set.boxes.data)[boxOffset + 0];
			float top = (*set.boxes.data)[boxOffset + 1];
			float right = (*set.boxes.data)[boxOffset + 2];
			float bottom = (*set.boxes.data)[boxOffset + 3];

			// ストライドを掛けるかどうかの設定に応じて、バウンディングボックスの座標を調整
			if (config.multiplyByStride) {
				left *= static_cast<float>(set.stride);
				top *= static_cast<float>(set.stride);
				right *= static_cast<float>(set.stride);
				bottom *= static_cast<float>(set.stride);
			}

			// バウンディングボックスの座標を計算
			FaceDetectionResult result;
			result.score = score;
			result.bboxMin = { centerX - left + config.decodeOffsetX, centerY - top + config.decodeOffsetY };
			result.bboxMax = { centerX + right + config.decodeOffsetX, centerY + bottom + config.decodeOffsetY };
			
			// ランドマークの座標を取得
			const int landmarkOffset = anchorIndex * set.landmarks.channels;
			Vector2* landmarkPoints[5] = {
				&result.landmark.leftEye,
				&result.landmark.rightEye,
				&result.landmark.nose,
				&result.landmark.leftMouth,
				&result.landmark.rightMouth,
			};

			for (int i = 0; i < 5; ++i) {
				float x = (*set.landmarks.data)[landmarkOffset + i * 2 + 0];
				float y = (*set.landmarks.data)[landmarkOffset + i * 2 + 1];
				if (config.multiplyByStride) {
					x *= static_cast<float>(set.stride);
					y *= static_cast<float>(set.stride);
				}
				*landmarkPoints[i] = { centerX + x + config.decodeOffsetX, centerY + y + config.decodeOffsetY };
			}

			ClampToInputSize(result, config.inputWidth, config.inputHeight);
			candidates.push_back(result);
		}
	}

	std::sort(candidates.begin(), candidates.end(), [](const FaceDetectionResult& a, const FaceDetectionResult& b) {
		return a.score > b.score;
		});

	return ApplyNms(candidates, config.nmsThreshold, config.maxDetections);
}

//================================================================
//		出力セットの構築
//================================================================
bool ScrfdDecoder::BuildOutputSets(
	const std::vector<std::string>& outputNames,
	const std::vector<std::vector<float>>& outputDataList,
	const std::vector<std::vector<int64_t>>& outputShapeList,
	const Config& config,
	std::vector<OutputSet>& outputSets) {

	static_cast<void>(outputNames);

	if (outputDataList.size() < config.strides.size() * 3 || outputShapeList.size() < config.strides.size() * 3) {
		return false;
	}

	outputSets.clear();
	outputSets.reserve(config.strides.size());

	// SCRFDの一般的な9出力順:
	// scores(stride8,16,32), bboxes(stride8,16,32), landmarks(stride8,16,32)
	for (size_t i = 0; i < config.strides.size(); ++i) {
		OutputSet set;
		set.stride = config.strides[i];
		set.featureWidth = config.inputWidth / set.stride;
		set.featureHeight = config.inputHeight / set.stride;
		if (set.featureWidth <= 0 || set.featureHeight <= 0) {
			return false;
		}

		const size_t scoreIndex = i;
		const size_t boxIndex = i + config.strides.size();
		const size_t landmarkIndex = i + config.strides.size() * 2;

		set.scores.data = &outputDataList[scoreIndex];
		set.scores.shape = outputShapeList[scoreIndex];
		set.boxes.data = &outputDataList[boxIndex];
		set.boxes.shape = outputShapeList[boxIndex];
		set.landmarks.data = &outputDataList[landmarkIndex];
		set.landmarks.shape = outputShapeList[landmarkIndex];

		if (!ResolveTensorLayout(set.scores.shape, set.scores.data->size(), 1, set.scores.elementCount, set.scores.channels) ||
			!ResolveTensorLayout(set.boxes.shape, set.boxes.data->size(), 4, set.boxes.elementCount, set.boxes.channels) ||
			!ResolveTensorLayout(set.landmarks.shape, set.landmarks.data->size(), 10, set.landmarks.elementCount, set.landmarks.channels)) {
			return false;
		}

		const int expectedCells = set.featureWidth * set.featureHeight;
		if (expectedCells <= 0 || set.scores.elementCount % expectedCells != 0) {
			return false;
		}

		set.anchorsPerCell = set.scores.elementCount / expectedCells;
		if (set.anchorsPerCell <= 0 ||
			set.boxes.elementCount != set.scores.elementCount ||
			set.landmarks.elementCount != set.scores.elementCount ||
			set.boxes.channels < 4 ||
			set.landmarks.channels < 10) {
			return false;
		}

		outputSets.push_back(std::move(set));
	}

	return true;
}

//================================================================
//		テンソルレイアウトの解決
//================================================================
bool ScrfdDecoder::ResolveTensorLayout(const std::vector<int64_t>& shape, size_t dataSize, int preferredChannels, int& elementCount, int& channels) {
	elementCount = 0;
	channels = 0;

	if (dataSize == 0 || preferredChannels <= 0) {
		return false;
	}

	if (!shape.empty()) {
		const int64_t lastDimension = shape.back();
		if (lastDimension > 0 && dataSize % static_cast<size_t>(lastDimension) == 0) {
			channels = static_cast<int>(lastDimension);
			elementCount = static_cast<int>(dataSize / static_cast<size_t>(channels));
			return elementCount > 0;
		}
	}

	if (dataSize % static_cast<size_t>(preferredChannels) != 0) {
		return false;
	}

	channels = preferredChannels;
	elementCount = static_cast<int>(dataSize / static_cast<size_t>(channels));
	return elementCount > 0;
}

//================================================================
//		IoUの計算
//================================================================
float ScrfdDecoder::CalculateIoU(const FaceDetectionResult& a, const FaceDetectionResult& b) {
	const float x1 = std::max(a.bboxMin.x, b.bboxMin.x);
	const float y1 = std::max(a.bboxMin.y, b.bboxMin.y);
	const float x2 = std::min(a.bboxMax.x, b.bboxMax.x);
	const float y2 = std::min(a.bboxMax.y, b.bboxMax.y);
	const float width = std::max(0.0f, x2 - x1);
	const float height = std::max(0.0f, y2 - y1);
	const float intersection = width * height;

	const float areaA = std::max(0.0f, a.bboxMax.x - a.bboxMin.x) * std::max(0.0f, a.bboxMax.y - a.bboxMin.y);
	const float areaB = std::max(0.0f, b.bboxMax.x - b.bboxMin.x) * std::max(0.0f, b.bboxMax.y - b.bboxMin.y);
	const float unionArea = areaA + areaB - intersection;
	if (unionArea <= 0.0f) {
		return 0.0f;
	}

	return intersection / unionArea;
}

//================================================================
//		NMSの適用
//================================================================
std::vector<FaceDetectionResult> ScrfdDecoder::ApplyNms(const std::vector<FaceDetectionResult>& candidates, float threshold, int maxDetections) {
	std::vector<FaceDetectionResult> results;
	std::vector<bool> suppressed(candidates.size(), false);

	for (size_t i = 0; i < candidates.size(); ++i) {
		if (suppressed[i]) {
			continue;
		}

		results.push_back(candidates[i]);
		if (maxDetections > 0 && static_cast<int>(results.size()) >= maxDetections) {
			break;
		}

		for (size_t j = i + 1; j < candidates.size(); ++j) {
			if (!suppressed[j] && CalculateIoU(candidates[i], candidates[j]) > threshold) {
				suppressed[j] = true;
			}
		}
	}

	return results;
}

//================================================================
//		入力サイズにクランプ
//================================================================
void ScrfdDecoder::ClampToInputSize(FaceDetectionResult& result, int inputWidth, int inputHeight) {
	const float maxX = static_cast<float>(std::max(inputWidth - 1, 0));
	const float maxY = static_cast<float>(std::max(inputHeight - 1, 0));

	result.bboxMin.x = std::clamp(result.bboxMin.x, 0.0f, maxX);
	result.bboxMin.y = std::clamp(result.bboxMin.y, 0.0f, maxY);
	result.bboxMax.x = std::clamp(result.bboxMax.x, 0.0f, maxX);
	result.bboxMax.y = std::clamp(result.bboxMax.y, 0.0f, maxY);

	Vector2* landmarkPoints[5] = {
		&result.landmark.leftEye,
		&result.landmark.rightEye,
		&result.landmark.nose,
		&result.landmark.leftMouth,
		&result.landmark.rightMouth,
	};

	for (Vector2* point : landmarkPoints) {
		point->x = std::clamp(point->x, 0.0f, maxX);
		point->y = std::clamp(point->y, 0.0f, maxY);
	}
}
