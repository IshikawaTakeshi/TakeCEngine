#include "Collider.h"

void Collider::Update() {
	collisionObject_->Update();
}

void Collider::DrawCollisionObj() {

	collisionObject_->Draw();
}
