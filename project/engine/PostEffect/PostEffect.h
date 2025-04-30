#pragma once
#include "DirectXCommon.h"
#include <string>
class PsotEffect {
public:
	PsotEffect() = default;
	~PsotEffect();

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(DirectXCommon* dxCommon, const std::string& filePath, const std::string& envMapfilePath = "Resources/images/white.png");

	/// <summary>
	/// 更新処理
	/// </summary>
	void UpdateMaterialImGui();

	/// <summary>
	/// マテリアルリソース初期化
	/// </summary>
	void InitializeMaterialResource(Microsoft::WRL::ComPtr<ID3D12Device> device);
};

