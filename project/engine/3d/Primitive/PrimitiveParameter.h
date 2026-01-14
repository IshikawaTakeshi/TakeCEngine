#pragma once
#include "math/AABB.h"
#include "engine/Utility/JsonDirectoryPathData.h"
#include <variant>
#include <cstdint>
#include <json.hpp>

namespace TakeC {

	/// <summary>
	/// SphereParam構造体
	/// </summary>
	struct SphereParam {
		float radius = 1.0f;       // 半径
		uint32_t subDivision = 16; // 分割数
	};

	/// <summary>
	/// ConeParam構造体
	/// </summary>
	struct ConeParam {
		float radius = 1.0f;       // 底面の半径
		float height = 1.0f;       // 高さ
		uint32_t subDivision = 16; // 分割数
	};

	/// <summary>
	/// CubeParam構造体
	/// </summary>
	struct CubeParam {
		AABB size = { // サイズ
			{ -0.5f, -0.5f, -0.5f }, // min
			{ 0.5f, 0.5f, 0.5f }     // max
		};
	};

	/// <summary>
	/// RingParam構造体
	/// </summary>
	struct RingParam {
		float outerRadius = 1.0f; // 外側の半径
		float innerRadius = 0.01f; // 内側の半径
		uint32_t subDivision = 32; // 分割数
	};

	/// <summary>
	/// PlaneParam構造体
	/// </summary>
	struct PlaneParam {
		float width = 1.0f;  // 幅
		float height = 1.0f; // 高さ
	};

	/// <summary>
	/// CylinderParam構造体
	/// </summary>
	struct CylinderParam {
		float radius = 1.0f;       // 半径
		float height = 1.0f;       // 高さ
		uint32_t subDivision = 16; // 分割数
	};

	//各プリミティブのパラメータを保持するためのvariant型
	using PrimitiveParameter = std::variant<
		std::monostate,
		SphereParam,
		ConeParam,
		CubeParam,
		RingParam,
		PlaneParam,
		CylinderParam
	>;

	//JSON変換定義
	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ConeParam, radius, height, subDivision)
	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CubeParam, size)
	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(PlaneParam, width, height)
	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(RingParam, outerRadius, innerRadius, subDivision)
	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(SphereParam, radius, subDivision)
	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CylinderParam, radius, height, subDivision)

	//JSONディレクトリパス定義
	TAKEC_DEFINE_JSON_DIRECTORY_PATH(PrimitiveParameter, "Resources/JsonLoader/PrimitiveParameters/");
};