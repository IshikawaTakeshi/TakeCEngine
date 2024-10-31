#pragma once
#include <unordered_map>
#include <map>
#include <vector>
#include <string>
#include <memory>
class Camera;
class CameraManager {
public:
	
	static CameraManager* GetInstance();

	void Update();

	void UpdateImGui();

	void Finalize();

	// カメラを追加するメソッド
	void AddCamera(std::string name, const Camera& camera);

	//カメラの情報のリセット
	void ResetCameras();

	// アクティブカメラのリセット
	//void ResetActiveCamera();

	// アクティブなカメラを設定する
	void SetActiveCamera(std::string name);

	// アクティブなカメラを取得する
	Camera* GetActiveCamera();

	// カメラの数を取得する
	int GetCameraCount() const;


private:

	CameraManager() = default;
	~CameraManager() = default;
	CameraManager(const CameraManager&) = delete;
	CameraManager& operator=(const CameraManager&) = delete;

private:

	static CameraManager* instance_;
	std::unordered_map<std::string, std::unique_ptr<Camera>> cameras_;
	Camera* activeCamera_;  // 現在アクティブなカメラ
};

