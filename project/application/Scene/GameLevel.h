#pragma once

#include "application/Entity/LevelObject/LevelObject.h"
#include "application/Scene/LevelData.h"

#include <map>
#include <memory>
#include <string>

/// ゲーム固有のレベルJSONと、そこから生成した配置オブジェクトを所有します。
/// SceneManagerから分離することで、エンジン側はゲームのレベル形式に依存しません。
class GameLevel {
public:
	bool Load(const std::string& levelName);
	void Clear();

	const std::map<std::string, std::unique_ptr<LevelObject>>& GetObjects() const { return objects_; }
	std::map<std::string, std::unique_ptr<LevelObject>>& GetObjects() { return objects_; }

private:
	static std::unique_ptr<LevelData> LoadLevelData(const std::string& levelName);

	std::unique_ptr<LevelData> levelData_;
	std::map<std::string, std::unique_ptr<LevelObject>> objects_;
};
