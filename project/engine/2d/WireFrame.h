#pragma once
#include "Vector3.h"
#include "Matrix4x4.h"
#include "TransformMatrix.h"
#include "Transform.h"
#include "ResourceDataStructure.h"
#include "Mesh/Mesh.h"
#include "PipelineStateObject.h"
#include "DirectXCommon.h"
#include "math/AABB.h"
#include "math/OBB.h"
#include <stdint.h>
#include <string>
#include <d3d12.h>
#include <wrl.h>

//線用の頂点データ構造体
struct WireFrameVertexData {
	Vector3 position;
	Vector4 color;
};

//線用の変換行列データ構造体
struct WireFrameTransFormMatrixData {
	Matrix4x4 WVP;
};

//線データ構造体
struct LineData {
	ComPtr<ID3D12Resource> vertexBuffer_;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferViews_;
	WireFrameVertexData* vertexData_ = nullptr;
};

//============================================================================
// WireFrame class
//============================================================================
class WireFrame {
public:

	//=========================================================================
	// functions
	//=========================================================================
	
	//初期化
	void Initialize(DirectXCommon* directXCommon);
	//更新処理
	void Update();
	//線の描画
	void DrawLine(const Vector3& start, const Vector3& end,const Vector4& color);
	//AABBの描画
	void DrawOBB(const OBB& obb, const Vector4& color);
	//球の描画
	void DrawSphere(const Vector3& center, float radius, const Vector4& color);
	//グリッド平面の描画
	void DrawGridGround(const Vector3& center, const Vector3& size,uint32_t division);
	//グリッドボックスの描画
	void DrawGridBox(const AABB& aabb, uint32_t division);
	//描画処理
	void Draw();
	//リセット
	void Reset();
	//終了・開放処理
	void Finalize();

private:

	//頂点データ作成
	void CreateVertexData();
	//球の頂点データ計算
	void CalculateSphereVertexData();
	//グリッド線の描画
	void DrawGridLines(const Vector3& start, const Vector3& end, const Vector3& offset, uint32_t division, const Vector4& color);

private:

	//dxCommon
	DirectXCommon* dxCommon_ = nullptr;
	//パイプラインステートオブジェクト
	std::unique_ptr<PSO> pso_ = nullptr;
	ComPtr<ID3D12RootSignature> rootSignature_ = nullptr;
	
	//線データ
	LineData* lineData_ = nullptr;
	//球の頂点データ
	std::vector<Vector3> spheres_;

	//TransformationMatrix用の頂点リソース
	ComPtr<ID3D12Resource> wvpResource_;
	//TransformationMatrix用の頂点データ
	WireFrameTransFormMatrixData* TransformMatrixData_ = nullptr;

	//instancingResource
	ComPtr<ID3D12Resource> instancingResource_;
	
	//CPU用のTransform
	EulerTransform transform_{};
	//行列
	Matrix4x4 worldMatrix_;
	Matrix4x4 viewMatrix_;
	Matrix4x4 projectionMatrix_;
	Matrix4x4 worldViewProjectionMatrix_;

	//線のインデックス
	uint32_t lineIndex_ = 0;
	//最大線本数
	const uint32_t kMaxLineCount_ = 100000;
	//線の頂点数
	const uint32_t kLineVertexCount_ = 2;

};