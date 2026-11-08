#pragma once

#include "engine/GameObject/GameObject.h"
#include "engine/GameObject/EngineServices.h"

#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace TakeC {

/// <summary>
/// GameObjectの所有、検索、一括更新、安全な遅延破棄を担当するクラス
/// </summary>
class GameObjectManager final {
public:

	/// コンストラクタ・デストラクタ
	GameObjectManager() = default;
	~GameObjectManager() = default;

	/// コピー禁止・ムーブ禁止
	GameObjectManager(const GameObjectManager&) = delete;
	GameObjectManager& operator=(const GameObjectManager&) = delete;
	GameObjectManager(GameObjectManager&&) = delete;
	GameObjectManager& operator=(GameObjectManager&&) = delete;

	/// GameObjectの生成
	GameObject& CreateObject(std::string name = "GameObject");
	/// 完成したStandaloneオブジェクトを登録する。Component本体のアドレスは変えない。
	GameObject& AdoptObject(std::unique_ptr<GameObject> object);
	/// GameObjectの削除
	void DestroyObject(GameObjectId id);
	/// GameObjectの検索
	GameObject* FindObject(GameObjectId id);
	const GameObject* FindObject(GameObjectId id) const;
	/// GameObjectの名前で検索
	GameObject* FindObjectByName(std::string_view name);
	const GameObject* FindObjectByName(std::string_view name) const;

	/// 初期化
	void Initialize();
	/// 更新処理
	void Update(float deltaTime);
	/// 固定更新処理
	void FixedUpdate(float fixedDeltaTime);
	/// 遅延更新処理
	void LateUpdate(float deltaTime);
	/// 変更の反映
	void FlushPendingChanges();
	/// 登録した全GameObjectの破棄
	void Clear();

public:

	/// Frame番号の取得
	std::uint64_t GetFrameIndex() const { return frameIndex_; }
	/// EngineServicesの取得
	EngineServices& GetServices() { return services_; }
	const EngineServices& GetServices() const { return services_; }

	/// GameObjectの総数の取得
	std::size_t GetObjectCount() const {
		return objects_.size() + pendingObjects_.size();
	}

private:

	// フレーム番号の進行が必要な場合に進める処理
	void BeginFrameIfNeeded(GameObject::UpdateKind kind);
	void RunPhasedUpdate(GameObject::UpdateKind kind, float deltaTime);

	// GameObjectの総当たり更新処理
	template<class Function>
	void ForEachObject(Function&& function) {
		iteratingObjects_ = true;
		for (const std::unique_ptr<GameObject>& object : objects_) {
			if (!object->IsDestroyRequested()) {
				function(*object);
			}
		}
		iteratingObjects_ = false;
		FlushPendingChanges();
	}

	
	EngineServices services_;

	// GameObjectの所有リスト
	std::vector<std::unique_ptr<GameObject>> objects_;
	// GameObjectの追加待ちリスト
	std::vector<std::unique_ptr<GameObject>> pendingObjects_;
	// 現在のフレーム番号
	std::uint64_t frameIndex_ = 0;
	// フレーム番号が進んだかどうか
	bool frameOpen_ = false;
	// Updateが呼ばれたフレームで、フレーム番号を1回だけ進めるためのフラグ
	bool updatedThisFrame_ = false;
	// GameObjectの更新中かどうか
	bool iteratingObjects_ = false;
	// GameObjectのクリアが要求されたかどうか
	bool clearRequested_ = false;
};

} // namespace TakeC
