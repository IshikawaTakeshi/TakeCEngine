#include "mathEnv.h"
#include "engine/Utility/StringUtility.h"
#include "engine/Utility/Logger.h"

//============================================================================
// radianDegree変換関数
//============================================================================
float radianToDegree(float radian) {
	return radian * kRad2Deg;
}

//============================================================================
// degreeToRadian変換関数
//============================================================================
float degreeToRadian(float degree) {
	return degree * kDeg2Rad;
}
//============================================================================
// 乱数生成エンジン取得関数
//============================================================================
std::mt19937& GetRandomEngine() {

	static std::random_device rd;
	static std::mt19937 engine(rd());
	return engine;
}

//============================================================================
// 乱数生成関数（float）
//============================================================================
float GetRandomFloat(float min, float max) {

	std::uniform_real_distribution<float> dist(min, max);
	return dist(GetRandomEngine());
}

//============================================================================
// 乱数生成関数（int）
//============================================================================
int GetRandomInt(int min, int max) {

	std::uniform_int_distribution<int> dist(min, max);
	return dist(GetRandomEngine());
}

uint64_t Measure(const wchar_t* label, const std::function<void()>& fn) {
	auto start = Clock::now();
	fn();
	auto end = Clock::now();
	auto ms = std::chrono::duration_cast<Ms>(end - start).count();
	Logger::Log(StringUtility::ConvertString(std::wstring(L"[LoadTime] ") + label + L" : " + std::to_wstring(ms) + L"ms"));
	return ms;
}
