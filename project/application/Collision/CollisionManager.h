#pragma once


#include <list>
#include <memory>
class Collider;
class CollisionManager {


public:

	////////////////////////////////////////////////////////////////////////////////////////
	///		publicメンバ関数
	////////////////////////////////////////////////////////////////////////////////////////


	/// <summary>
	/// シングルトンインスタンスの取得関数
	/// </summary>
	/// <returns></returns>
	static CollisionManager* GetInstance();

	/// <summary>
	/// シングルトンインスタンスの解放処理
	/// </summary>
	void Finalize();

	/// <summary>
	/// コライダーリストへの登録をする関数
	/// </summary>
	void RegisterCollider(Collider* collider);

	/// <summary>
	/// コライダーリストをクリアする関数
	/// </summary>
	void ClearCollider();

	/// <summary>
	/// 全てのコライダーの衝突判定を行う関数
	/// </summary>
	void CheckAllCollisions();

	/// <summary>
	/// コライダー2つ衝突判定と応答処理
	/// </summary>
	/// <param name="colliderB">コライダーA</param>
	/// <param name="colliderB">コライダーB</param>
	void CheckCollisionPair(Collider* colliderA, Collider* colliderB);

private:

	//シングルトンインスタンス
	static CollisionManager* instance_;

	CollisionManager() = default;
	~CollisionManager() = default;
	CollisionManager(CollisionManager&) = delete;
	CollisionManager& operator=(CollisionManager&) = delete;

private:

	////////////////////////////////////////////////////////////////////////////////////////
	///		privateメンバ関数
	////////////////////////////////////////////////////////////////////////////////////////

	//コライダーリスト
	std::list<Collider*> colliders_;
};

