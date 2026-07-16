#include "FaceAnalysisDebugRenderer.h"

#include <algorithm>

#include "engine/CameraCapture/CameraCapture.h"
#include "externals/imgui/imgui.h"

std::string TakeC::FaceAnalysisDebugRenderer::ShapeToString(
	const std::vector<int64_t>& shape) {

	if (shape.empty()) {
		return "[]";
	}

	std::string result = "[";
	for (size_t i = 0; i < shape.size(); ++i) {
		if (i != 0) {
			result += ", ";
		}
		result += shape[i] < 0 ? "dynamic" : std::to_string(shape[i]);
	}
	result += "]";
	return result;
}

void TakeC::FaceAnalysisDebugRenderer::DrawOnnxTensorInfo(
	const char* label,
	const std::vector<std::string>& names,
	const std::vector<std::vector<int64_t>>& shapes) {

	if (!ImGui::TreeNode(label)) {
		return;
	}

	for (size_t i = 0; i < names.size(); ++i) {
		const std::string shape = i < shapes.size() ? ShapeToString(shapes[i]) : "[]";
		ImGui::Text("Name : %s", names[i].c_str());
		ImGui::Text("Shape: %s", shape.c_str());
		ImGui::Separator();
	}

	ImGui::TreePop();
}

void TakeC::FaceAnalysisDebugRenderer::DrawFaceDetectionResult(
	const FaceDetectionResult& result,
	size_t index) {

	ImGui::Text("Face %d score: %.3f", static_cast<int>(index), result.score);
	ImGui::Text(
		"bbox: min(%.1f, %.1f) max(%.1f, %.1f)",
		result.bboxMin.x,
		result.bboxMin.y,
		result.bboxMax.x,
		result.bboxMax.y);
	ImGui::Text("leftEye : %.1f, %.1f", result.landmark.leftEye.x, result.landmark.leftEye.y);
	ImGui::Text("rightEye: %.1f, %.1f", result.landmark.rightEye.x, result.landmark.rightEye.y);
	ImGui::Text("nose    : %.1f, %.1f", result.landmark.nose.x, result.landmark.nose.y);
	ImGui::Text("leftMouth : %.1f, %.1f", result.landmark.leftMouth.x, result.landmark.leftMouth.y);
	ImGui::Text("rightMouth: %.1f, %.1f", result.landmark.rightMouth.x, result.landmark.rightMouth.y);
	ImGui::Separator();
}

void TakeC::FaceAnalysisDebugRenderer::DrawFaceDetectionOverlay(
	const CameraCapture& cameraCapture,
	const std::vector<FaceDetectionResult>& results,
	int inputWidth,
	int inputHeight) {

	if (!cameraCapture.HasLastImGuiImageRect() || inputWidth <= 0 || inputHeight <= 0) {
		return;
	}

	const CameraCapture::ImGuiImageRect& rect = cameraCapture.GetLastImGuiImageRect();
	const float imageWidth = rect.maxX - rect.minX;
	const float imageHeight = rect.maxY - rect.minY;
	if (imageWidth <= 0.0f || imageHeight <= 0.0f) {
		return;
	}

	ImDrawList* drawList = ImGui::GetForegroundDrawList();
	const ImU32 boxColor = IM_COL32(0, 255, 80, 255);
	const ImU32 landmarkColor = IM_COL32(255, 80, 40, 255);
	const ImU32 textColor = IM_COL32(255, 255, 255, 255);
	const float scaleX = imageWidth / static_cast<float>(inputWidth);
	const float scaleY = imageHeight / static_cast<float>(inputHeight);

	const auto toScreen = [&](const Vector2& point) {
		return ImVec2(rect.minX + point.x * scaleX, rect.minY + point.y * scaleY);
	};

	for (size_t i = 0; i < results.size(); ++i) {
		const FaceDetectionResult& result = results[i];
		const ImVec2 min = toScreen(result.bboxMin);
		const ImVec2 max = toScreen(result.bboxMax);
		drawList->AddRect(min, max, boxColor, 0.0f, 0, 2.0f);
		drawList->AddText(ImVec2(min.x, min.y - 16.0f), textColor, std::to_string(i).c_str());

		const Vector2 landmarks[5] = {
			result.landmark.leftEye,
			result.landmark.rightEye,
			result.landmark.nose,
			result.landmark.leftMouth,
			result.landmark.rightMouth,
		};

		for (const Vector2& landmark : landmarks) {
			const ImVec2 center = toScreen(landmark);
			drawList->AddCircle(center, 4.0f, landmarkColor, 12, 2.0f);
			drawList->AddLine(
				ImVec2(center.x - 5.0f, center.y),
				ImVec2(center.x + 5.0f, center.y),
				landmarkColor,
				1.5f);
			drawList->AddLine(
				ImVec2(center.x, center.y - 5.0f),
				ImVec2(center.x, center.y + 5.0f),
				landmarkColor,
				1.5f);
		}
	}
}

void TakeC::FaceAnalysisDebugRenderer::DrawLandmark106Canvas(
	const std::vector<Vector2>& points,
	int width,
	int height) {

	if (points.empty() ||
		width <= 0 ||
		height <= 0 ||
		!ImGui::TreeNode("106 Landmark Preview")) {
		return;
	}

	const float canvasSize = std::min(ImGui::GetContentRegionAvail().x, 360.0f);
	const ImVec2 origin = ImGui::GetCursorScreenPos();
	const ImVec2 canvasEnd = ImVec2(origin.x + canvasSize, origin.y + canvasSize);
	ImDrawList* drawList = ImGui::GetWindowDrawList();
	drawList->AddRectFilled(origin, canvasEnd, IM_COL32(20, 20, 20, 255));
	drawList->AddRect(origin, canvasEnd, IM_COL32(120, 120, 120, 255));

	const float scaleX = canvasSize / static_cast<float>(width);
	const float scaleY = canvasSize / static_cast<float>(height);
	for (size_t i = 0; i < points.size(); ++i) {
		const ImVec2 point = ImVec2(
			origin.x + points[i].x * scaleX,
			origin.y + points[i].y * scaleY);
		drawList->AddCircleFilled(point, 2.0f, IM_COL32(255, 180, 40, 255), 8);
		if (i == 0 || i + 1 == points.size()) {
			drawList->AddText(
				ImVec2(point.x + 3.0f, point.y + 3.0f),
				IM_COL32(255, 255, 255, 255),
				std::to_string(i).c_str());
		}
	}

	ImGui::Dummy(ImVec2(canvasSize, canvasSize));
	ImGui::TreePop();
}
