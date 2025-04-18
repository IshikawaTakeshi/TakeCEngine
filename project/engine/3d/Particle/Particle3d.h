#pragma once
#include "3d/Particle/BaseParticleGroup.h"

//加速フィールド
struct AccelerationField {
	Vector3 acceleration_; //加速度
	Vector3 position_;     //位置
	AABB aabb_;            //当たり判定
};

class ParticleCommon;
class Particle3d : public BaseParticleGroup {
public:

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

	void UpdateImGui();

	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

	void DrawPrimitive();

	/// <summary>
	/// パーティクルの生成
	/// </summary>
	Particle MakeNewParticle(std::mt19937& randomEngine, const Vector3& translate);

	/// <summary>
	/// パーティクルの発生
	/// </summary>
	std::list<Particle> Emit(const Vector3& emitterPos, uint32_t particleCount);

	bool IsCollision(const AABB& aabb, const Vector3& point);

	void SpliceParticles(std::list<Particle> particles);

	void EmitMove(std::list<Particle>::iterator particleIterator);

public: //getter

public: //setter

	void SetModel(const std::string& filePath);
	//void SetModelTexture(const std::string& texturefilePath);
	void SetCamera(Camera* camera) { camera_ = camera; }

private: // privateメンバ変数

	//Particleの総数
	static const uint32_t kNumMaxInstance_ = 10240; 
	//1フレームの時間
	const float kDeltaTime_ = 1.0f / 60.0f;
	//描画するインスタンス数
	uint32_t numInstance_ = 0; 
	//Particleの配列
	std::list<Particle> particles_; 

	//加速フィールド
	AccelerationField accelerationField_;
	//パーティクルの属性
	ParticleAttributes particleAttributes_;
	
private:

	//ParticleCommon
	ParticleCommon* particleCommon_ = nullptr;

	//Camera
	Camera* camera_ = nullptr;

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