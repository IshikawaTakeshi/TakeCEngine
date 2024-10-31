#pragma once
#include "ResourceDataStructure.h"
#include "Transform.h"
#include "TransformMatrix.h"
#include "DirectXCommon.h"
#include <d3d12.h>
#include <wrl.h>
#include <random>
#include <list>

class DirectXCommon;
class Camera;
class Model;
class PSO;
class ParticleCommon;
class SrvManager;
class Particle3d {
public:

	//Particle1個分のデータ
	struct Particle {
		Transform transforms_;
		Vector3 velocity_;
		Vector4 color_;
		float lifeTime_;
		float currentTime_;
	};

	//エイリアステンプレート
	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;


	Particle3d() = default;
	~Particle3d();


	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(ParticleCommon* particleCommon,const std::string& filePath);

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

	Particle MakeNewParticle(std::mt19937& randomEngine);

public: //getter

public: //setter

	void SetModel(const std::string& filePath);
	void SetCamera(Camera* camera) { camera_ = camera; }

private: // privateメンバ変数

	const float kDeltaTime_ = 1.0f / 60.0f; //1フレームの時間

	static const uint32_t kNumMaxInstance_ = 10; //Particleの総数

	uint32_t numInstance_ = 0; //描画するインスタンス数

	//Particleの配列
	std::list<Particle> particles_;

	bool isBillboard_ = false;


private:

	//ParticleCommon
	ParticleCommon* particleCommon_ = nullptr;
	//RootSignature
	ComPtr<ID3D12RootSignature> rootSignature_ = nullptr;
	//PSO
	PSO* pso_ = nullptr;
	//モデル
	Model* model_ = nullptr;
	//TransformationMatrix用のデータ
	ParticleForGPU* instancingData_ = nullptr;
	//Matrix
	Matrix4x4 worldMatrix_;
	Matrix4x4 WVPMatrix_;
	//Matrix4x4 billboardMatrix_;
	//Camera
	Camera* camera_ = nullptr;
	uint32_t useSrvIndex_ = 0;
	ComPtr<ID3D12Resource> instancingResource_;
	D3D12_CPU_DESCRIPTOR_HANDLE instancingSrvHandleCPU; //CPUディスクリプタハンドル
	D3D12_GPU_DESCRIPTOR_HANDLE instancingSrvHandleGPU; //GPUディスクリプタハンドル
};