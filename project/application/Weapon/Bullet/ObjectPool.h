#pragma once
#include <vector>
#include <memory>
#include "engine/3d/Object3dCommon.h"

//=============================================================================================
/// ObjectPool class
//=============================================================================================
template <class T>
class ObjectPool {
public:

	/// <summary>
	/// コンストラクタ・デストラクタ
	/// </summary>
	ObjectPool() = default;
	~ObjectPool() = default;

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="size"></param>
	void Initialize(Object3dCommon* objectCommon, size_t size);

	/// <summary>
	/// 終了処理
	/// </summary>
	void Finalize() { pool_.clear(); }

	/// <summary>
	/// オブジェクトの取得
	/// </summary>
	/// <returns></returns>
	T* GetObject();

	/// <summary>
	/// 全オブジェクトの更新
	/// </summary>
	void UpdateAll();

	/// <summary>
	/// 全オブジェクトの描画
	/// </summary>
	void DrawAll();

	/// <summary>
	/// 全オブジェクトのコライダー描画
	/// </summary>
	void DrawAllCollider();

	/// <summary>
	/// プール内の全オブジェクトを取得
	/// </summary>
	/// <returns></returns>
	std::vector<T*> GetPool();

protected:
	// 型ごとの初期化処理

	virtual void OnInitializeObject([[maybe_unused]]T& object) {
		// デフォルトは何もしない
	}

	// Object3dCommonのポインタ
	Object3dCommon* objectCommon_ = nullptr;

	// オブジェクトプール
	std::vector<std::unique_ptr<T>> pool_;
};

///-------------------------------------------------------------------------------
/// 初期化
///-------------------------------------------------------------------------------
template<class T>
inline void ObjectPool<T>::Initialize(Object3dCommon* objectCommon, size_t size) {
	//objectCommonの保存
	objectCommon_ = objectCommon;

	// プールの生成
	pool_.reserve(size);
	for (size_t i = 0; i < size; ++i) {
		pool_.emplace_back(std::make_unique<T>());
		OnInitializeObject(*pool_.back());
	}
}

///-------------------------------------------------------------------------------
/// オブジェクトの取得
///-------------------------------------------------------------------------------
template<class T>
inline T* ObjectPool<T>::GetObject() {

	for (const auto& obj : pool_) {
		if (!obj->IsActive()) {
			obj->SetIsActive(true);
			return obj.get();
		}
	}
	return nullptr; // プールに空きがない場合はnullptrを返す
}

///-------------------------------------------------------------------------------
/// 全オブジェクトの更新
///-------------------------------------------------------------------------------
template<class T>
inline void ObjectPool<T>::UpdateAll() {

	for (const auto& obj : pool_) {
		if (obj->IsActive()) {
			obj->Update();
		}
	}
}

///-------------------------------------------------------------------------------
/// 全オブジェクトの描画
///-------------------------------------------------------------------------------
template<class T>
inline void ObjectPool<T>::DrawAll() {

	for (const auto& obj : pool_) {
		if (obj->IsActive()) {
			obj->Draw();
		}
	}
}

///-------------------------------------------------------------------------------
/// 全オブジェクトのコライダー描画
///-------------------------------------------------------------------------------
template<class T>
inline void ObjectPool<T>::DrawAllCollider() {

	for (const auto& obj : pool_) {
		if (obj->IsActive()) {
			obj->DrawCollider();
		}
	}
}

///-------------------------------------------------------------------------------
/// プール内の全オブジェクトを取得
///-------------------------------------------------------------------------------
template<class T>
inline std::vector<T*> ObjectPool<T>::GetPool() {

	std::vector<T*> objects;
	objects.reserve(pool_.size());
	for (const auto& obj : pool_) {
		objects.push_back(obj.get());
	}
	return objects;
}
