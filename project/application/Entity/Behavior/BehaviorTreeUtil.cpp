#include "BehaviorTreeUtil.h"


//============================================================================
// ComboSetData の JSON 変換関数
//============================================================================

void from_json(const nlohmann::json& j, ComboSetData& data) {

	// 必須項目 (存在しない場合は例外)
	j.at("setName").get_to(data.setName);
	j.at("combos").get_to(data.combos);

	// 任意項目（なければデフォルト値を維持）
	if (j.contains("rootType") && !j["rootType"].is_null()) {
		j.at("rootType").get_to(data.rootType);
	}

	if (j.contains("editorNodes") && !j["editorNodes"].is_null()) {
		j.at("editorNodes").get_to(data.editorNodes);
	} else {
		data.editorNodes.clear();
	}

	if (j.contains("editorLinks") && !j["editorLinks"].is_null()) {
		j.at("editorLinks").get_to(data.editorLinks);
	} else {
		data.editorLinks.clear();
	}
}

void to_json(nlohmann::json& j, const ComboSetData& data) {
	j = nlohmann::json::object();
	j["setName"] = data.setName;

	j["combos"] = nlohmann::json::array();
	for (const auto& combo : data.combos) {
		nlohmann::json combo_json;
		to_json(combo_json, combo); // 明示的に変換関数を呼ぶ
		j["combos"].push_back(combo_json);
	}

	j["rootType"] = data.rootType;

	j["editorNodes"] = nlohmann::json::array();
	for (const auto& node : data.editorNodes) {
		nlohmann::json node_json;
		to_json(node_json, node); // 明示的に変換関数を呼ぶ
		j["editorNodes"].push_back(node_json);
	}

	j["editorLinks"] = nlohmann::json::array();
	for (const auto& link : data.editorLinks) {
		nlohmann::json link_json;
		to_json(link_json, link); // 明示的に変換関数を呼ぶ
		j["editorLinks"].push_back(link_json);
	}
}

//============================================================================
// BehaviorNodeData の JSON 変換関数
//============================================================================

void from_json(const nlohmann::json& j, BehaviorNodeData& data) {

	data.name = j.value("name", "UnnamedNode");
	data.nodeType = j.value("nodeType", "ACTION");
	data.targetState = j.value("targetState", "");

	data.field = j.value("field", "");
	data.op = j.value("op", ">=");
	data.conditionThreshold = j.value("conditionThreshold", 0.0f);

	data.children.clear();
	if (j.contains("children") && j["children"].is_array()) {
		for (const auto& child_json : j["children"]) {
			BehaviorNodeData childData;
			from_json(child_json, childData); // 明示的に変換関数を呼ぶ
			data.children.push_back(childData);
		}
	}

	data.posX = j.value("posX", 0.0f);
	data.posY = j.value("posY", 0.0f);

	data.nodeUID = j.value("nodeUID", -1);
	data.bbKey = j.value("bbKey", "");
	data.bbValue = j.value("bbValue", false);
	data.bbStringValue = j.value("bbStringValue", "");
}

void to_json(nlohmann::json& j, const BehaviorNodeData& data) {

	j["name"] = data.name;
	j["nodeType"] = data.nodeType;
	j["targetState"] = data.targetState;

	j["field"] = data.field;
	j["op"] = data.op;
	j["conditionThreshold"] = data.conditionThreshold;

	j["children"] = nlohmann::json::array();
	for (const auto& child : data.children) {
		nlohmann::json child_json;
		to_json(child_json, child); // 明示的に変換関数を呼ぶ
		j["children"].push_back(child_json);
	}
	j["posX"] = data.posX;
	j["posY"] = data.posY;

	j["nodeUID"] = data.nodeUID;
	j["bbKey"] = data.bbKey;
	j["bbValue"] = data.bbValue;
	j["bbStringValue"] = data.bbStringValue;
}