#include "BehaviorTreeUtil.h"

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
    j = nlohmann::json{
        {"setName", data.setName},
        {"combos", data.combos},
        {"rootType", data.rootType},
        {"editorNodes", data.editorNodes},
        {"editorLinks", data.editorLinks}
    };
}