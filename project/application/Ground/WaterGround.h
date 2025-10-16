#pragma once
#include "engine/3d/Object3d.h"

class WaterGround : public Object3d {
public:

	WaterGround() = default;
	~WaterGround() = default;

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
	/// <summary>
	/// ImGuiでのデバッグ処理
	/// </summary>
	void ImGuiDebug();

};

