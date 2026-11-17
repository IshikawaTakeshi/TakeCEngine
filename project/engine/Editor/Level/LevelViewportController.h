#pragma once

#include "engine/Level/LevelEditorSession.h"
#include "engine/Math/Matrix4x4.h"
#include "engine/Math/Vector3.h"
#include "engine/math/physics/Ray.h"

#include <cstdint>
#include <string>

namespace TakeC::Level {

/// <summary>LevelDocumentのプレビューに対するレイ選択と、1操作1履歴のTransform編集を管理します。</summary>
class LevelViewportController final {
public:
	enum class Mode { Translate, Rotate, Scale };

	LevelViewportController(LevelEditorSession& session, GameObjectManager& world)
		: session_(session), world_(world) {}

	static Ray MakeRay(float mouseX, float mouseY, float left, float top,
		float width, float height, const Matrix4x4& viewProjection);
	void Select(const Ray& ray);
	void SetMode(Mode mode) { mode_ = mode; }
	Mode GetMode() const { return mode_; }
	bool BeginDrag();
	void Drag(float deltaX, float deltaY, const Matrix4x4& inverseView,
		const Vector3& cameraPosition);
	void EndDrag();
	void CancelDrag();
	bool IsDragging() const { return dragging_; }

private:
	LevelEditorSession& session_;
	GameObjectManager& world_;
	Mode mode_ = Mode::Translate;
	std::string draggedId_;
	std::uint64_t draggedRevision_ = 0;
	Vector3 initialPosition_{};
	Vector3 initialRotation_{};
	Vector3 initialScale_{ 1.0f, 1.0f, 1.0f };
	Vector3 position_{};
	Vector3 rotation_{};
	Vector3 scale_{ 1.0f, 1.0f, 1.0f };
	bool dragging_ = false;
	bool changed_ = false;
	void SynchronizePreview();
};

} // namespace TakeC::Level
