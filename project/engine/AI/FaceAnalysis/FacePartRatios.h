#pragma once

namespace TakeC {

	/// <summary>
	/// 顔幅・顔高で正規化した顔パーツのサイズ
	/// </summary>
	struct NormalizedFacePartSize {
		float widthByFaceWidth = 0.0f;
		float heightByFaceHeight = 0.0f;
		float widthToHeight = 0.0f;
	};

	/// <summary>
	/// 106点ランドマークから計算した、スケールに依存しない顔パーツ比率
	/// </summary>
	struct FacePartRatios {
		// 顔幅: 点1と17の水平距離
		// 顔高: 点1と17の中点から、顎先の点0までの垂直距離
		float faceWidthPixels = 0.0f;
		float faceHeightPixels = 0.0f;
		float faceHeightToWidth = 0.0f;

		// 顔輪郭
		float cheekWidthByFaceWidth = 0.0f;
		float lowerJawWidthByFaceWidth = 0.0f;

		// 目・眉。左右は画像座標上の左右。
		NormalizedFacePartSize imageLeftEye;
		NormalizedFacePartSize imageRightEye;
		float eyeGapByFaceWidth = 0.0f;
		float imageLeftEyebrowWidthByFaceWidth = 0.0f;
		float imageRightEyebrowWidthByFaceWidth = 0.0f;
		float imageLeftEyebrowToEyeByFaceHeight = 0.0f;
		float imageRightEyebrowToEyeByFaceHeight = 0.0f;

		// 鼻
		NormalizedFacePartSize nose;

		// 口
		NormalizedFacePartSize mouth;
		float mouthOpeningByFaceHeight = 0.0f;
		float noseToMouthByFaceHeight = 0.0f;
		float mouthToChinByFaceHeight = 0.0f;
	};

}
