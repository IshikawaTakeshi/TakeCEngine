#pragma once

#include "engine/base/PipelineStateObject.h"
#include "engine/Entity/GameCharacter.h"
#include "engine/math/physics/Ray.h"
#include <list>
#include <memory>

//前方宣言
class DirectXCommon;
class Collider;

//============================================================================
//	CollisionManager class
//============================================================================
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

	/// <summary>
	/// ゲームキャラクターの解放
	/// </summary>
	void ClearGameCharacter();

	/// <summary>
	/// 全てのゲームキャラクターの衝突判定を行う関数
	/// </summary>
	void CheckAllCollisionsForGameCharacter();

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


private:

	////////////////////////////////////////////////////////////////////////////////////////
	///		privateメンバ関数
	////////////////////////////////////////////////////////////////////////////////////////

	//dxCommon
	TakeC::DirectXCommon* dxCommon_ = nullptr;

	//コライダーリスト
	std::vector<Collider*> colliders_;
	//ゲームキャラクターリスト
	std::list<GameCharacter*> gameCharacters_;
	//パイプラインステートオブジェクト
	std::unique_ptr<PSO> pso_ = nullptr;
	//ルートシグネチャ
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_ = nullptr;
};

