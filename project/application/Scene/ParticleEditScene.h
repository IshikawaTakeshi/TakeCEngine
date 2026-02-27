#pragma once
#include "BaseScene.h"
#include "camera/Camera.h"
#include "SkyBox/SkyBox.h"
#include "2d/Sprite.h"
#include "3d/Particle/ParticleCommon.h"
#include "3d/Particle/ParticleEditor.h"
#include "3d/Particle/EffectEditor.h"

//============================================================================
// ParticleEditScene class
//============================================================================
class ParticleEditScene : public BaseScene {
public:

	//======================================================================
	/// functions
	//======================================================================

	//初期化
	void Initialize() override;

	//終了処理
	void Finalize() override;

	//更新処理
	void Update() override;

	//ImGuiの更新処理
	void UpdateImGui() override;

	//描画処理
	void DrawObject_GBuffer() override;
	//描画処理(Forward)
	void DrawObject_Forward() override;
	//スプライト描画処理
	void DrawSprite() override;
	//シャドウ描画処理
	void DrawShadow() override;

private:

	// カメラ
	std::shared_ptr<Camera> previewCamera_ = nullptr;
	//SkyBox
	std::unique_ptr<SkyBox> skyBox_ = nullptr;

	//ParticleEditor
	std::unique_ptr<ParticleEditor> particleEditor_ = nullptr;
	//EffectEditor（追加）
	std::unique_ptr<TakeC::EffectEditor> effectEditor_ = nullptr;

	// エディタモード切り替え
	enum class EditorMode {
		Particle,  // パーティクル編集
		Effect     // エフェクト編集
	};
	EditorMode currentMode_ = EditorMode::Particle;
};