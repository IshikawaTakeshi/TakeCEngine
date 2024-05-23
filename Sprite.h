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

class Sprite {
public:

	Sprite() = default;
	~Sprite() = default;

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

	/// <summary>
	/// 終了・開放処理
	/// </summary>
	void Finalize();

private:

	//Sprite用の頂点リソース
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResourceSprite_;
	//頂点バッファビュー
	D3D12_VERTEX_BUFFER_VIEW vertexBufferViewSprite_{};

};

