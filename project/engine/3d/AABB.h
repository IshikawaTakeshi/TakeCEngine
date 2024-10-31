#pragma once
#include "Vector3.h"
#include "Matrix4x4.h"
#include <string>

class DirectXCommon;
class Camera;
class Mesh;
class AABB {
public:

	AABB() = default;
	~AABB() = default;

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(DirectXCommon* dxCommon, Vector3 min,Vector3 max);

	/// <summary>
	/// 毎フレーム処理
	/// </summary>
	void Update();

	void UpdateImGui(std::string name);

	/// <summary>
	/// 描画
	/// </summary>
	void Draw(const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix);

	/// <summary>
	/// AABB同士の当たり判定
	/// </summary>
	bool IsCollision(const AABB& aabb2);

	void SetColor(int color) { color_ = color; }

	void SetMax(Vector3 max) { max_ = max; }

	void SetMin(Vector3 min) { min_ = min; }

private:

	Vector3 min_;
	Vector3 max_;
	int color_;


	DirectXCommon* dxCommon_ = nullptr;

	Camera* camera_ = nullptr;

	//メッシュ
	Mesh* mesh_;

	//filePath
	std::string filePath_;


	//TransformationMatrix用の頂点リソース
	Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource_;
	//TransformationMatrix用の頂点データ
	TransformMatrix* TransformMatrixData_ = nullptr;

	//CPU用のTransform
	Transform transform_{};
	//行列
	Matrix4x4 worldMatrix_;
	Matrix4x4 WVPMatrix_;
};

