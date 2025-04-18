#pragma once
#include "ResourceDataStructure.h"
#include "Transform.h"
#include "TransformMatrix.h"
#include "DirectXCommon.h"
#include "math/AABB.h"
#include "3d/Model.h"
#include "camera/CameraManager.h"

#include <d3d12.h>
#include <wrl.h>
#include <random>
#include <numbers>
#include <list>

//Particle1個分のデータ
struct Particle {
	EulerTransform transforms_;  //位置
	Vector3 velocity_; 	    //速度
	Vector4 color_;         //色
	float lifeTime_;        //寿命
	float currentTime_;     //経過時間
};

struct AttributeRange {
	float min;
	float max;
};

// パーティクルの属性を保持する構造体
struct ParticleAttributes {
	Vector3 scale = { 1.0f,1.0f,1.0f };
	Vector3 color = { 1.0f,1.0f,1.0f };
	AttributeRange scaleRange = { 0.1f,3.0f };
	AttributeRange rotateRange = { -std::numbers::pi_v<float>, std::numbers::pi_v<float> };
	AttributeRange positionRange = {-1.0f, 1.0f};
	AttributeRange velocityRange = { -1.0f,1.0f };
	AttributeRange colorRange = { 0.0f,1.0f };
	AttributeRange lifetimeRange = { 1.0f,3.0f };
	//色を編集するかどうか
	bool editColor = false;
};

class ParticleCommon;
class BaseParticleGroup {
public:
	BaseParticleGroup() = default;
	~BaseParticleGroup() = default;
	virtual void Initialize(ParticleCommon* particleCommon);
	virtual void Finalize();

	void Update();
	void Draw(PSO* pso, UINT instanceCount);


protected:

	//Particleの総数
	static const uint32_t kNumMaxInstance_ = 10240; 
	//1フレームの時間
	const float kDeltaTime_ = 1.0f / 60.0f;
	//描画するインスタンス数
	uint32_t numInstance_ = 0; 
	//Particleの配列
	std::list<Particle> particles_; 

	//加速フィールド
	//AccelerationField accelerationField_;
	//パーティクルの属性
	ParticleAttributes particleAttributes_;

protected:

	//ParticleCommon
	ParticleCommon* particleCommon_ = nullptr;

	//モデル
	Model* model_ = nullptr;

	//instancing用のデータ
	ParticleForGPU* particleData_ = nullptr;
	PerView* perViewData_ = nullptr;
	uint32_t particleSrvIndex_ = 0;
	ComPtr<ID3D12Resource> particleResource_;
	ComPtr<ID3D12Resource> perViewResource_;
	//Matrix
	Matrix4x4 worldMatrix_;
	Matrix4x4 WVPMatrix_;
};

