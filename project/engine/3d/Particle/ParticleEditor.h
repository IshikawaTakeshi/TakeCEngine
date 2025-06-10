#pragma once
#include "Particle/BaseParticleGroup.h"
#include "Particle/ParticleEmitter.h"
#include "Particle/ParticleManager.h"
#include "Particle/ParticleCommon.h"
#include <string>
#include <vector>
#include <memory>

class ParticleEditor {
public:

	ParticleEditor() = default;
	~ParticleEditor() = default;

	void Initialize(ParticleManager* particleManager,ParticleCommon* particleCommon);
	void Update();
	void UpdateImGui();
	void Finalize();
	void Draw();

	//Preset Management
	void SavePreset(const std::string& presetName);
	void LoadPreset(const std::string& presetName);
	void DeletePreset(const std::string& presetName);
	void LoadDefaultPreset();
	void RefreshPresetList();

private:

	void DrawParticleAttributesEditor();
	void DrawEmitterControls();
	void DrawPreviewSettings();
	void DrawPresetManager();

private:

	// パーティクルマネージャー
	ParticleManager* particleManager_ = nullptr;
	// パーティクル共通情報
	ParticleCommon* particleCommon_ = nullptr;
	//エディター専用エミッター
	std::unique_ptr<ParticleEmitter> previewEmitter_;

	//エディター設定
	ParticleAttributes currentAttributes_;
	std::string currentGroupName_;
	std::string selectedPresetName_;

	//プレビュー設定
	EulerTransform emitterTransform_;
	uint32_t emitCount_ = 10;
	float emitFrequency_ = 0.1f;
	bool autoEmit_ = false;

	// プリセット管理
	std::vector<std::string> presetNames_;
	std::map<std::string, ParticleAttributes> presets_;
};