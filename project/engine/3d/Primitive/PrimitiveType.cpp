#include "PrimitiveType.h"
#include "engine/Utility/StringUtility.h"

//============================================================================
// JSON <- PrimitiveType
//============================================================================
void to_json(nlohmann::json& j, const PrimitiveType& type) {
	j = StringUtility::EnumToString(type);
}

//============================================================================
// JSON -> PrimitiveType
//============================================================================
void from_json(const nlohmann::json& j, PrimitiveType& type) {

	std::string str = j.get<std::string>();
	type = StringUtility::StringToEnum<PrimitiveType>(str);
}