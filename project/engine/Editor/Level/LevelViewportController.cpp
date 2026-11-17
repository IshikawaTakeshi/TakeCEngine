#include "engine/Editor/Level/LevelViewportController.h"

#include "engine/Collision/Collider.h"
#include "engine/GameObject/CollisionComponent.h"
#include "engine/GameObject/GameObject.h"
#include "engine/GameObject/PrefabFactory.h"
#include "engine/GameObject/TransformComponent.h"
#include "engine/Math/MatrixMath.h"
#include "engine/Math/Quaternion.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace TakeC::Level {

Ray LevelViewportController::MakeRay(float mouseX, float mouseY, float left, float top,
	float width, float height, const Matrix4x4& viewProjection) {
	if (width <= 0.0f || height <= 0.0f) {
		return {};
	}
	const float x = (mouseX - left) / width * 2.0f - 1.0f;
	const float y = 1.0f - (mouseY - top) / height * 2.0f;
	const Matrix4x4 inverse = MatrixMath::Inverse(viewProjection);
	const Vector3 nearPoint = MatrixMath::Transform({ x, y, 0.0f }, inverse);
	const Vector3 farPoint = MatrixMath::Transform({ x, y, 1.0f }, inverse);
	return { nearPoint, (farPoint - nearPoint).Normalize(), 10000.0f };
}

void LevelViewportController::Select(const Ray& ray) {
	CancelDrag();
	float nearest = std::numeric_limits<float>::max();
	std::string selected;
	for (const GameObjectDefinition& definition : session_.GetLevelDocument().objects) {
		GameObject* object = world_.FindObject(session_.GetPreview().FindObjectId(definition.id));
		if (!object || !object->IsActive()) {
			continue;
		}
		float distance = std::numeric_limits<float>::max();
		if (auto* collision = object->GetComponent<CollisionComponent>(); collision && collision->GetCollider()) {
			collision->SynchronizeTransform();
			RayCastHit hit{};
			if (collision->GetCollider()->Intersects(ray, hit)) {
				distance = hit.distance;
			}
		} else {
			const TransformComponent& transform = object->GetTransform();
			const Vector3 center = transform.GetWorldPosition();
			const Vector3 scale = transform.GetLocalScale();
			const float radius = std::max({ std::abs(scale.x), std::abs(scale.y), std::abs(scale.z), 0.5f });
			const Vector3 offset = ray.origin - center;
			const float projection = offset.Dot(ray.direction);
			const float discriminant = projection * projection - (offset.Dot(offset) - radius * radius);
			if (discriminant >= 0.0f) {
				const float nearDistance = -projection - std::sqrt(discriminant);
				const float farDistance = -projection + std::sqrt(discriminant);
				distance = nearDistance >= 0.0f ? nearDistance : farDistance;
			}
		}
		if (distance >= 0.0f && distance <= ray.distance && distance < nearest) {
			nearest = distance;
			selected = definition.id;
		}
	}
	session_.Select(selected);
}

bool LevelViewportController::BeginDrag() {
	CancelDrag();
	const GameObjectDefinition* object = session_.GetSelectedObject();
	if (!object || !world_.FindObject(session_.GetSelectedRuntimeId())) {
		return false;
	}
	const auto& recipe = object->gameObject;
	const auto read = [&](const char* key, Vector3 fallback) {
		if (!recipe.contains("transform") || !recipe.at("transform").contains(key)) {
			return fallback;
		}
		return PrefabFactory::Vector(recipe.at("transform").at(key));
	};
	initialPosition_ = position_ = read("position", {});
	initialRotation_ = rotation_ = read("rotation", {});
	initialScale_ = scale_ = read("scale", { 1.0f, 1.0f, 1.0f });
	draggedId_ = object->id;
	draggedRevision_ = session_.GetRevision();
	changed_ = false;
	dragging_ = true;
	return true;
}

void LevelViewportController::Drag(float deltaX, float deltaY, const Matrix4x4& inverseView,
	const Vector3& cameraPosition) {
	if (!dragging_) { return; }
	GameObject* object = world_.FindObject(session_.GetSelectedRuntimeId());
	if (!object || session_.GetSelectedId() != draggedId_ || session_.GetRevision() != draggedRevision_) {
		CancelDrag();
		return;
	}
	if (deltaX == 0.0f && deltaY == 0.0f) { return; }
	if (mode_ == Mode::Translate) {
		const Vector3 right{ inverseView.m[0][0], inverseView.m[0][1], inverseView.m[0][2] };
		const Vector3 up{ inverseView.m[1][0], inverseView.m[1][1], inverseView.m[1][2] };
		const float distance = (object->GetTransform().GetWorldPosition() - cameraPosition).Length();
		const float speed = std::max(distance * 0.0015f, 0.01f);
		Vector3 displacement = right * (deltaX * speed) + up * (-deltaY * speed);
		if (const auto* parent = object->GetTransform().GetParent()) {
			const Matrix4x4 inverseParent = MatrixMath::Inverse(parent->GetWorldMatrix());
			displacement = MatrixMath::Transform(displacement, inverseParent) -
				MatrixMath::Transform({}, inverseParent);
		}
		position_ += displacement;
	} else if (mode_ == Mode::Rotate) {
		rotation_.x -= deltaY * 0.01f;
		rotation_.y += deltaX * 0.01f;
	} else {
		const float amount = (deltaX - deltaY) * 0.01f;
		scale_.x = std::max(scale_.x + amount, 0.001f);
		scale_.y = std::max(scale_.y + amount, 0.001f);
		scale_.z = std::max(scale_.z + amount, 0.001f);
	}
	changed_ = true;
	SynchronizePreview();
}

void LevelViewportController::SynchronizePreview() {
	GameObject* object = world_.FindObject(session_.GetPreview().FindObjectId(draggedId_));
	if (!object) { return; }
	auto& transform = object->GetTransform();
	transform.SetLocalPosition(position_);
	transform.SetLocalRotation(QuaternionMath::FromEuler(rotation_));
	transform.SetLocalScale(scale_);
}

void LevelViewportController::EndDrag() {
	if (!dragging_) { return; }
	const bool valid = session_.GetSelectedId() == draggedId_ &&
		session_.GetRevision() == draggedRevision_;
	if (valid && changed_) {
		const Vector3 position = position_, rotation = rotation_, scale = scale_;
		if (!session_.ApplyToSelected([&](GameObjectDefinition& object) {
			auto& transform = object.gameObject["transform"];
			transform["position"] = { position.x, position.y, position.z };
			transform["rotation"] = { rotation.x, rotation.y, rotation.z };
			transform["scale"] = { scale.x, scale.y, scale.z };
		})) {
			position_ = initialPosition_; rotation_ = initialRotation_; scale_ = initialScale_;
			SynchronizePreview();
		}
	} else if (session_.GetRevision() == draggedRevision_) {
		position_ = initialPosition_; rotation_ = initialRotation_; scale_ = initialScale_;
		SynchronizePreview();
	}
	dragging_ = false;
	changed_ = false;
	draggedId_.clear();
}

void LevelViewportController::CancelDrag() {
	if (dragging_ && session_.GetRevision() == draggedRevision_) {
		position_ = initialPosition_; rotation_ = initialRotation_; scale_ = initialScale_;
		SynchronizePreview();
	}
	dragging_ = false;
	changed_ = false;
	draggedId_.clear();
}

} // namespace TakeC::Level
