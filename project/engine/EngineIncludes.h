#pragma once
#pragma warning(push)
//C4023の警告を見なかったことにする
#pragma warning(disable:4023)
//Include
#include "D3DResourceLeakChecker.h"
#include "WinApp.h"
#include "Logger.h"
#include "DirectXCommon.h"
#include "Input.h"
#include "TextureManager.h"
#include "Audio.h"
#include "Camera.h"
#include "CameraManager.h"
#include "MatrixMath.h"
#include "Model.h"
#include "ModelManager.h"
#include "Object3dCommon.h"
#include "Object3d.h"
#include "SpriteCommon.h"
#include "Sprite.h"
#include "Sphere.h"
#include "SrvManager.h"
#include "scene/SceneManager.h"
#include "Transform.h"
#include "Triangle.h"
#include "Particle3d.h"
#include "ParticleCommon.h"
#include "ParticleManager.h"
#include "Animation/Animator.h"

//DirectXTex
#include <dxgidebug.h>
#pragma comment(lib,"dxguid.lib")

//imgui
#include "ImGuiManager.h"