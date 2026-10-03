#include "CollisionManager.h"
#include "Collider.h"
#include "engine/Math/Vector3Math.h"
#include "engine/base/DirectXCommon.h"
#include "engine/GameObject/CollisionComponent.h"
#include "engine/GameObject/GameObject.h"
#include "engine/Collision/SphereCollider.h"

#include <algorithm>
#include <cmath>
#include <optional>

namespace {

std::optional<TakeC::CollisionContactInfo> MakeContact(Collider* first, Collider* second) {
	auto* sphereA = dynamic_cast<SphereCollider*>(first);
	auto* sphereB = dynamic_cast<SphereCollider*>(second);
	if (!sphereA || !sphereB) {
		return std::nullopt; // 現行の箱判定は接触点を公開していない
	}
	const Vector3 difference = sphereB->GetWorldPos() - sphereA->GetWorldPos();
	const float distance = difference.Length();
	const Vector3 normal = distance > 0.00001f
		? difference / distance : Vector3{ 1.0f, 0.0f, 0.0f };
	return TakeC::CollisionContactInfo{
		sphereA->GetWorldPos() + normal * sphereA->GetRadius(),
		normal,
		std::max(0.0f, sphereA->GetRadius() + sphereB->GetRadius() - distance),
	};
}

} // namespace

CollisionManager& CollisionManager::GetInstance() {
	static CollisionManager instance;
	return instance;
}

//=============================================================================
// 初期化
//=============================================================================

void CollisionManager::Initialize(TakeC::DirectXCommon* dxCommon) {

	dxCommon_ = dxCommon;

	pso_ = std::make_unique<TakeC::PSO>();
	pso_->CompileVertexShader(dxCommon_->GetDXC(), L"3d/SkyBox.VS.hlsl");
	pso_->CompilePixelShader(dxCommon_->GetDXC(), L"3d/SkyBox.PS.hlsl");
	pso_->CreateGraphicPSO(dxCommon_->GetDevice(), D3D12_FILL_MODE_WIREFRAME, D3D12_DEPTH_WRITE_MASK_ZERO);
	pso_->SetGraphicPipelineName("CollisionPSO");
	rootSignature_ = pso_->GetGraphicRootSignature();
}

//=============================================================================
// 描画前処理
//=============================================================================

void CollisionManager::PreDraw() {
	//プリミティブトポロジー設定
	dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//ルートシグネチャ設定
	dxCommon_->GetCommandList()->SetGraphicsRootSignature(rootSignature_.Get());
	//PSO設定
	dxCommon_->GetCommandList()->SetPipelineState(pso_->GetGraphicPipelineState());
}

//=============================================================================
// シングルトンインスタンスの解放処理
//=============================================================================

void CollisionManager::Finalize() {
	collisionComponents_.clear();
	activeCollisionComponents_.clear();
	gameCharacters_.clear();
	activeGameCharacters_.clear();
	legacyColliderIds_.clear();
	registeredEntries_.clear();
	previousPairs_.clear();
	eventQueue_.clear();
	pso_.reset();
	rootSignature_.Reset();
}

//=============================================================================
// ゲームキャラクターの登録・解放・衝突判定
//=============================================================================
void CollisionManager::RegisterGameCharacter(GameCharacter* gameCharacter) {
	if (gameCharacter != nullptr && gameCharacter->GetCollider() != nullptr &&
		activeGameCharacters_.insert(gameCharacter).second) {
		gameCharacters_.push_back(gameCharacter);
		const TakeC::ColliderId id = TakeC::AllocateColliderId();
		legacyColliderIds_[gameCharacter] = id;
		registeredEntries_[id] = { id, TakeC::kInvalidGameObjectId, nullptr, gameCharacter };
	}
}

void CollisionManager::UnregisterGameCharacter(GameCharacter* gameCharacter) {
	if (const auto iterator = legacyColliderIds_.find(gameCharacter);
		iterator != legacyColliderIds_.end()) {
		registeredEntries_.erase(iterator->second);
		legacyColliderIds_.erase(iterator);
	}
	activeGameCharacters_.erase(gameCharacter);
	std::erase(gameCharacters_, gameCharacter);
}

void CollisionManager::RegisterCollisionComponent(TakeC::CollisionComponent* component) {
	if (component != nullptr && component->GetCollider() != nullptr &&
		activeCollisionComponents_.insert(component).second) {
		collisionComponents_.push_back(component);
		const TakeC::ColliderId id = component->GetColliderId();
		registeredEntries_[id] = { id, component->GetOwner().GetId(), component, nullptr };
	}
}

void CollisionManager::UnregisterCollisionComponent(TakeC::CollisionComponent* component) {
	if (component != nullptr) {
		registeredEntries_.erase(component->GetColliderId());
	}
	activeCollisionComponents_.erase(component);
	std::erase(collisionComponents_, component);
}

//=============================================================================
// ゲームキャラクターの全解放
//=============================================================================
void CollisionManager::ClearGameCharacter() {
	for (const auto& [character, id] : legacyColliderIds_) {
		registeredEntries_.erase(id);
	}
	legacyColliderIds_.clear();
	gameCharacters_.clear();
	activeGameCharacters_.clear();
}

//=============================================================================
// ゲームキャラクター同士の全衝突判定
//=============================================================================
void CollisionManager::CheckAllCollisionsForGameCharacter() {
	// 旧 API も同じ収集・判定経路へ統合する。
	CheckAllCollisions();
}

std::vector<CollisionManager::CollisionEntry> CollisionManager::CollectEntries() const {
	std::vector<CollisionEntry> entries;
	entries.reserve(collisionComponents_.size() + gameCharacters_.size());
	std::unordered_set<Collider*> componentColliders;
	for (TakeC::CollisionComponent* component : collisionComponents_) {
		const auto iterator = registeredEntries_.find(component->GetColliderId());
		if (iterator != registeredEntries_.end()) {
			entries.push_back(iterator->second);
			componentColliders.insert(component->GetCollider());
		}
	}
	for (GameCharacter* character : gameCharacters_) {
		if (character == nullptr || character->GetCollider() == nullptr) {
			continue;
		}
		if (!componentColliders.contains(character->GetCollider())) {
			const auto id = legacyColliderIds_.find(character);
			if (id != legacyColliderIds_.end()) {
				entries.push_back(registeredEntries_.at(id->second));
			}
		}
	}
	return entries;
}

bool CollisionManager::IsEntryActive(const CollisionEntry& entry) const {
	const auto registered = registeredEntries_.find(entry.colliderId);
	if (registered == registeredEntries_.end() ||
		registered->second.component != entry.component ||
		registered->second.character != entry.character ||
		registered->second.objectId != entry.objectId) {
		return false;
	}
	if (entry.component != nullptr) {
		return activeCollisionComponents_.contains(entry.component) &&
			entry.component->GetColliderId() == entry.colliderId &&
			entry.component->GetOwner().GetId() == entry.objectId &&
			entry.component->GetCollider() != nullptr && entry.component->IsEnabled() &&
			entry.component->GetOwner().IsActive() &&
			!entry.component->GetOwner().IsDestroyRequested();
	}
	return entry.character != nullptr &&
		activeGameCharacters_.contains(entry.character) &&
		entry.character->GetCollider() != nullptr;
}

const CollisionManager::CollisionEntry* CollisionManager::ResolveEntry(
	TakeC::ColliderId id, TakeC::GameObjectId objectId) const {
	const auto iterator = registeredEntries_.find(id);
	if (iterator == registeredEntries_.end() || iterator->second.objectId != objectId ||
		!IsEntryActive(iterator->second)) {
		return nullptr;
	}
	return &iterator->second;
}

Collider* CollisionManager::GetEntryCollider(const CollisionEntry& entry) const {
	return entry.component != nullptr
		? entry.component->GetCollider()
		: entry.character->GetCollider();
}

GameCharacter* CollisionManager::GetEntryCharacter(const CollisionEntry& entry) const {
	return entry.component != nullptr
		? entry.component->GetLegacyOwner()
		: entry.character;
}

std::uint32_t CollisionManager::GetEntryMask(const CollisionEntry& entry) const {
	return entry.component != nullptr
		? entry.component->GetCollisionMask()
		: 0xffffffffu;
}

bool CollisionManager::TestPair(const CollisionEntry& first, const CollisionEntry& second,
	bool& isTrigger, std::optional<TakeC::CollisionContactInfo>& contact) const {
	if (!IsEntryActive(first) || !IsEntryActive(second) ||
		(first.component && second.component &&
			&first.component->GetOwner() == &second.component->GetOwner())) {
		return false;
	}
	GameCharacter* firstCharacter = GetEntryCharacter(first);
	GameCharacter* secondCharacter = GetEntryCharacter(second);
	if (firstCharacter && secondCharacter &&
		(firstCharacter == secondCharacter ||
			firstCharacter->GetCharacterType() == secondCharacter->GetCharacterType())) {
		return false;
	}
	Collider* colliderA = GetEntryCollider(first);
	Collider* colliderB = GetEntryCollider(second);
	const std::uint32_t layerA = first.component
		? first.component->GetCollisionLayer()
		: static_cast<std::uint32_t>(colliderA->GetCollisionLayerID());
	const std::uint32_t layerB = second.component
		? second.component->GetCollisionLayer()
		: static_cast<std::uint32_t>(colliderB->GetCollisionLayerID());
	const std::uint32_t maskA = GetEntryMask(first);
	const std::uint32_t maskB = GetEntryMask(second);
	if (maskA == 0 || maskB == 0 ||
		(layerA != 0 && (maskB & layerA) == 0) ||
		(layerB != 0 && (maskA & layerB) == 0) ||
		!colliderA->CheckCollision(colliderB)) {
		return false;
	}
	isTrigger = (first.component && first.component->IsTrigger()) ||
		(second.component && second.component->IsTrigger());
	contact = isTrigger ? std::nullopt : MakeContact(colliderA, colliderB);
	return true;
}

void CollisionManager::QueuePairEvents(const CollisionEntry& first, const CollisionEntry& second,
	TakeC::CollisionPhase phase, bool isTrigger,
	const std::optional<TakeC::CollisionContactInfo>& contact) {
	eventQueue_.push_back({ first.objectId, second.objectId, first.colliderId,
		second.colliderId, phase, isTrigger, contact });
	auto reverseContact = contact;
	if (reverseContact) {
		reverseContact->normal = -reverseContact->normal;
	}
	eventQueue_.push_back({ second.objectId, first.objectId, second.colliderId,
		first.colliderId, phase, isTrigger, reverseContact });
}

void CollisionManager::NotifyCollision(const TakeC::CollisionEvent& event) {
	const CollisionEntry* current = ResolveEntry(event.selfColliderId, event.selfObjectId);
	if (!current) {
		return;
	}
	const CollisionEntry* other = ResolveEntry(event.otherColliderId, event.otherObjectId);
	if (!other && event.phase != TakeC::CollisionPhase::Exit) {
		return;
	}
	if (current->component != nullptr) {
		current->component->NotifyCollisionEvent(event);
	}
	// 新 API の通知で所有者が破棄され得るため、必ず ID から引き直す。
	current = ResolveEntry(event.selfColliderId, event.selfObjectId);
	other = ResolveEntry(event.otherColliderId, event.otherObjectId);
	if (!current || !other || event.phase == TakeC::CollisionPhase::Exit) {
		return;
	}
	if (current->component != nullptr) {
		current->component->NotifyCollision({
			GetEntryCollider(*other),
			other->component != nullptr ? &other->component->GetOwner() : nullptr,
			GetEntryCharacter(*other),
		});
	}
	current = ResolveEntry(event.selfColliderId, event.selfObjectId);
	other = ResolveEntry(event.otherColliderId, event.otherObjectId);
	if (current && other && !event.isTrigger) {
		GameCharacter* currentCharacter = GetEntryCharacter(*current);
		GameCharacter* otherCharacter = GetEntryCharacter(*other);
		if (currentCharacter != nullptr && otherCharacter != nullptr) {
			currentCharacter->OnCollisionAction(otherCharacter);
		}
	}
}

void CollisionManager::DetectCollisions() {
	if (dispatchingEvents_) {
		return;
	}
	const std::vector<CollisionEntry> entries = CollectEntries();
	std::unordered_map<PairKey, PairState, PairHash> currentPairs;
	for (std::size_t indexA = 0; indexA < entries.size(); ++indexA) {
		for (std::size_t indexB = indexA + 1; indexB < entries.size(); ++indexB) {
			const CollisionEntry& entryA = entries[indexA];
			const CollisionEntry& entryB = entries[indexB];
			bool isTrigger = false;
			std::optional<TakeC::CollisionContactInfo> contact;
			if (!TestPair(entryA, entryB, isTrigger, contact)) {
				continue;
			}
			const PairKey key{ std::min(entryA.colliderId, entryB.colliderId),
				std::max(entryA.colliderId, entryB.colliderId) };
			currentPairs[key] = entryA.colliderId == key.first
				? PairState{ entryA.objectId, entryB.objectId, isTrigger }
				: PairState{ entryB.objectId, entryA.objectId, isTrigger };
			QueuePairEvents(entryA, entryB,
				previousPairs_.contains(key) ? TakeC::CollisionPhase::Stay : TakeC::CollisionPhase::Enter,
				isTrigger, contact);
		}
	}
	for (const auto& [key, state] : previousPairs_) {
		if (!currentPairs.contains(key)) {
			eventQueue_.push_back({ state.firstObjectId, state.secondObjectId,
				key.first, key.second, TakeC::CollisionPhase::Exit, state.isTrigger, std::nullopt });
			eventQueue_.push_back({ state.secondObjectId, state.firstObjectId,
				key.second, key.first, TakeC::CollisionPhase::Exit, state.isTrigger, std::nullopt });
		}
	}
	previousPairs_ = std::move(currentPairs);
}

void CollisionManager::DispatchCollisionEvents() {
	if (dispatchingEvents_) {
		return;
	}
	dispatchingEvents_ = true;
	std::vector<TakeC::CollisionEvent> pending;
	pending.swap(eventQueue_);
	try {
		for (const TakeC::CollisionEvent& event : pending) {
			NotifyCollision(event);
		}
	} catch (...) {
		dispatchingEvents_ = false;
		throw;
	}
	dispatchingEvents_ = false;
}

void CollisionManager::CheckAllCollisions() {
	if (dispatchingEvents_) {
		return;
	}
	DetectCollisions();
	DispatchCollisionEvents();
}

//=============================================================================
// ゲームキャラクター同士の衝突判定
//=============================================================================
void CollisionManager::CheckCollisionPairForGameCharacter(GameCharacter* gameCharacterA, GameCharacter* gameCharacterB) {
	if (!gameCharacterA || !gameCharacterB || gameCharacterA == gameCharacterB || dispatchingEvents_) {
		return;
	}
	const bool temporaryA = !activeGameCharacters_.contains(gameCharacterA);
	const bool temporaryB = !activeGameCharacters_.contains(gameCharacterB);
	if (temporaryA) { RegisterGameCharacter(gameCharacterA); }
	if (temporaryB) { RegisterGameCharacter(gameCharacterB); }
	const std::vector<CollisionEntry> entries = CollectEntries();
	const CollisionEntry* first = nullptr;
	const CollisionEntry* second = nullptr;
	for (const CollisionEntry& entry : entries) {
		if (GetEntryCharacter(entry) == gameCharacterA) { first = &entry; }
		if (GetEntryCharacter(entry) == gameCharacterB) { second = &entry; }
	}
	if (!first || !second) {
		if (temporaryA) { UnregisterGameCharacter(gameCharacterA); }
		if (temporaryB) { UnregisterGameCharacter(gameCharacterB); }
		return;
	}
	const PairKey key{ std::min(first->colliderId, second->colliderId),
		std::max(first->colliderId, second->colliderId) };
	bool isTrigger = false;
	std::optional<TakeC::CollisionContactInfo> contact;
	if (TestPair(*first, *second, isTrigger, contact)) {
		const TakeC::CollisionPhase phase = previousPairs_.contains(key)
			? TakeC::CollisionPhase::Stay : TakeC::CollisionPhase::Enter;
		previousPairs_[key] = first->colliderId == key.first
			? PairState{ first->objectId, second->objectId, isTrigger }
			: PairState{ second->objectId, first->objectId, isTrigger };
		QueuePairEvents(*first, *second, phase, isTrigger, contact);
	} else if (const auto previous = previousPairs_.find(key); previous != previousPairs_.end()) {
		QueuePairEvents(*first, *second, TakeC::CollisionPhase::Exit, previous->second.isTrigger);
		previousPairs_.erase(previous);
	}
	DispatchCollisionEvents();
	if (temporaryA) { UnregisterGameCharacter(gameCharacterA); }
	if (temporaryB) { UnregisterGameCharacter(gameCharacterB); }
}

//=============================================================================
// レイキャスト処理
//=============================================================================
bool CollisionManager::RayCast(const Ray& ray, RayCastHit& outHit,uint32_t layerMask) {

	bool result = false;
	float closestDistance = ray.distance;
	RayCastHit tempHit;
	for (const CollisionEntry& entry : CollectEntries()) {
		if (!IsEntryActive(entry)) {
			continue;
		}
		Collider* collider = GetEntryCollider(entry);
		// レイヤーマスクによる絞り込み
		// もしコライダーのレイヤーIDがlayerMaskに含まれていなければスキップ
		if (!(static_cast<uint32_t>(collider->GetCollisionLayerID()) & layerMask)) continue;

		// レイとコライダーの交差判定
		if (collider->Intersects(ray, tempHit)) {
			// 最も近いヒットを記録
			if (tempHit.distance < closestDistance) {
				closestDistance = tempHit.distance;
				outHit = tempHit;
				result = true;
			}
		}
	}
	return result;
}

//=============================================================================
// 球キャスト処理
//=============================================================================
bool CollisionManager::SphereCast(const Ray& ray, float radius, RayCastHit& outHit, uint32_t layerMask) {
	bool result = false;
	float closestDistance = ray.distance;
	RayCastHit tempHit;

	for (const CollisionEntry& entry : CollectEntries()) {
		if (!IsEntryActive(entry)) {
			continue;
		}
		Collider* collider = GetEntryCollider(entry);
		// レイヤーマスクによる絞り込み
		if (!(static_cast<uint32_t>(collider->GetCollisionLayerID()) & layerMask)) continue;

		// スフィアキャスト判定
		// IntersectsSphere を呼び出す
		if (collider->IntersectsSphere(ray, radius, tempHit)) {
			// 最も近いヒットを記録
			if (tempHit.distance < closestDistance) {
				closestDistance = tempHit.distance;
				outHit = tempHit;
				result = true;
			}
		}
	}
	return result;
}

//=============================================================================
// カプセルキャスト処理
//=============================================================================
bool CollisionManager::CapsuleCast(const Capsule& capsule, RayCastHit& outHit, uint32_t layerMask){
	bool result = false;
	float closestDistance = Vector3Math::Length(capsule.end - capsule.start);
	RayCastHit tempHit;

	for (const CollisionEntry& entry : CollectEntries()) {
		if (!IsEntryActive(entry)) {
			continue;
		}
		Collider* collider = GetEntryCollider(entry);
		// レイヤーマスクによる絞り込み
		if (!(static_cast<uint32_t>(collider->GetCollisionLayerID()) & layerMask)) continue;

		// カプセル判定
		if (collider->IntersectsCapsule(capsule, tempHit)) {
			// 最も近いヒットを記録
			if (tempHit.distance < closestDistance) {
				closestDistance = tempHit.distance;
				outHit = tempHit;
				result = true;
			}
		}
	}
	return result;
}
