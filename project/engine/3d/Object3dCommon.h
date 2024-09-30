#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>

class PSO;
class Camera;
class DirectXCommon;
class Object3dCommon {

public:

	//エイリアステンプレート
	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;

	static Object3dCommon* GetInstance();

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(DirectXCommon* directXCommon);

	/// <summary>
	/// 開放処理
	/// </summary>
	void Finalize();

	/// <summary>
	/// 描画前処理
	/// </summary>
	void PreDraw();

//================================================================================================
// 	   getter
//================================================================================================

	DirectXCommon* GetDirectXCommon() const { return dxCommon_; }

	Camera* GetDefaultCamera() const { return defaultCamera_; }

//================================================================================================
// 	   setter
//================================================================================================

	void SetDefaultCamera(Camera* camera) { defaultCamera_ = camera; }

private:

	Object3dCommon() = default;
	~Object3dCommon() = default;
	Object3dCommon(const Object3dCommon&) = delete;
	Object3dCommon& operator=(const Object3dCommon&) = delete;

private:

	//インスタンス
	static Object3dCommon* instance_;

	//DirectXCommon
	DirectXCommon* dxCommon_ = nullptr;

	//RootSignature
	ComPtr<ID3D12RootSignature> rootSignature_ = nullptr;

	//PSO
	PSO* pso_ = nullptr;

	//defaultCamera
	Camera* defaultCamera_ = nullptr;

};

