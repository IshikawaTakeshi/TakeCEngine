#pragma once
#include "engine/2d/Sprite.h"
#include <memory>

//============================================================================
// PlayerReticle class
//============================================================================
class PlayerReticle {
public:

	PlayerReticle() = default;
	~PlayerReticle() = default;

	//========================================================================
	// functions
	//========================================================================

	// 初期化
	void Initialize();
	// 更新
	void Update(const Vector3& targetPosition);
	// 描画
	void Draw();
	// ImGuiの更新
	void UpdateImGui();

private:

	// レティクルのスプライト
	std::unique_ptr<Sprite> reticleSprite_ = nullptr; 
	//予測着弾点のレティクルスプライト
	std::unique_ptr<Sprite> predictedImpactReticle_ = nullptr;
	Vector2 screenPosition_{0.0f,0.0f}; // スクリーン上のレティクルの位置
	Vector3 targetPosition_{0.0f,0.0f,0.0f}; // レティクルが狙う位置
	float size_ = 256.0f; // レティクルのサイズ
	float alpha_ = 1.0f; // レティクルの透明度
	bool isFocus_ = true; // 敵を狙っているかどうか
};

