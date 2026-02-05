#pragma once
#include "engine/3d/Particle/ParticleCommon.h"
#include "engine/3d/Particle/EffectGroup.h"
#include "math/Vector3.h"
#include <memory>
#include <vector>
#include <string>


namespace TakeC {

	//============================================================================
	// EditorState enum - エディタの状態
	//============================================================================
	enum class EditorState {
		Idle,           // アイドル状態
		Playing,        // プレビュー再生中
		Paused,         // 一時停止中
		EditingEmitter  // エミッター編集中
	};

	//============================================================================
	// EffectEditor class
	//============================================================================

	class EffectEditor {
	public:
		EffectEditor() = default;
		~EffectEditor() = default;

		//======================================================================
		// 基本機能
		//======================================================================

		/// <summary>
		/// 初期化
		/// </summary>
		void Initialize(ParticleCommon* particleCommon);

		/// <summary>
		/// 終了処理
		/// </summary>
		void Finalize();

		/// <summary>
		/// 更新処理
		/// </summary>
		void Update();

		/// <summary>
		/// ImGui更新処理
		/// </summary>
		void UpdateImGui();

		/// <summary>
		/// 描画処理
		/// </summary>
		void Draw();

	private:

		//======================================================================
		// ImGUIウィンドウ
		//======================================================================

		/// <summary>
		/// メインウィンドウ
		/// </summary>
		void DrawMainWindow();

		/// <summary>
		/// エフェクトリストタブ
		/// </summary>
		void DrawEffectListTab();

		/// <summary>
		/// エフェクト設定タブ
		/// </summary>
		void DrawEffectSettingsTab();

		/// <summary>
		/// エミッターリストタブ
		/// </summary>
		void DrawEmitterListTab();

		/// <summary>
		/// エミッター設定タブ
		/// </summary>
		void DrawEmitterSettingsTab();

		/// <summary>
		/// プレビューコントロールタブ
		/// </summary>
		void DrawPreviewControlTab();

		/// <summary>
		/// ファイル操作タブ
		/// </summary>
		void DrawFileOperationTab();

		//======================================================================
		// エフェクト操作
		//======================================================================

		/// 新しいエフェクトを作成
		void CreateNewEffect();
		/// エフェクトを複製
		void DuplicateEffect(int index);
		/// エフェクトを削除
		void DeleteEffect(int index);
		/// エフェクトを選択
		void SelectEffect(int index);
		/// 保存済みエフェクトを読み込み
		void LoadSavedEffects();

		//======================================================================
		// エミッター操作
		//======================================================================

		/// 新しいエミッターを追加
		void AddEmitter();
		/// エミッターを削除
		void RemoveEmitter(int index);
		/// エミッターを複製
		void DuplicateEmitter(int index);
		/// エミッターを並び替え（上へ）
		void MoveEmitterUp(int index);
		/// エミッターを並び替え（下へ）
		void MoveEmitterDown(int index);
		/// エミッターを選択
		void SelectEmitter(int index);

		//======================================================================
		// プレビュー操作
		//======================================================================

		/// プレビューを再生
		void PlayPreview();
		/// プレビューを停止
		void StopPreview();
		/// プレビューを一時停止
		void PausePreview();
		/// プレビューをリセット
		void ResetPreview();

		//======================================================================
		// ファイル操作
		//======================================================================

		/// エフェクトをJSONに保存
		void SaveEffectToJson(const std::string& filePath);
		/// JSONからエフェクトを読み込み
		void LoadEffectFromJson(const std::string& filePath);
		/// 利用可能なパーティクルプリセットを読み込み
		void LoadAvailablePresets();

		//======================================================================
		// ユーティリティ
		//======================================================================

		/// プレビュー位置にギズモを描画
		void DrawPreviewGizmo();
		/// 選択中のエミッターのギズモを描画
		void DrawEmitterGizmo();

		/// エフェクトグループの設定を更新
		void UpdateEffectGroupConfig();
		/// エフェクトグループを再構築
		void RebuildEffectGroup();

		//======================================================================
		// メンバ変数
		//======================================================================

		// ParticleCommon
		ParticleCommon* particleCommon_ = nullptr;

		// 編集中のエフェクト設定
		EffectGroupConfig currentConfig_;

		// プレビュー用エフェクトグループ
		std::unique_ptr<TakeC::EffectGroup> previewEffect_;

		// 保存済みエフェクトリスト
		std::vector<EffectGroupConfig> savedEffects_;

		// 利用可能なパーティクルプリセット一覧
		std::vector<std::string> availablePresets_;

		// 選択状態
		int selectedEffectIndex_ = -1;
		int selectedEmitterIndex_ = -1;

		// エディタ状態
		EditorState editorState_ = EditorState::Idle;

		// プレビュー設定
		Vector3 previewPosition_ = { 0.0f, 0.0f, 0.0f };
		Vector3 previewRotation_ = { 0.0f, 0.0f, 0.0f };
		Vector3 previewScale_ = { 1.0f, 1.0f, 1.0f };

		// UI状態
		// タブ化に伴い表示フラグは削除

		// 入力バッファ
		char effectNameBuffer_[256] = "NewEffect";
		char emitterNameBuffer_[256] = "NewEmitter";
		int selectedPresetIndex_ = 0;

		// デルタタイム
		float deltaTime_;
	};
}