#pragma once
#include "engine/math/Quaternion.h"

//==========================================
// ParticleForGPU.h
//==========================================

#include "Vector3.h"
#include "Vector4.h"


//パーティクル用の行列,色データ
/// <summary>
/// ParticleForGPUに必要な値をまとめて保持する構造体です。
/// </summary>
struct ParticleForGPU {
	Vector3 translate;
	Quaternion rotate;
	Vector3 scale;
	Vector3 velocity;
	Vector4 color;
	float lifeTime;
	float currentTime;
};