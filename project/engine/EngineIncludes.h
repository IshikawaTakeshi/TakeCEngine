#pragma once
#pragma warning(push)
//C4023の警告を見なかったことにする
#pragma warning(disable:4023)
//Include
#include "base/DirectXCommon.h"
#include "base/D3DResourceLeakChecker.h"
#include "base/TextureManager.h"
#include "base/ModelManager.h"
#include "base/SrvManager.h"
#include "base/RtvManager.h"
#include "base/WinApp.h"
#include "base/Particle/ParticleManager.h"
#include "3d/Object3dCommon.h"
#include "3d/Primitive/PrimitiveDrawer.h"
#include "3d/Particle/ParticleCommon.h"
#include "2d/SpriteCommon.h"
#include "2d/WireFrame.h"
#include "Animation/Animator.h"
#include "audio/Audio.h"
#include "io/Input.h"
#include "camera/CameraManager.h"
#include "primitive/Sphere.h"
#include "PostEffect/PostEffectManager.h"
#include "scene/SceneManager.h"
#include "Utility/Logger.h"
#include "Utility/ResourceBarrier.h"

//DirectXTex
#include <dxgidebug.h>
#pragma comment(lib,"dxguid.lib")

//imgui
#include "ImGuiManager.h"