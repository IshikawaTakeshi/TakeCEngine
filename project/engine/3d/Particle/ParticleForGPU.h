#pragma once

//==========================================
// ParticleForGPU.h
//==========================================

#include "Vector3.h"
#include "Vector4.h"


//パーティクル用の行列,色データ
struct ParticleForGPU {
	Vector3 translate;
	Vector3 rotate;
	Vector3 scale;
	Vector3 velocity;
	Vector4 color;
	float lifeTime;
	float currentTime;
};