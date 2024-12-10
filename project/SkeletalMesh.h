#pragma once
#include "Mesh.h"

class SkeletalMesh {
public:

	SkeletalMesh() = default;
	~SkeletalMesh();

	/// <summary>
	/// 初期化
	/// </summary>
	void InitializeMesh(DirectXCommon* dxCommon, const std::string& filePath);


private:

	Mesh* mesh_ = nullptr;

};

