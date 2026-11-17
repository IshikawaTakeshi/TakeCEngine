#pragma once

#include "engine/Level/LevelDocument.h"
#include "engine/Level/LevelInstance.h"

#include <filesystem>
#include <functional>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace TakeC::Level {

/// <summary>LevelDocumentを正本として編集履歴・選択・プレビュー同期を管理します。</summary>
class LevelEditorSession final {
public:
	LevelEditorSession(GameObjectManager& previewWorld, const PrefabFactory& factory);

	const LevelDocument& GetLevelDocument() const { return document_; }
	const LevelInstance& GetPreview() const { return preview_; }
	const std::filesystem::path& GetFilePath() const { return filePath_; }
	const std::string& GetSelectedId() const { return selectedId_; }
	const std::string& GetLastError() const { return lastError_; }
	const std::string& GetPreviewError() const { return previewError_; }
	std::uint64_t GetRevision() const { return revision_; }
	bool IsDirty() const { return dirty_; }
	bool CanUndo() const { return !undo_.empty(); }
	bool CanRedo() const { return !redo_.empty(); }

	const GameObjectDefinition* GetSelectedObject() const;
	GameObjectId GetSelectedRuntimeId() const;
	bool Select(std::string_view id);

	void NewLevelDocument();
	bool SetLevelDocument(LevelDocument document);
	bool Open(const std::filesystem::path& path);
	bool Save(const std::filesystem::path& path);
	bool Save();

	/// <summary>変更を構造検証してから履歴へ記録します。生成失敗は編集を取り消しません。</summary>
	bool Apply(std::function<void(LevelDocument&)> edit);
	bool ApplyToSelected(std::function<void(GameObjectDefinition&)> edit);
	std::string AddObject();
	bool DuplicateSelected();
	bool DeleteSelected();
	bool Undo();
	bool Redo();

private:
	static constexpr std::size_t kHistoryLimit = 100;
	std::string NextId();
	void RebuildPreview();
	void PushHistory(std::vector<LevelDocument>& stack, const LevelDocument& snapshot);
	bool HasObject(std::string_view id) const;

	LevelDocument document_;
	LevelInstance preview_;
	std::filesystem::path filePath_;
	std::string selectedId_;
	std::string lastError_;
	std::string previewError_;
	std::vector<LevelDocument> undo_;
	std::vector<LevelDocument> redo_;
	std::uint64_t nextGeneratedId_ = 1;
	std::uint64_t revision_ = 0;
	bool dirty_ = false;
};

} // namespace TakeC::Level
