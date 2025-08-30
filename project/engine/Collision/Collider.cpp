#include "Collider.h"

// コライダーの色を取得
Vector4 Collider::GetColor() const {
	return color_;
}

// コライダーの半径を取得
float Collider::GetRadius() const {
	return radius_;
}

//種別IDの取得
CollisionLayer Collider::GetCollisionLayerID() {
	return layerID_;
}

//surfaceTypeの取得
SurfaceType Collider::GetSurfaceType() const {
	return surfaceType_;
}
