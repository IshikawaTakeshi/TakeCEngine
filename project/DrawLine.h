#pragma once
#include "Vector3.h"
#include "Matrix4x4.h"
#include "TransformMatrix.h"
#include "Transform.h"
#include "ResourceDataStructure.h"
#include "Mesh/Mesh.h"
#include <stdint.h>
#include <string>
#include <d3d12.h>
#include <wrl.h>

class DrawLine {
public:


private:

	//メッシュ
	std::unique_ptr<Mesh> mesh_;

	//filePath
	std::string filePath_;

	//TransformationMatrix用の頂点リソース
	ComPtr<ID3D12Resource> wvpResource_;
	//TransformationMatrix用の頂点データ
	TransformMatrix* TransformMatrixData_ = nullptr;

	//instancingResource
	ComPtr<ID3D12Resource> instancingResource_;

	//CPU用のTransform
	EulerTransform transform_{};
	//行列
	Matrix4x4 worldMatrix_;
	Matrix4x4 viewMatrix_;
	Matrix4x4 projectionMatrix_;
	Matrix4x4 worldViewProjectionMatrix_;

};