#pragma once

#include <array>
#include <cstddef>

namespace TakeC {

	/// <summary>
	/// InsightFace 2d106detのランドマーク範囲
	/// </summary>
	struct Landmark106IndexRange {
		size_t first = 0;
		size_t last = 0;

		constexpr size_t Count() const {
			return last >= first ? last - first + 1 : 0;
		}

		constexpr bool Contains(size_t index) const {
			return index >= first && index <= last;
		}
	};

	namespace Landmark106Topology {

		inline constexpr size_t kLandmarkCount = 106;

		// 2d106detの出力順。左右は画像座標上の左右を表す。
		inline constexpr Landmark106IndexRange kFaceContour = { 0, 32 };
		inline constexpr Landmark106IndexRange kImageLeftEye = { 33, 42 };
		inline constexpr Landmark106IndexRange kImageLeftEyebrow = { 43, 51 };
		inline constexpr Landmark106IndexRange kMouth = { 52, 71 };
		inline constexpr Landmark106IndexRange kNose = { 72, 86 };
		inline constexpr Landmark106IndexRange kImageRightEye = { 87, 96 };
		inline constexpr Landmark106IndexRange kImageRightEyebrow = { 97, 105 };

		/// <summary>
		/// 標準68点形式の0..67に対応する2d106det側のインデックス
		/// </summary>
		inline constexpr std::array<size_t, 68> kStandard68Indices = {
			// Face contour
			1, 10, 12, 14, 16, 3, 5, 7, 0, 23, 21, 19, 32, 30, 28, 26, 17,
			// Image-left eyebrow
			43, 48, 49, 51, 50,
			// Image-right eyebrow
			102, 103, 104, 105, 101,
			// Nose
			72, 73, 74, 86, 78, 79, 80, 85, 84,
			// Image-left eye
			35, 41, 42, 39, 37, 36,
			// Image-right eye
			89, 95, 96, 93, 91, 90,
			// Mouth
			52, 64, 63, 71, 67, 68, 61, 58, 59, 53,
			56, 55, 65, 66, 62, 70, 69, 57, 60, 54,
		};

		namespace Anchor {

			// Face contour
			inline constexpr size_t kImageLeftFaceEdge = 1;
			inline constexpr size_t kChin = 0;
			inline constexpr size_t kImageRightFaceEdge = 17;
			inline constexpr size_t kImageLeftCheek = 16;
			inline constexpr size_t kImageRightCheek = 32;
			inline constexpr size_t kImageLeftLowerJaw = 5;
			inline constexpr size_t kImageRightLowerJaw = 21;

			// Image-left eye
			inline constexpr size_t kImageLeftEyeOuterCorner = 35;
			inline constexpr size_t kImageLeftEyeUpperOuter = 41;
			inline constexpr size_t kImageLeftEyeUpperInner = 42;
			inline constexpr size_t kImageLeftEyeInnerCorner = 39;
			inline constexpr size_t kImageLeftEyeLowerInner = 37;
			inline constexpr size_t kImageLeftEyeLowerOuter = 36;

			// Image-right eye
			inline constexpr size_t kImageRightEyeInnerCorner = 89;
			inline constexpr size_t kImageRightEyeUpperInner = 95;
			inline constexpr size_t kImageRightEyeUpperOuter = 96;
			inline constexpr size_t kImageRightEyeOuterCorner = 93;
			inline constexpr size_t kImageRightEyeLowerOuter = 91;
			inline constexpr size_t kImageRightEyeLowerInner = 90;

			// Eyebrows
			inline constexpr size_t kImageLeftEyebrowOuter = 43;
			inline constexpr size_t kImageLeftEyebrowInner = 50;
			inline constexpr size_t kImageRightEyebrowInner = 102;
			inline constexpr size_t kImageRightEyebrowOuter = 101;

			// Nose
			inline constexpr size_t kNoseBridgeTop = 72;
			inline constexpr size_t kNoseTip = 86;
			inline constexpr size_t kImageLeftNostril = 78;
			inline constexpr size_t kNoseBaseCenter = 80;
			inline constexpr size_t kImageRightNostril = 84;

			// Mouth
			inline constexpr size_t kImageLeftMouthCorner = 52;
			inline constexpr size_t kOuterUpperLipCenter = 71;
			inline constexpr size_t kImageRightMouthCorner = 61;
			inline constexpr size_t kOuterLowerLipCenter = 53;
			inline constexpr size_t kInnerUpperLipCenter = 62;
			inline constexpr size_t kInnerLowerLipCenter = 60;

		}

		static_assert(kFaceContour.Count() +
			kImageLeftEye.Count() +
			kImageLeftEyebrow.Count() +
			kMouth.Count() +
			kNose.Count() +
			kImageRightEye.Count() +
			kImageRightEyebrow.Count() == kLandmarkCount);

	}

}
