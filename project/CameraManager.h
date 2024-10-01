#pragma once
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
	void AddCamera(const Camera& camera);

	// アクティブなカメラを設定する
	void SetActiveCamera(int index);

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
	std::vector<std::unique_ptr<Camera>> cameras_;
	Camera* activeCamera_;  // 現在アクティブなカメラ
};

