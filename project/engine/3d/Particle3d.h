#pragma once
#include "ResourceDataStructure.h"
#include "Transform.h"
#include "TransformMatrix.h"
#include "DirectXCommon.h"
#include <d3d12.h>
#include <wrl.h>

class DirectXCommon;
class Camera;
class Model;
class PSO;
class SrvManager;
class Particle3d {
public:

	//エイリアステンプレート
	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;


	Particle3d() = default;
	~Particle3d();


	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(const std::string& filePath);

	/// <summary>
	/// 終了処理
	/// </summary>
	void Update();

#ifdef _DEBUG
	void UpdateImGui();
#endif // _DEBUG

	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

public: //getter

public: //setter

	void SetModel(const std::string& filePath);
	void SetCamera(Camera* camera) { camera_ = camera; }

private: // privateメンバ変数

	//RootSignature
	ComPtr<ID3D12RootSignature> rootSignature_ = nullptr;

	//PSO
	PSO* pso_ = nullptr;

	//モデル
	Model* model_ = nullptr;

	//TransformationMatrix用のリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource_;
	//TransformationMatrix用のデータ
	TransformMatrix* instancingData_ = nullptr;

	static const uint32_t kNumInstance_ = 10;
	//Transforms
	Transform transforms_[kNumInstance_];
	//WorldMatrix
	Matrix4x4 worldMatrix_;
	Matrix4x4 WVPMatrix_;
	//Camera
	Camera* camera_ = nullptr;

	ComPtr<ID3D12Resource> instancingResource_;
	D3D12_CPU_DESCRIPTOR_HANDLE instancingSrvHandleCPU; //CPUディスクリプタハンドル
	D3D12_GPU_DESCRIPTOR_HANDLE instancingSrvHandleGPU; //GPUディスクリプタハンドル
};

