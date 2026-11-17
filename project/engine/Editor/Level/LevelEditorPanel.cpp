#include "engine/Editor/Level/LevelEditorPanel.h"

#include <imgui.h>

#include <algorithm>
#include <filesystem>
#include <string>
#include <utility>

namespace TakeC::Level {

LevelEditorPanel::LevelEditorPanel(LevelEditorSession& session) : session_(session) {
	SetText(pathBuffer_, {});
	SetText(levelNameBuffer_, session_.GetLevelDocument().name);
	synchronizedLevelName_ = session_.GetLevelDocument().name;
	SetText(nameBuffer_, {});
	SetText(componentBuffer_, "[]");
}

int LevelEditorPanel::ResizeTextBuffer(ImGuiInputTextCallbackData* data) {
	if (data->EventFlag == ImGuiInputTextFlags_CallbackResize) {
		auto& buffer = *static_cast<std::vector<char>*>(data->UserData);
		buffer.resize(static_cast<std::size_t>(data->BufSize));
		data->Buf = buffer.data();
	}
	return 0;
}

void LevelEditorPanel::SetText(std::vector<char>& buffer, std::string_view text) {
	buffer.assign(std::max<std::size_t>(text.size() + 256, 1024), '\0');
	std::copy(text.begin(), text.end(), buffer.begin());
}

std::string LevelEditorPanel::GetText(const std::vector<char>& buffer) {
	return buffer.empty() ? std::string{} : std::string(buffer.data());
}

bool LevelEditorPanel::InputText(const char* label, std::vector<char>& buffer, bool multiline) {
	if (multiline) {
		return ImGui::InputTextMultiline(
			label, buffer.data(), buffer.size(), ImVec2(-1.0f, 180.0f),
			ImGuiInputTextFlags_CallbackResize, ResizeTextBuffer, &buffer);
	}
	return ImGui::InputText(label, buffer.data(), buffer.size(),
		ImGuiInputTextFlags_CallbackResize, ResizeTextBuffer, &buffer);
}

void LevelEditorPanel::SyncDraft() {
	const GameObjectDefinition* object = session_.GetSelectedObject();
	const std::string selectedId = object ? object->id : std::string{};
	if (selectedId == draftId_ && draftRevision_ == session_.GetRevision()) {
		return;
	}
	draftId_ = selectedId;
	draftRevision_ = session_.GetRevision();
	if (!object) {
		SetText(nameBuffer_, {});
		SetText(componentBuffer_, "[]");
		return;
	}
	const nlohmann::json& recipe = object->gameObject;
	SetText(nameBuffer_, recipe.value("name", std::string("GameObject")));
	SetText(componentBuffer_, recipe.at("components").dump(2));
	const auto readVector = [&](const char* field, std::array<float, 3> fallback) {
		if (!recipe.contains("transform") || !recipe.at("transform").contains(field)) {
			return fallback;
		}
		const auto& value = recipe.at("transform").at(field);
		return std::array<float, 3>{ value.at(0).get<float>(), value.at(1).get<float>(),
			value.at(2).get<float>() };
	};
	position_ = readVector("position", { 0.0f, 0.0f, 0.0f });
	rotation_ = readVector("rotation", { 0.0f, 0.0f, 0.0f });
	scale_ = readVector("scale", { 1.0f, 1.0f, 1.0f });
}

void LevelEditorPanel::ExecuteAction(PendingAction action) {
	if (action == PendingAction::New) {
		session_.NewLevelDocument();
		SetText(pathBuffer_, {});
		status_ = "Created a new LevelDocument.";
	} else if (action == PendingAction::Open) {
		if (session_.Open(std::filesystem::path(GetText(pathBuffer_)))) {
			status_ = "Loaded LevelDocument.";
		} else {
			status_ = session_.GetLastError();
		}
	}
	draftId_.clear();
	synchronizedLevelName_.clear();
	SyncDraft();
}

void LevelEditorPanel::RequestAction(PendingAction action) {
	if (session_.IsDirty()) {
		pendingAction_ = action;
		ImGui::OpenPopup("Discard LevelDocument changes?");
	} else {
		ExecuteAction(action);
	}
}

void LevelEditorPanel::DrawToolbar() {
	if (!ImGui::Begin("TakeC Level")) {
		ImGui::End();
		return;
	}
	if (synchronizedLevelName_ != session_.GetLevelDocument().name) {
		SetText(levelNameBuffer_, session_.GetLevelDocument().name);
		synchronizedLevelName_ = session_.GetLevelDocument().name;
	}
	InputText("Level name", levelNameBuffer_);
	if (ImGui::IsItemDeactivatedAfterEdit()) {
		const std::string name = GetText(levelNameBuffer_);
		if (session_.Apply([&](LevelDocument& document) { document.name = name; })) {
			synchronizedLevelName_ = name;
		}
	}
	InputText("Level file", pathBuffer_);
	if (ImGui::Button("New")) {
		RequestAction(PendingAction::New);
	}
	ImGui::SameLine();
	if (ImGui::Button("Open")) {
		RequestAction(PendingAction::Open);
	}
	ImGui::SameLine();
	if (ImGui::Button("Save")) {
		status_ = session_.Save(std::filesystem::path(GetText(pathBuffer_)))
			? "Saved LevelDocument." : session_.GetLastError();
	}
	ImGui::SameLine();
	if (ImGui::Button("Undo") && session_.CanUndo()) {
		session_.Undo();
		draftId_.clear();
	}
	ImGui::SameLine();
	if (ImGui::Button("Redo") && session_.CanRedo()) {
		session_.Redo();
		draftId_.clear();
	}
	ImGui::Text("%s%s", session_.GetLevelDocument().name.c_str(),
		session_.IsDirty() ? " *" : "");
	if (ImGui::BeginPopupModal("Discard LevelDocument changes?", nullptr,
		ImGuiWindowFlags_AlwaysAutoResize)) {
		ImGui::TextUnformatted("Unsaved changes will be discarded.");
		if (ImGui::Button("Discard")) {
			ExecuteAction(pendingAction_);
			pendingAction_ = PendingAction::None;
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel")) {
			pendingAction_ = PendingAction::None;
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
	ImGui::End();
}

void LevelEditorPanel::DrawHierarchyObject(const GameObjectDefinition& object) {
	const LevelDocument& document = session_.GetLevelDocument();
	const bool hasChildren = std::ranges::any_of(document.objects, [&](const auto& candidate) {
		return candidate.parentId && *candidate.parentId == object.id;
	});
	const std::string name = object.gameObject.value("name", std::string("GameObject"));
	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
	if (!hasChildren) {
		flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
	}
	if (object.id == session_.GetSelectedId()) {
		flags |= ImGuiTreeNodeFlags_Selected;
	}
	const bool open = ImGui::TreeNodeEx(object.id.c_str(), flags, "%s", name.c_str());
	if (ImGui::IsItemClicked()) {
		session_.Select(object.id);
	}
	if (hasChildren && open) {
		for (const GameObjectDefinition& child : document.objects) {
			if (child.parentId && *child.parentId == object.id) {
				DrawHierarchyObject(child);
			}
		}
		ImGui::TreePop();
	}
}

void LevelEditorPanel::DrawHierarchy() {
	if (!ImGui::Begin("TakeC Level Hierarchy")) {
		ImGui::End();
		return;
	}
	if (ImGui::Button("Add GameObject")) {
		session_.AddObject();
	}
	ImGui::SameLine();
	if (ImGui::Button("Duplicate")) {
		session_.DuplicateSelected();
	}
	ImGui::SameLine();
	if (ImGui::Button("Delete")) {
		session_.DeleteSelected();
	}
	ImGui::Separator();
	const LevelDocument& document = session_.GetLevelDocument();
	for (const GameObjectDefinition& object : document.objects) {
		if (!object.parentId) {
			DrawHierarchyObject(object);
		}
	}
	ImGui::End();
}

void LevelEditorPanel::DrawInspector() {
	SyncDraft();
	if (!ImGui::Begin("TakeC Level Inspector")) {
		ImGui::End();
		return;
	}
	const GameObjectDefinition* selected = session_.GetSelectedObject();
	if (!selected) {
		ImGui::TextUnformatted("Select a GameObject in the Hierarchy.");
		ImGui::End();
		return;
	}
	ImGui::Text("Persistent ID: %s", selected->id.c_str());
	InputText("Name", nameBuffer_);
	if (ImGui::IsItemDeactivatedAfterEdit()) {
		const std::string value = GetText(nameBuffer_);
		session_.ApplyToSelected([&](GameObjectDefinition& object) {
			object.gameObject["name"] = value;
		});
		ImGui::End();
		return;
	}
	bool active = selected->gameObject.value("active", true);
	if (ImGui::Checkbox("Active", &active)) {
		session_.ApplyToSelected([&](GameObjectDefinition& object) {
			object.gameObject["active"] = active;
		});
		ImGui::End();
		return;
	}
	const char* parentLabel = selected->parentId ? selected->parentId->c_str() : "<none>";
	if (ImGui::BeginCombo("Parent", parentLabel)) {
		if (ImGui::Selectable("<none>", !selected->parentId)) {
			session_.ApplyToSelected([](GameObjectDefinition& object) {
				object.parentId.reset();
			});
			ImGui::EndCombo();
			ImGui::End();
			return;
		}
		for (const GameObjectDefinition& candidate : session_.GetLevelDocument().objects) {
			if (candidate.id != selected->id && ImGui::Selectable(candidate.id.c_str(),
				selected->parentId == candidate.id)) {
				const std::string id = candidate.id;
				session_.ApplyToSelected([&](GameObjectDefinition& object) {
					object.parentId = id;
				});
				ImGui::EndCombo();
				ImGui::End();
				return;
			}
		}
		ImGui::EndCombo();
	}
	ImGui::SeparatorText("Transform (local, rotation in radians)");
	const auto editVector = [&](const char* label, const char* key, std::array<float, 3>& draft) {
		ImGui::DragFloat3(label, draft.data(), 0.05f);
		if (!ImGui::IsItemDeactivatedAfterEdit()) {
			return false;
		}
		const std::array<float, 3> value = draft;
		session_.ApplyToSelected([&](GameObjectDefinition& object) {
			object.gameObject["transform"][key] = { value[0], value[1], value[2] };
		});
		return true;
	};
	if (editVector("Position", "position", position_) ||
		editVector("Rotation", "rotation", rotation_) ||
		editVector("Scale", "scale", scale_)) {
		ImGui::End();
		return;
	}
	ImGui::SeparatorText("Components (JSON array)");
	InputText("##Components", componentBuffer_, true);
	if (ImGui::Button("Apply Components")) {
		try {
			const nlohmann::json components = nlohmann::json::parse(GetText(componentBuffer_));
			if (session_.ApplyToSelected([&](GameObjectDefinition& object) {
				object.gameObject["components"] = components;
			})) {
				status_ = "Updated components.";
				draftId_.clear();
			}
		} catch (const std::exception& error) {
			status_ = error.what();
		}
		ImGui::End();
		return;
	}
	ImGui::InputText("New component type", newComponentType_.data(), newComponentType_.size());
	if (ImGui::Button("Add Component") && newComponentType_[0] != '\0') {
		const std::string type = newComponentType_.data();
		if (session_.ApplyToSelected([&](GameObjectDefinition& object) {
			object.gameObject["components"].push_back({ { "type", type } });
		})) {
			newComponentType_.fill('\0');
			draftId_.clear();
		}
		ImGui::End();
		return;
	}
	ImGui::End();
}

void LevelEditorPanel::DrawDiagnostics() {
	if (!ImGui::Begin("TakeC Level Diagnostics")) {
		ImGui::End();
		return;
	}
	if (!status_.empty()) {
		ImGui::TextWrapped("%s", status_.c_str());
	}
	if (!session_.GetLastError().empty()) {
		ImGui::TextWrapped("Edit error: %s", session_.GetLastError().c_str());
	}
	if (!session_.GetPreviewError().empty()) {
		ImGui::TextWrapped("Preview error: %s", session_.GetPreviewError().c_str());
	}
	ImGui::Text("Preview GameObjects: %zu", session_.GetPreview().GetObjectCount());
	ImGui::End();
}

void LevelEditorPanel::Draw() {
	DrawToolbar();
	DrawHierarchy();
	DrawInspector();
	DrawDiagnostics();
}

} // namespace TakeC::Level
