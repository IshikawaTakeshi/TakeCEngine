#pragma once
#include "math/AABB.h"
#include "engine/Utility/JsonDirectoryPathData.h"
#include <variant>
#include <cstdint>
#include <json.hpp>

namespace TakeC {


	struct SphereParam {
		float radius = 1.0f;
		uint32_t subDivision = 16;
	};

	struct ConeParam {
		float radius = 1.0f;
		float height = 1.0f;
		uint32_t subDivision = 16;
	};

	struct CubeParam {
		AABB size = { { -0.5f, -0.5f, -0.5f }, { 0.5f, 0.5f, 0.5f } };
	};

	struct RingParam {
		float outerRadius = 1.0f; // 外側の半径
		float innerRadius = 0.01f; // 内側の半径
		uint32_t subDivision = 32; // 分割数
	};

	struct PlaneParam {
		float width = 1.0f;
		float height = 1.0f;
	};

	//各プリミティブのパラメータを保持するためのvariant型
	using PrimitiveParameter = std::variant<
		std::monostate,
		SphereParam,
		ConeParam,
		CubeParam,
		RingParam,
		PlaneParam
	>;

	void to_json(nlohmann::json& jsonData, const PrimitiveParameter& param);
	void from_json(const nlohmann::json& jsonData, PrimitiveParameter& param);

	//JSON変換定義
	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ConeParam, radius, height, subDivision);
	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CubeParam, size);
	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(PlaneParam, width, height);
	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(RingParam, outerRadius, innerRadius, subDivision);
	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(SphereParam, radius, subDivision);

	//JSONディレクトリパス定義
	TAKEC_DEFINE_JSON_DIRECTORY_PATH(PrimitiveParameter, "Resources/JsonLoader/PrimitiveParameters/");
};