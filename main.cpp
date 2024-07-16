#pragma warning(push)
//C4023の警告を見なかったことにする
#pragma warning(disable:4023)
//Include
#include "Include/WinApp.h"
#include "Include/Logger.h"
#include "Include/DirectXCommon.h"
#include "MyMath/Transform.h"
#include "Texture/Texture.h"
#include "MyMath/MatrixMath.h"
#include "Sprite/Sprite.h"
#include "Triangle/Triangle.h"
#include "Sphere/Sphere.h"
#include "ModelData/Model.h"
#include <dxgidebug.h>
#pragma comment(lib,"dxguid.lib")

#pragma region imgui
#ifdef _DEBUG
#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
	HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif // DEBUG
#pragma endregion

struct D3DResourceLeakChecker {
	~D3DResourceLeakChecker() {
		Microsoft::WRL::ComPtr<IDXGIDebug1> debug;
		if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&debug)))) {
			debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
			debug->ReportLiveObjects(DXGI_DEBUG_APP, DXGI_DEBUG_RLO_ALL);
			debug->ReportLiveObjects(DXGI_DEBUG_D3D12, DXGI_DEBUG_RLO_ALL);
		}
	}
};

//////////////////////////////////////////////////////////
//Windowsアプリでのエントリーポイント(main関数)
//////////////////////////////////////////////////////////

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	CoInitializeEx(0, COINIT_MULTITHREADED);

	D3DResourceLeakChecker leakCheck;

	//タイトルバーの名前の入力
	WinApp* winApp = WinApp::GetInstance();
	winApp->Initialize(L"CG2_06_02");

	//DirectX初期化
	DirectXCommon* directXCommon = DirectXCommon::GetInstance();
	directXCommon->Initialize();

	//テクスチャ初期化
	Texture* texture1 = new Texture();
	texture1->Initialize(1, directXCommon, "./Resources/uvChecker.png");
	//テクスチャ初期化
	Texture* texture2 = new Texture();
	texture2->Initialize(2, directXCommon, "./Resources/monsterBall.png");

	//カメラ
	Transform cameraTransform{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,-10.0f} };
	Matrix4x4 cameraMatrix = MatrixMath::MakeAffineMatrix(
		cameraTransform.scale,
		cameraTransform.rotate,
		cameraTransform.translate
	);

	//モデル
	Model* model = new Model();
	model->Initialize(directXCommon,cameraMatrix,"Resources/obj_mtl_blend","axis.obj");

	//球
	Sphere* sphere = new Sphere();
	sphere->Initialize(directXCommon, cameraMatrix);

	//スプライト
	//Sprite* sprite = new Sprite();
	//sprite->Initialize(directXCommon);

	
	//////////////////////////////////////////////////////////
	//メインループ
	//////////////////////////////////////////////////////////
	
	//ウィンドウの×ボタンが押されるまでループ
	while (winApp->ProcessMessage() == 0) {
		
		//描画前処理
		directXCommon->PreDraw();

		model->Update();

		//球
		//sphere->Update();

		//スプライト
		//sprite->Update();


		model->DrawCall(directXCommon,texture1);
		sphere->DrawCall(directXCommon, texture1, texture2);
		//sprite->DrawCall(directXCommon, texture1);

		//描画後処理
		directXCommon->PostDraw();
	}

	delete texture1;
	delete texture2;
	delete model;
	delete sphere;
	//delete sprite;
	
	winApp->Finalize();
	directXCommon->Finalize();
	

	leakCheck.~D3DResourceLeakChecker();

	return 0;
}




