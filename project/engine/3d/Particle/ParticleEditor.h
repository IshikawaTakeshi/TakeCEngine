#pragma once
#include "engine/3d/Particle/BaseParticleGroup.h"
#include "engine/3d/Particle/ParticleEmitter.h"
#include "engine/3d/Particle/ParticleCommon.h"
#include "engine/3d/Primitive/PrimitiveParameter.h"
#include "engine/base/Particle/ParticleManager.h"
#include <string>
#include <vector>
#include <memory>

//============================================================================
// ParticleEditor class
//============================================================================
class ParticleEditor {
public:

	ParticleEditor() = default;
	~ParticleEditor() = default;

	//======================================================================
	// functions
	//======================================================================

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="particleManager"></param>
	/// <param name="particleCommon"></param>
	void Initialize(ParticleCommon* particleCommon);

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
	/// 描画処理
	/// </summary>
	void Draw();

private:
	//Preset Management
	//プリセットを保存
	void SavePreset(const std::string& presetName);
	//プリセットを適用
	void LoadPreset(const std::string& presetName);
	//プリセットを削除
	void DeletePreset(const std::string& presetName);

	//デフォルトプリセットを読み込む
	void LoadDefaultPreset();
	//全てのプリセットを読み込む
	void LoadAllPresets();
	//プリセットリストを更新
	void RefreshPresetList();

	//UIの描画
	void DrawParticleAttributesEditor();
	void DrawEmitterControls();
	void DrawPreviewSettings();
	void DrawPresetManager();

	//上書き確認ダイアログの描画
	void DrawOverwriteConfirmDialog();

	//プリミティブパラメータ初期化・UI描画・更新
	void InitPrimitiveParam(PrimitiveType type, TakeC::PrimitiveParameter& params);
	bool DrawPrimitiveParametersUI(PrimitiveType type, TakeC::PrimitiveParameter& params);
	void UpdatePrimitiveFromParameters(const std::string& groupName, PrimitiveType type, const TakeC::PrimitiveParameter& params);

	//textureAnimation設定UI描画・更新
	void InitTextureAnimationParam(TakeC::TextureAnimationType type, TextureAnimationVariant& params);
	bool DrawTextureAnimationUI(TakeC::TextureAnimationType type, TextureAnimationVariant& params);
	void UpdateTextureAnimationFromParameters(TakeC::TextureAnimationType type, const TextureAnimationVariant& params);

private:

	// パーティクル共通情報
	ParticleCommon* particleCommon_ = nullptr;
	//エディター専用エミッター
	std::unique_ptr<ParticleEmitter> previewEmitter_;

	//エディター設定
	ParticlePreset currentPreset_;
	std::string currentGroupName_;
	std::string selectedPresetName_;

	//プレビュー設定
	EulerTransform emitterTransform_;
	Vector3 emitterDirection_;
	uint32_t emitCount_ = 10;
	float emitFrequency_ = 0.1f;
	bool autoEmit_ = false;
	bool autoApply_ = true; //属性を自動的に適用するかどうか

	// プリセット管理
	std::vector<std::string> presetNames_;
	std::map<std::string, ParticlePreset> presets_;
	// テクスチャファイル名のリスト(参照渡し)
	std::vector<std::string> textureFileNames_; 

	// 上書き確認ダイアログ表示フラグ
	bool showOverwriteConfirm_ = false;
	// 上書き予定のプリセット名
	std::string pendingPresetName_;

	// エラーメッセージ表示用
	std::string saveErrorMessage_;
	float errorDisplayTimer_ = 0.0f;
};