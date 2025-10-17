#pragma once
struct Material {
	float4 color; //カラー
	float4x4 uvTransform;
	int enableLighting; //Lightingを有効にするフラグ
	float shininess;
	float envCoefficient; //環境マップの反射係数
};