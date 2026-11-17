#pragma once

#include "engine/Level/LevelEditorSession.h"

#include <array>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

struct ImGuiInputTextCallbackData;

namespace TakeC::Level {

/// <summary>LevelDocumentをHierarchy・Inspector・保存画面で編集する汎用ImGuiパネルです。</summary>
class LevelEditorPanel final {
public:
	explicit LevelEditorPanel(LevelEditorSession& session);
	/// <summary>呼び出し元のImGuiフレーム内で各ウィンドウを描画します。</summary>
	void Draw();

private:
	enum class PendingAction { None, New, Open };
	static int ResizeTextBuffer(ImGuiInputTextCallbackData* data);
	static void SetText(std::vector<char>& buffer, std::string_view text);
	static std::string GetText(const std::vector<char>& buffer);
	static bool InputText(const char* label, std::vector<char>& buffer, bool multiline = false);
	void SyncDraft();
	void RequestAction(PendingAction action);
	void ExecuteAction(PendingAction action);
	void DrawToolbar();
	void DrawHierarchy();
	void DrawHierarchyObject(const GameObjectDefinition& object);
	void DrawInspector();
	void DrawDiagnostics();

	LevelEditorSession& session_;
	std::string draftId_;
	std::uint64_t draftRevision_ = 0;
	std::string synchronizedLevelName_;
	std::string status_;
	std::vector<char> pathBuffer_;
	std::vector<char> levelNameBuffer_;
	std::vector<char> nameBuffer_;
	std::vector<char> componentBuffer_;
	std::array<char, 128> newComponentType_{};
	std::array<float, 3> position_{};
	std::array<float, 3> rotation_{};
	std::array<float, 3> scale_{ 1.0f, 1.0f, 1.0f };
	PendingAction pendingAction_ = PendingAction::None;
};

} // namespace TakeC::Level
