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
#include "Animation/Animator.h"
#include "CameraManager.h"
#include "ModelManager.h"
#include "Object3dCommon.h"
#include "Object3d.h"
#include "SpriteCommon.h"
#include "Sprite.h"
#include "primitive/Sphere.h"
#include "SrvManager.h"
#include "scene/SceneManager.h"
#include "3d/Particle/ParticleCommon.h"
#include "base/Particle/ParticleManager.h"
#include "Animation/Animator.h"
#include "3d/Primitive/PrimitiveDrawer.h"
#include "2d/WireFrame.h"

//DirectXTex
#include <dxgidebug.h>
#pragma comment(lib,"dxguid.lib")

//imgui
#include "ImGuiManager.h"