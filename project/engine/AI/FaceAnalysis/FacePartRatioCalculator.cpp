#include "FacePartRatioCalculator.h"

#include <cmath>

#include "Landmark106Topology.h"

namespace {

	constexpr float kMinimumReferenceLength = 0.0001f;

	float HorizontalDistance(const Vector2& a, const Vector2& b) {
		return std::abs(b.x - a.x);
	}

	float VerticalDistance(const Vector2& a, const Vector2& b) {
		return std::abs(b.y - a.y);
	}

	Vector2 Midpoint(const Vector2& a, const Vector2& b) {
		return {
			(a.x + b.x) * 0.5f,
			(a.y + b.y) * 0.5f,
		};
	}

	Vector2 AveragePoint(
		const std::vector<Vector2>& landmarks,
		const TakeC::Landmark106IndexRange& range) {

		Vector2 average = {};
		for (size_t i = range.first; i <= range.last; ++i) {
			average.x += landmarks[i].x;
			average.y += landmarks[i].y;
		}

		const float count = static_cast<float>(range.Count());
		average.x /= count;
		average.y /= count;
		return average;
	}

	float SafeDivide(float numerator, float denominator) {
		return std::abs(denominator) > kMinimumReferenceLength
			? numerator / denominator
			: 0.0f;
	}

	TakeC::NormalizedFacePartSize BuildNormalizedSize(
		float width,
		float height,
		float faceWidth,
		float faceHeight) {

		TakeC::NormalizedFacePartSize result;
		result.widthByFaceWidth = SafeDivide(width, faceWidth);
		result.heightByFaceHeight = SafeDivide(height, faceHeight);
		result.widthToHeight = SafeDivide(width, height);
		return result;
	}

	bool ContainsOnlyFinitePoints(const std::vector<Vector2>& landmarks) {
		if (landmarks.size() < TakeC::Landmark106Topology::kLandmarkCount) {
			return false;
		}

		for (size_t i = 0; i < TakeC::Landmark106Topology::kLandmarkCount; ++i) {
			if (!std::isfinite(landmarks[i].x) || !std::isfinite(landmarks[i].y)) {
				return false;
			}
		}
		return true;
	}

}

std::optional<TakeC::FacePartRatios> TakeC::FacePartRatioCalculator::Calculate(
	const std::vector<Vector2>& landmarks) {

	if (!ContainsOnlyFinitePoints(landmarks)) {
		return std::nullopt;
	}

	using namespace Landmark106Topology;
	using namespace Landmark106Topology::Anchor;

	const float faceWidth = HorizontalDistance(
		landmarks[kImageLeftFaceEdge],
		landmarks[kImageRightFaceEdge]);
	const Vector2 upperFaceCenter = Midpoint(
		landmarks[kImageLeftFaceEdge],
		landmarks[kImageRightFaceEdge]);
	const float faceHeight = VerticalDistance(upperFaceCenter, landmarks[kChin]);
	if (faceWidth <= kMinimumReferenceLength || faceHeight <= kMinimumReferenceLength) {
		return std::nullopt;
	}

	const float imageLeftEyeWidth = HorizontalDistance(
		landmarks[kImageLeftEyeOuterCorner],
		landmarks[kImageLeftEyeInnerCorner]);
	const float imageLeftEyeHeight =
		(VerticalDistance(landmarks[kImageLeftEyeUpperOuter], landmarks[kImageLeftEyeLowerOuter]) +
			VerticalDistance(landmarks[kImageLeftEyeUpperInner], landmarks[kImageLeftEyeLowerInner])) *
		0.5f;

	const float imageRightEyeWidth = HorizontalDistance(
		landmarks[kImageRightEyeInnerCorner],
		landmarks[kImageRightEyeOuterCorner]);
	const float imageRightEyeHeight =
		(VerticalDistance(landmarks[kImageRightEyeUpperInner], landmarks[kImageRightEyeLowerInner]) +
			VerticalDistance(landmarks[kImageRightEyeUpperOuter], landmarks[kImageRightEyeLowerOuter])) *
		0.5f;

	const float noseWidth = HorizontalDistance(
		landmarks[kImageLeftNostril],
		landmarks[kImageRightNostril]);
	const float noseHeight = VerticalDistance(
		landmarks[kNoseBridgeTop],
		landmarks[kNoseBaseCenter]);

	const float mouthWidth = HorizontalDistance(
		landmarks[kImageLeftMouthCorner],
		landmarks[kImageRightMouthCorner]);
	const float mouthHeight = VerticalDistance(
		landmarks[kOuterUpperLipCenter],
		landmarks[kOuterLowerLipCenter]);

	const Vector2 imageLeftEyeCenter = AveragePoint(landmarks, kImageLeftEye);
	const Vector2 imageRightEyeCenter = AveragePoint(landmarks, kImageRightEye);
	const Vector2 imageLeftEyebrowCenter = AveragePoint(landmarks, kImageLeftEyebrow);
	const Vector2 imageRightEyebrowCenter = AveragePoint(landmarks, kImageRightEyebrow);

	FacePartRatios result;
	result.faceWidthPixels = faceWidth;
	result.faceHeightPixels = faceHeight;
	result.faceHeightToWidth = faceHeight / faceWidth;
	result.cheekWidthByFaceWidth = HorizontalDistance(
		landmarks[kImageLeftCheek],
		landmarks[kImageRightCheek]) / faceWidth;
	result.lowerJawWidthByFaceWidth = HorizontalDistance(
		landmarks[kImageLeftLowerJaw],
		landmarks[kImageRightLowerJaw]) / faceWidth;

	result.imageLeftEye = BuildNormalizedSize(
		imageLeftEyeWidth,
		imageLeftEyeHeight,
		faceWidth,
		faceHeight);
	result.imageRightEye = BuildNormalizedSize(
		imageRightEyeWidth,
		imageRightEyeHeight,
		faceWidth,
		faceHeight);
	result.eyeGapByFaceWidth = HorizontalDistance(
		landmarks[kImageLeftEyeInnerCorner],
		landmarks[kImageRightEyeInnerCorner]) / faceWidth;

	result.imageLeftEyebrowWidthByFaceWidth = HorizontalDistance(
		landmarks[kImageLeftEyebrowOuter],
		landmarks[kImageLeftEyebrowInner]) / faceWidth;
	result.imageRightEyebrowWidthByFaceWidth = HorizontalDistance(
		landmarks[kImageRightEyebrowInner],
		landmarks[kImageRightEyebrowOuter]) / faceWidth;
	result.imageLeftEyebrowToEyeByFaceHeight =
		VerticalDistance(imageLeftEyebrowCenter, imageLeftEyeCenter) / faceHeight;
	result.imageRightEyebrowToEyeByFaceHeight =
		VerticalDistance(imageRightEyebrowCenter, imageRightEyeCenter) / faceHeight;

	result.nose = BuildNormalizedSize(noseWidth, noseHeight, faceWidth, faceHeight);
	result.mouth = BuildNormalizedSize(mouthWidth, mouthHeight, faceWidth, faceHeight);
	result.mouthOpeningByFaceHeight = VerticalDistance(
		landmarks[kInnerUpperLipCenter],
		landmarks[kInnerLowerLipCenter]) / faceHeight;
	result.noseToMouthByFaceHeight = VerticalDistance(
		landmarks[kNoseBaseCenter],
		landmarks[kOuterUpperLipCenter]) / faceHeight;
	result.mouthToChinByFaceHeight = VerticalDistance(
		landmarks[kOuterLowerLipCenter],
		landmarks[kChin]) / faceHeight;

	return result;
}
