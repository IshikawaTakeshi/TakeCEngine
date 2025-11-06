#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>

#include <dxcapi.h>

#include <wrl.h>
#include <cassert>
#include <string>
#include <vector>
#include <iostream>
#include <memory>

#include "Matrix4x4.h"
#include "Transform.h"
#include "TransformMatrix.h"
#include "ResourceDataStructure.h"
#include "Mesh/Mesh.h"
#include "SpriteCommon.h"
#include "engine/Animation/SpriteAnimation.h"

//前方宣言
class DirectXCommon;

//============================================================================
// Sprite class
//============================================================================
class Sprite {
public:

	Sprite() = default;
	~Sprite() = default;

	//========================================================================
	// functions
	//========================================================================

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(SpriteCommon* spriteCommon,const std::string& filePath);

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update();

	void UpdateImGui([[maybe_unused]]const std::string& name);

	/// <summary>
	/// 描画処理
	/// </summary>
	void Draw();

	//テクスチャサイズをイメージに合わせる
	void AdjustTextureSize();
private:

	//頂点データ更新
	void UpdateVertexData();
	//サイズを相対座標にセット
	void SetSizeRelative();

public:
	//========================================================================
	// accessors
	//========================================================================

	//----- getter ---------------------------

	//トランスフォーム取得
	EulerTransform GetTransform() { return transform_; }
	//メッシュ取得
	const std::unique_ptr<Mesh>& GetMesh() const { return mesh_; }

	//SpriteAnimator取得
	SpriteAnimator* Animation() const { return spriteAnimator_.get(); }
	//アンカーポイント取得
	const Vector2& GetAnchorPoint() const { return anchorPoint_; }
	//座標取得
	const Vector2& GetTranslate() const { return position_; }
	//回転取得
	const float GetRotate() const { return rotation_; }
	//サイズ取得
	const Vector2& GetSize() const { return size_; }
	//左右フリップ取得
	const bool GetIsFlipX() const { return isFlipX_; }
	//上下フリップ取得
	const bool GetIsFlipY() const { return isFlipY_; }
	//テクスチャの左上座標取得
	const Vector2& GetTextureLeftTop() const { return textureLeftTop_; }
	//テクスチャの切り出しサイズ取得
	const Vector2& GetTextureSize() const { return textureSize_; }
	
	//----- setter ---------------------------
	
	//アンカーポイント設定
	void SetAnchorPoint(const Vector2& anchorPoint) { anchorPoint_ = anchorPoint; }
	//座標設定
	void SetPosition(const Vector2& position) { position_ = position; }
	//回転設定
	void SetRotate(const float rotation) { rotation_ = rotation; }
	//サイズ設定
	void SetSize(const Vector2& size) { size_ = size; }
	//左右フリップ設定
	void SetIsFlipX(const bool isFlipX) { isFlipX_ = isFlipX; }
	//上下フリップ設定
	void SetIsFlipY(const bool isFlipY) { isFlipY_ = isFlipY; }
	//テクスチャの左上座標設定
	void SetTextureLeftTop(const Vector2& textureLeftTop) { textureLeftTop_ = textureLeftTop; }
	//テクスチャの切り出しサイズ設定
	void SetTextureSize(const Vector2& textureSize) { textureSize_ = textureSize; }
	//マテリアルカラー設定
	void SetMaterialColor(const Vector4& color) { mesh_->GetMaterial()->SetMaterialColor(color); }
	//ファイルパス設定
	void SetFilePath(const std::string& filePath);

private:

	//SpriteCommon
	SpriteCommon* spriteCommon_ = nullptr;

	//メッシュ
	std::unique_ptr<Mesh> mesh_ = nullptr;

	//filePath
	std::string filePath_;
	//SpriteAnimator
	std::unique_ptr<SpriteAnimator> spriteAnimator_{};

	//sprite用のTransformationMatrix用の頂点リソース
	Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource_;
	//sprite用のTransformationMatrix用の頂点データ
	TransformMatrix* wvpData_ = nullptr;

	//Transform
	EulerTransform transform_{};
	Matrix4x4 worldMatrix_;
	Matrix4x4 viewMatrix_;
	Matrix4x4 projectionMatrix_;
	Matrix4x4 worldViewProjectionMatrix_;

	Vector2 position_ = { 0.0f,0.0f };
	float rotation_ = 0.0f;
	Vector2 size_ = { 400.0f,200.0f };

	
	Vector2 anchorPoint_ = { 0.0f,0.0f }; //アンカーポイント
	bool isFlipX_ = false; //左右フリップ
	bool isFlipY_ = false; //上下フリップ
	bool adjustSwitch_ = false; //テクスチャサイズ調整スイッチ
	bool firstUpdate_ = true; //初回更新フラグ

	Vector2 textureLeftTop_ = { 0.0f,0.0f  }; //テクスチャの左上座標
	Vector2 textureSize_ = { 100.0f,100.0f }; //テクスチャの切り出しサイズ
};