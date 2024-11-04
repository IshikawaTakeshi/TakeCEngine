#pragma once
#include "ResourceDataStructure.h"
#include "Transform.h"
#include "TransformMatrix.h"
#include "DirectXCommon.h"

#include <d3d12.h>
#include <wrl.h>
#include <random>
#include <list>

//Particle1個分のデータ
struct Particle {
	Transform transforms_;  //位置
	Vector3 velocity_; 	    //速度
	Vector4 color_;         //色
	float lifeTime_;        //寿命
	float currentTime_;     //経過時間
};

//エミッター
struct Emitter {
	Transform transforms_;   //エミッターの位置
	uint32_t particleCount_; //発生するParticleの数
	float frequency_;        //発生頻度
	float frequencyTime_;    //経過時間
};

struct AABB {
	Vector3 min_;
	Vector3 max_;
};

//加速フィールド
struct AccelerationField {
	Vector3 acceleration_; //加速度
	Vector3 position_;     //位置
	AABB aabb_;            //当たり判定
};

class DirectXCommon;
class Camera;
class Model;
class PSO;
class ParticleCommon;
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

	Particle MakeNewParticle(std::mt19937& randomEngine, const Vector3& translate);


	std::list<Particle> Emit(const Emitter& emitter, std::mt19937& randomEngine);

	bool IsCollision(const AABB& aabb, const Vector3& point);

public: //getter

public: //setter

	void SetModel(const std::string& filePath);
	void SetCamera(Camera* camera) { camera_ = camera; }

private: // privateメンバ変数

	static const uint32_t kNumMaxInstance_ = 100; //Particleの総数
	const float kDeltaTime_ = 1.0f / 60.0f; //1フレームの時間
	uint32_t numInstance_ = 0; //描画するインスタンス数
	std::list<Particle> particles_; //Particleの配列
	bool isBillboard_ = false;
	Emitter emitter_;
	AccelerationField accelerationField_;
	
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