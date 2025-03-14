#pragma once

#include "Camera.h"
#include "engine/3d/Object3d.h"

#include <list>

/// <summary>
/// プレイヤー
/// </summary>
class Ground {
public:
	////////////////////////////////////////////////////////////////////////////////////////
	///		publicメンバ関数
	////////////////////////////////////////////////////////////////////////////////////////

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

	/// <summary>
	/// ImGuiでのデバッグ処理
	/// </summary>
	void ImGuiDebug();

private:


};