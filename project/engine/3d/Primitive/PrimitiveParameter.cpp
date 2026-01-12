#include "PrimitiveParameter.h"

//============================================================================
// PrimitiveParameter <-- JSON
//============================================================================
void TakeC::to_json(nlohmann::json& jsonData, const PrimitiveParameter& param) {
	std::visit([&jsonData](auto&& arg) {
		using T = std::decay_t<decltype(arg)>;
		if constexpr (std::is_same_v<T, std::monostate>) {
			jsonData = nullptr; // or any representation for monostate
		}
		else {
			jsonData = arg; // 利用可能な型の場合、そのままJSONに変換
		}
		}, param);
}

//============================================================================
// PrimitiveParameter --> JSON
//============================================================================
void TakeC::from_json(const nlohmann::json& jsonData, PrimitiveParameter& param) {
	if (jsonData.is_null()) {
		param = std::monostate{};
		return;
	}
	
	// "type" フィールドに基づいて分岐（例：構造に特徴的なキーがある場合）
	if (jsonData.contains("radius") && jsonData.contains("center")) {
		param = jsonData.get<SphereParam>();
	} else if (jsonData.contains("height") && jsonData.contains("base")) {
		param = jsonData.get<ConeParam>();
	} else if (jsonData.contains("size")) {
		param = jsonData.get<CubeParam>();
	} else if (jsonData.contains("outerRadius") && jsonData.contains("innerRadius")) {
		param = jsonData.get<RingParam>();
	} else if (jsonData.contains("width") && jsonData.contains("height")) {
		param = jsonData.get<PlaneParam>();
	} else {
		param = std::monostate{}; // 不明な型の場合
	}
}