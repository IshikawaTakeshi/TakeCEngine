#pragma once

#include "engine/base/PipelineStateObject.h"
#include "engine/Entity/GameCharacter.h"
#include "engine/math/physics/Ray.h"
#include "engine/Collision/Capsule.h"
#include "engine/Collision/CollisionEvent.h"
#include <cstdint>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

//前方宣言
class DirectXCommon;
namespace TakeC {
	class Collider;
	class CollisionComponent;
}
using TakeC::Collider;

//============================================================================
//	CollisionManager class
//============================================================================
/// <summary>
/// Colliderの登録、衝突判定、レイ・形状キャストを一元管理するクラスです。
/// </summary>
class CollisionManager {
private:

	//コピーコンストラクタ・代入演算子禁止
	CollisionManager() = default;
	~CollisionManager() = default;
	CollisionManager(CollisionManager&) = delete;
	CollisionManager& operator=(CollisionManager&) = delete;

public:

	///=======================================================================
	// functions
	///=======================================================================

	/// <summary>
	/// シングルトンインスタンスの取得
	/// </summary>
	/// <returns></returns>
	static CollisionManager& GetInstance();

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="dxCommon"></param>
	void Initialize(TakeC::DirectXCommon* dxCommon);

	/// <summary>
	/// 描画前処理
	/// </summary>
	void PreDraw();

	/// <summary>
	/// 解放処理
	/// </summary>
	void Finalize();

	/// <summary>
	/// ゲームキャラクターの登録
	/// </summary>
	/// <param name="gameCharacter"></param>
	void RegisterGameCharacter(GameCharacter* gameCharacter);
	void UnregisterGameCharacter(GameCharacter* gameCharacter);
	void RegisterCollisionComponent(TakeC::CollisionComponent* component);
	void UnregisterCollisionComponent(TakeC::CollisionComponent* component);

	/// <summary>
	/// ゲームキャラクターの解放
	/// </summary>
	void ClearGameCharacter();
	/// <summary>
	/// 1 フレームにつき一度、衝突 API を呼ぶ前に実行します。
	/// 旧・新 API が同じフレームに呼ばれても、各コライダー組を一度だけ判定します。
	/// </summary>
	void BeginCollisionFrame();

	/// <summary>
	/// 全てのゲームキャラクターの衝突判定を行う関数
	/// </summary>
	void CheckAllCollisionsForGameCharacter();
	void CheckAllCollisions();
	/// <summary>現在の衝突を検出し、Enter/Stay/Exit をキューに積みます。</summary>
	void DetectCollisions();
	/// <summary>登録 ID と破棄状態を再検証してイベントを配送します。</summary>
	void DispatchCollisionEvents();

	/// <summary>
	/// ゲームキャラクター同士の衝突判定を行う関数
	/// </summary>
	/// <param name="gameCharacterA"></param>
	/// <param name="gameCharacterB"></param>
	void CheckCollisionPairForGameCharacter(GameCharacter* gameCharacterA, GameCharacter* gameCharacterB);

	/// <summary>
	/// レイキャスト処理
	/// </summary>
	/// <param name="ray"></param>
	/// <param name="outHit"></param>
	/// <param name="layerMask"></param>
	/// <returns></returns>
	bool RayCast(const Ray& ray, RayCastHit& outHit,uint32_t layerMask);

	/// <summary>
	/// 球キャスト処理
	/// </summary>
	/// <param name="ray"></param>
	/// <param name="radius"></param>
	/// <param name="outHit"></param>
	/// <param name="layerMask"></param>
	/// <returns></returns>
	bool SphereCast(const Ray& ray, float radius, RayCastHit& outHit, uint32_t layerMask);

	bool CapsuleCast(const Capsule& capsule, RayCastHit& outHit, uint32_t layerMask);

private:
	/// <summary>衝突判定中に参照する登録先を表します。</summary>
	struct CollisionEntry final {
		TakeC::ColliderId colliderId = TakeC::kInvalidColliderId;
		TakeC::GameObjectId objectId = TakeC::kInvalidGameObjectId;
		TakeC::CollisionComponent* component = nullptr;
		GameCharacter* character = nullptr;
	};
	/// <summary>コライダー ID を昇順に並べた衝突組のキーです。</summary>
	struct PairKey final {
		TakeC::ColliderId first = TakeC::kInvalidColliderId;
		TakeC::ColliderId second = TakeC::kInvalidColliderId;
		bool operator==(const PairKey&) const = default;
	};
	/// <summary>衝突組をハッシュ表で検索するための関数オブジェクトです。</summary>
	struct PairHash final {
		std::size_t operator()(const PairKey& pair) const {
			return std::hash<TakeC::ColliderId>{}(pair.first) ^
				(std::hash<TakeC::ColliderId>{}(pair.second) << 1);
		}
	};
	/// <summary>Exit 通知に必要な前フレームの所有者と trigger 情報です。</summary>
	struct PairState final {
		TakeC::GameObjectId firstObjectId = TakeC::kInvalidGameObjectId;
		TakeC::GameObjectId secondObjectId = TakeC::kInvalidGameObjectId;
		bool isTrigger = false;
	};

	std::vector<CollisionEntry> CollectEntries() const;
	bool IsEntryActive(const CollisionEntry& entry) const;
	const CollisionEntry* ResolveEntry(TakeC::ColliderId id, TakeC::GameObjectId objectId) const;
	Collider* GetEntryCollider(const CollisionEntry& entry) const;
	GameCharacter* GetEntryCharacter(const CollisionEntry& entry) const;
	std::uint32_t GetEntryMask(const CollisionEntry& entry) const;
	bool TestPair(const CollisionEntry& first, const CollisionEntry& second,
		bool& isTrigger, std::optional<TakeC::CollisionContactInfo>& contact) const;
	void QueuePairEvents(const CollisionEntry& first, const CollisionEntry& second,
		TakeC::CollisionPhase phase, bool isTrigger,
		const std::optional<TakeC::CollisionContactInfo>& contact = std::nullopt);
	void NotifyCollision(const TakeC::CollisionEvent& event);

	////////////////////////////////////////////////////////////////////////////////////////
	///		privateメンバ関数
	////////////////////////////////////////////////////////////////////////////////////////

	//dxCommon
	TakeC::DirectXCommon* dxCommon_ = nullptr;

	//GameObjectが所有するコライダー
	std::vector<TakeC::CollisionComponent*> collisionComponents_;
	std::unordered_set<TakeC::CollisionComponent*> activeCollisionComponents_;
	//ゲームキャラクターリスト
	std::vector<GameCharacter*> gameCharacters_;
	std::unordered_set<GameCharacter*> activeGameCharacters_;
	std::unordered_map<GameCharacter*, TakeC::ColliderId> legacyColliderIds_;
	std::unordered_map<TakeC::ColliderId, CollisionEntry> registeredEntries_;
	std::unordered_map<PairKey, PairState, PairHash> previousPairs_;
	std::unordered_map<PairKey, PairState, PairHash> currentPairs_;
	std::unordered_set<PairKey, PairHash> checkedPairs_;
	std::unordered_set<PairKey, PairHash> exitedPairs_;
	std::vector<TakeC::CollisionEvent> eventQueue_;
	bool frameStarted_ = false;
	bool fullSweepDone_ = false;
	bool dispatchingEvents_ = false;
	//パイプラインステートオブジェクト
	std::unique_ptr<TakeC::PSO> pso_ = nullptr;
	//ルートシグネチャ
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_ = nullptr;
};

