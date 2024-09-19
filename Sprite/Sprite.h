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

#include "../MyMath/Matrix4x4.h"
#include "../MyMath/Transform.h"
#include "../MyMath/TransformMatrix.h"
#include "../Include/ResourceDataStructure.h"
#include "../Include/Mesh.h"
#include "../Sprite/SpriteCommon.h"

class DirectXCommon;
class Sprite {
public:

	Sprite() = default;
	~Sprite();

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(SpriteCommon* spriteCommon,const std::string& filePath);

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update(int num);

	

	/// <summary>
	/// directionalLightData初期化
	/// </summary>
	void InitializeDirectionalLightData(DirectXCommon* dxCommon);

	/// <summary>
	/// 描画処理
	/// </summary>
	void DrawCall();

	///テクスチャ変更処理
	//void ChangeTexture(const std::string& textureFilePath);
private:

	//頂点データ更新
	void UpdateVertexData();

	//テクスチャサイズをイメージに合わせる
	void AdjustTextureSize();
	
public:
	//================================================================================================
	// getter
	//================================================================================================

	//トランスフォーム取得
	Transform GetTransform() { return transform_; }

	//テクスチャの要素番号取得
	uint32_t GetTextureIndex() { return textureIndex_; }

	//アンカーポイント取得
	const Vector2& GetAnchorPoint() const { return anchorPoint_; }

	const Vector2& GetPosition() const { return position_; }

	const float GetRotation() const { return rotation_; }

	const Vector2& GetSize() const { return size_; }

	const bool GetIsFlipX() const { return isFlipX_; }

	const bool GetIsFlipY() const { return isFlipY_; }

	const Vector2& GetTextureLeftTop() const { return textureLeftTop_; }

	const Vector2& GetTextureSize() const { return textureSize_; }
	
	//================================================================================================
	// setter
	//================================================================================================

	//アンカーポイント設定
	void SetAnchorPoint(const Vector2& anchorPoint) { anchorPoint_ = anchorPoint; }

	void SetPosition(const Vector2& position) { position_ = position; }

	void SetRotation(const float rotation) { rotation_ = rotation; }

	void SetSize(const Vector2& size) { size_ = size; }

	void SetIsFlipX(const bool isFlipX) { isFlipX_ = isFlipX; }

	void SetIsFlipY(const bool isFlipY) { isFlipY_ = isFlipY; }

	void SetTextureLeftTop(const Vector2& textureLeftTop) { textureLeftTop_ = textureLeftTop; }

	void SetTextureSize(const Vector2& textureSize) { textureSize_ = textureSize; }

private:

	//SpriteCommon
	SpriteCommon* spriteCommon_ = nullptr;

	//メッシュ
	Mesh* mesh_ = nullptr;

	//テクスチャ番号
	uint32_t textureIndex_ = 0;

	//sprite用のTransformationMatrix用の頂点リソース
	Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource_;
	//sprite用のTransformationMatrix用の頂点データ
	TransformMatrix* wvpData_ = nullptr;

	//Transform
	Transform transform_{};
	Matrix4x4 worldMatrix_;
	Matrix4x4 viewMatrix_;
	Matrix4x4 projectionMatrix_;
	Matrix4x4 worldViewProjectionMatrix_;

	Vector2 position_ = { 0.0f,0.0f };
	float rotation_ = 0.0f;
	Vector2 size_ = { 100.0f,100.0f };

	
	Vector2 anchorPoint_ = { 0.0f,0.0f }; //アンカーポイント
	bool isFlipX_ = false; //左右フリップ
	bool isFlipY_ = false; //上下フリップ
	bool adjustSwitch_ = false; //テクスチャサイズ調整スイッチ

	Vector2 textureLeftTop_ = { 0.0f,0.0f  }; //テクスチャの左上座標
	Vector2 textureSize_ = { 65.0f,65.0f }; //テクスチャの切り出しサイズ
	
	//平行光源用のリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource_;
	DirectionalLightData* directionalLightData_ = nullptr;
};

