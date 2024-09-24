#pragma once
class Object3d {

public: // publicメンバ関数

	Object3d() = default;
	~Object3d() = default;

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

	/// <summary>
	/// 終了処理
	/// </summary>
	void Update();

	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

private: // privateメンバ変数

};

