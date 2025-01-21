#include "CollisionManager.h"
#include "Collider.h"
#include "Vector3Math.h"

CollisionManager* CollisionManager::instance_ = nullptr;

CollisionManager* CollisionManager::GetInstance() {
	if (instance_ == nullptr) {
		instance_ = new CollisionManager();
	}
	return instance_;
}

void CollisionManager::Finalize() {
	delete instance_;
	instance_ = nullptr;
}

void CollisionManager::RegisterCollider(Collider* collider) {

	colliders_.push_back(collider);
}

void CollisionManager::ClearCollider() {

	colliders_.clear();
}

void CollisionManager::CheckAllCollisions() {

	//リスト内のペアを総当たり
	std::list<Collider*>::iterator itrA = colliders_.begin();
	for (; itrA != colliders_.end(); ++itrA) {

		//イテレータAからコライダーAを取得する
		Collider* colliderA = *itrA;

		//イテレータA+1からコライダーBを取得する(重複判定の回避)
		std::list<Collider*>::iterator itrB = itrA;
		itrB++;

		for (; itrB != colliders_.end(); ++itrB) {

			//イテレータBからコライダーBを取得する
			Collider* colliderB = *itrB;

			//コライダーAとBの衝突判定
			CheckCollisionPair(colliderA, colliderB);
		}

	}

}

void CollisionManager::CheckCollisionPair(Collider* colliderA, Collider* colliderB) {

	//コライダーAとBの座標
	Vector3 posA, posB;

	//コライダーAとBの座標の取得
	posA = colliderA->GetWorldPos();
	posB = colliderB->GetWorldPos();

	//コライダーAとBの衝突判定
	if (Vector3Math::Length(posA - posB) <= colliderA->GetRadius() + colliderB->GetRadius()) {
		//コライダーAの衝突処理
		colliderA->OnCollision(colliderB);
		//コライダーBの衝突処理
		colliderB->OnCollision(colliderA);
	}
}
