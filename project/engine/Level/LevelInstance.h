#pragma once

#include "engine/GameObject/GameObjectManager.h"
#include "engine/Level/LevelDocument.h"

#include <string_view>
#include <utility>
#include <vector>

namespace TakeC {
class PrefabFactory;
}

namespace TakeC::Level {

/// <summary>レベル由来のGameObjectだけを管理し、永続IDと実行時IDを対応付けます。</summary>
/// <remarks>GameObjectManagerとPrefabFactoryはこのインスタンスより長く生存させてください。</remarks>
class LevelInstance final {
public:
	LevelInstance(GameObjectManager& world, const PrefabFactory& factory)
		: world_(world), factory_(factory) {}
	~LevelInstance();

	LevelInstance(const LevelInstance&) = delete;
	LevelInstance& operator=(const LevelInstance&) = delete;
	LevelInstance(LevelInstance&&) = delete;
	LevelInstance& operator=(LevelInstance&&) = delete;

	/// <summary>構築成功時に旧レベルを置換し、失敗時には旧レベルを保持します。</summary>
	void Load(const LevelDocument& document);
	/// <summary>このレベルから生成したGameObjectだけを破棄します。</summary>
	void Clear();
	GameObjectId FindObjectId(std::string_view persistentId) const;
	std::size_t GetObjectCount() const { return objects_.size(); }

private:
	GameObjectManager& world_;
	const PrefabFactory& factory_;
	std::vector<std::pair<std::string, GameObjectId>> objects_;
};

} // namespace TakeC::Level
