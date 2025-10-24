#pragma once
#include <list>

#include "Camera.h"
#include "engine/3d/Object3d.h"

//============================================================================
// Ground class
//============================================================================
class Ground : public Object3d {
public:

	Ground() = default;
	~Ground();

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(Object3dCommon* object3dCommon, const std::string& filePath);

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update();

	/// <summary>
	/// 描画処理
	/// </summary>
	void Draw();

	void DrawGridGround();

	/// <summary>
	/// ImGuiでのデバッグ処理
	/// </summary>
	void ImGuiDebug();

private:

	float gridGroundSize_ = 1000.0f;
};