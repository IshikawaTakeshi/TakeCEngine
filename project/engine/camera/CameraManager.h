#pragma once
#include <unordered_map>
#include <map>
#include <vector>
#include <string>
#include <memory>

#include "Camera.h"

//前方宣言
class DirectXCommon;
class Camera;

//============================================================================
// CameraManager class
//============================================================================
class CameraManager {
private:

	//コピーコンストラクタ・代入演算子禁止
	CameraManager() = default;
	~CameraManager() = default;
	CameraManager(const CameraManager&) = delete;
	CameraManager& operator=(const CameraManager&) = delete;

public:
	
	//=============================================================================
	// functions
	//=============================================================================

	/// <summary>
	/// インスタンスの取得
	/// </summary>
	static CameraManager& GetInstance();

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="dxCommon"></param>
	void Initialize(DirectXCommon* dxCommon);

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update();

	/// <summary>
	/// ImGui更新処理
	/// </summary>
	void UpdateImGui();

	/// <summary>
	/// 終了・開放処理
	/// </summary>
	void Finalize();

	/// <summary>
	/// カメラの追加
	/// </summary>
	/// <param name="name"></param>
	/// <param name="camera"></param>
	void AddCamera(std::string name, const Camera& camera);

	/// <summary>
	/// カメラのリセット
	/// </summary>
	void ResetCameras();

public:

	//=============================================================================
	// accessors
	//=============================================================================

	//----- getter ----------------

	/// アクティブなカメラの取得
	Camera* GetActiveCamera();
	/// カメラの数の取得
	int GetCameraCount() const;
	/// DirectXCommonの取得
	DirectXCommon* GetDirectXCommon() const { return dxCommon_; }

	//----- setter ----------------

	/// アクティブなカメラの設定
	void SetActiveCamera(std::string name);

private:

	//DirectXCommon
	DirectXCommon* dxCommon_ = nullptr;
	//登録したカメラのマップ
	std::unordered_map<std::string, std::unique_ptr<Camera>> cameras_;
	Camera* activeCamera_;  // 現在アクティブなカメラ
};