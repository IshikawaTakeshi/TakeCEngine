#pragma once
#include "DirectXCommon.h"

class PrimitiveDrawer {
public:
	PrimitiveDrawer() = default;
	~PrimitiveDrawer() = default;
	void Initialize(DirectXCommon* dxCommon);
	void Update();
	void Draw(DirectXCommon* dxCommon);
};

