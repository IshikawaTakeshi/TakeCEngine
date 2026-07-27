#pragma once
struct Material {
	float4 color; //カラー
	float4x4 uvTransform;
	int enableLighting; //Lightingを有効にするフラグ
	float shininess;    //鏡面反射の鋭さ
	float envCoefficient; //環境マップの反射係数
};