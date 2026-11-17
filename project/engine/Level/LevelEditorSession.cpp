#include "engine/Level/LevelEditorSession.h"

#include "engine/Level/LevelSerializer.h"

#include <algorithm>
#include <exception>
#include <ranges>
#include <unordered_set>
#include <utility>

namespace TakeC::Level {

LevelEditorSession::LevelEditorSession(GameObjectManager& previewWorld, const PrefabFactory& factory)
	: preview_(previewWorld, factory) {}

const GameObjectDefinition* LevelEditorSession::GetSelectedObject() const {
	const auto it = std::ranges::find(document_.objects, selectedId_, &GameObjectDefinition::id);
	return it == document_.objects.end() ? nullptr : &*it;
}

GameObjectId LevelEditorSession::GetSelectedRuntimeId() const {
	return preview_.FindObjectId(selectedId_);
}

bool LevelEditorSession::HasObject(std::string_view id) const {
	return std::ranges::find(document_.objects, id, &GameObjectDefinition::id) != document_.objects.end();
}

bool LevelEditorSession::Select(std::string_view id) {
	if (!id.empty() && !HasObject(id)) {
		return false;
	}
	selectedId_ = id;
	return true;
}

void LevelEditorSession::NewLevelDocument() {
	document_ = {};
	filePath_.clear();
	selectedId_.clear();
	lastError_.clear();
	previewError_.clear();
	undo_.clear();
	redo_.clear();
	dirty_ = true;
	nextGeneratedId_ = 1;
	++revision_;
	preview_.Clear();
}

bool LevelEditorSession::SetLevelDocument(LevelDocument document) {
	const auto errors = ValidateLevelDocument(document);
	if (!errors.empty()) {
		lastError_ = errors.front().message;
		return false;
	}
	document_ = std::move(document);
	selectedId_.clear();
	filePath_.clear();
	undo_.clear();
	redo_.clear();
	dirty_ = false;
	nextGeneratedId_ = 1;
	++revision_;
	lastError_.clear();
	RebuildPreview();
	return true;
}

bool LevelEditorSession::Open(const std::filesystem::path& path) {
	try {
		LevelDocument loaded = LoadLevelDocument(path);
		if (!SetLevelDocument(std::move(loaded))) {
			return false;
		}
		filePath_ = path;
		return true;
	} catch (const std::exception& error) {
		lastError_ = error.what();
		return false;
	}
}

bool LevelEditorSession::Save(const std::filesystem::path& path) {
	try {
		SaveLevelDocument(path, document_);
		filePath_ = path;
		dirty_ = false;
		lastError_.clear();
		return true;
	} catch (const std::exception& error) {
		lastError_ = error.what();
		return false;
	}
}

bool LevelEditorSession::Save() {
	return Save(filePath_);
}

void LevelEditorSession::PushHistory(
	std::vector<LevelDocument>& stack, const LevelDocument& snapshot) {
	if (stack.size() >= kHistoryLimit) {
		stack.erase(stack.begin());
	}
	stack.push_back(snapshot);
}

bool LevelEditorSession::Apply(std::function<void(LevelDocument&)> edit) {
	if (!edit) {
		lastError_ = "Level edit callback is empty.";
		return false;
	}
	LevelDocument next = document_;
	try {
		edit(next);
	} catch (const std::exception& error) {
		lastError_ = error.what();
		return false;
	}
	const auto errors = ValidateLevelDocument(next);
	if (!errors.empty()) {
		lastError_ = errors.front().message;
		return false;
	}
	PushHistory(undo_, document_);
	redo_.clear();
	document_ = std::move(next);
	++revision_;
	if (!selectedId_.empty() && !HasObject(selectedId_)) {
		selectedId_.clear();
	}
	dirty_ = true;
	lastError_.clear();
	RebuildPreview();
	return true;
}

bool LevelEditorSession::ApplyToSelected(std::function<void(GameObjectDefinition&)> edit) {
	if (selectedId_.empty() || !HasObject(selectedId_) || !edit) {
		lastError_ = "Select a GameObject before editing.";
		return false;
	}
	const std::string id = selectedId_;
	return Apply([&](LevelDocument& document) {
		auto it = std::ranges::find(document.objects, id, &GameObjectDefinition::id);
		edit(*it);
	});
}

std::string LevelEditorSession::NextId() {
	for (;;) {
		const std::string candidate = "object-" + std::to_string(nextGeneratedId_++);
		if (!HasObject(candidate)) {
			return candidate;
		}
	}
}

std::string LevelEditorSession::AddObject() {
	const std::string id = NextId();
	if (!Apply([&](LevelDocument& document) {
		GameObjectDefinition object;
		object.id = id;
		object.gameObject["name"] = "GameObject";
		document.objects.push_back(std::move(object));
	})) {
		return {};
	}
	selectedId_ = id;
	return id;
}

bool LevelEditorSession::DuplicateSelected() {
	const GameObjectDefinition* selected = GetSelectedObject();
	if (!selected) {
		return false;
	}
	GameObjectDefinition copy = *selected;
	copy.id = NextId();
	copy.gameObject["name"] = copy.gameObject.value("name", std::string("GameObject")) + " Copy";
	const std::string id = copy.id;
	if (!Apply([&](LevelDocument& document) { document.objects.push_back(copy); })) {
		return false;
	}
	selectedId_ = id;
	return true;
}

bool LevelEditorSession::DeleteSelected() {
	if (selectedId_.empty() || !HasObject(selectedId_)) {
		return false;
	}
	const std::string rootId = selectedId_;
	return Apply([&](LevelDocument& document) {
		std::unordered_set<std::string> removed{ rootId };
		bool changed = true;
		while (changed) {
			changed = false;
			for (const GameObjectDefinition& object : document.objects) {
				if (object.parentId && removed.contains(*object.parentId)) {
					changed |= removed.insert(object.id).second;
				}
			}
		}
		std::erase_if(document.objects, [&](const GameObjectDefinition& object) {
			return removed.contains(object.id);
		});
	});
}

bool LevelEditorSession::Undo() {
	if (undo_.empty()) {
		return false;
	}
	PushHistory(redo_, document_);
	document_ = std::move(undo_.back());
	undo_.pop_back();
	++revision_;
	if (!selectedId_.empty() && !HasObject(selectedId_)) {
		selectedId_.clear();
	}
	dirty_ = true;
	lastError_.clear();
	RebuildPreview();
	return true;
}

bool LevelEditorSession::Redo() {
	if (redo_.empty()) {
		return false;
	}
	PushHistory(undo_, document_);
	document_ = std::move(redo_.back());
	redo_.pop_back();
	++revision_;
	if (!selectedId_.empty() && !HasObject(selectedId_)) {
		selectedId_.clear();
	}
	dirty_ = true;
	lastError_.clear();
	RebuildPreview();
	return true;
}

void LevelEditorSession::RebuildPreview() {
	try {
		preview_.Load(document_);
		previewError_.clear();
	} catch (const std::exception& error) {
		preview_.Clear();
		previewError_ = error.what();
	}
}

} // namespace TakeC::Level
