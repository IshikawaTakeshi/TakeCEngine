#pragma warning(push)
//C4023の警告を見なかったことにする
#pragma warning(disable:4023)
//Include
#include "WinApp.h"
#include "Logger.h"
#include "DirectXCommon.h"
#include "Transform.h"
#include "Texture.h"
#include "MyMath/MatrixMath.h"
#include "Sprite.h"
#include "Triangle.h"

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
	winApp->Initialize(L"CG2_評価課題1");

	//DirectX初期化
	DirectXCommon* directXCommon = DirectXCommon::GetInstance();
	directXCommon->Initialize();

	//テクスチャ初期化
	Texture* texture = new Texture();

	texture->Initialize(directXCommon, "./Resources/uvChecker.png");
	
	//カメラ
	Transform cameraTransform{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,-5.0f} };
	Matrix4x4 cameraMatrix = MatrixMath::MakeAffineMatrix(
		cameraTransform.scale,
		cameraTransform.rotate,
		cameraTransform.translate
	);

	//三角形1
	Triangle* triangle1 = new Triangle();
	triangle1->Initialize(directXCommon,cameraMatrix);
	directXCommon->SetVertexBufferView(triangle1->GetVertexBufferView());
	directXCommon->SetWvpResource(triangle1->GetWvpResource());

	//三角形2
	Triangle* triangle2 = new Triangle();
	triangle2->Initialize(directXCommon, cameraMatrix);
	directXCommon->SetVertexBufferView(triangle2->GetVertexBufferView());
	directXCommon->SetWvpResource(triangle2->GetWvpResource());

	//スプライト
	Sprite* sprite = new Sprite();
	sprite->Initialize(directXCommon);
	directXCommon->SetVertexBufferViewSprite(sprite->GetVertexBufferView());
	directXCommon->SetTransformationMatrixResourceSprite(sprite->GetTransformationMatrixResource());
	

	
	//////////////////////////////////////////////////////////
	//メインループ
	//////////////////////////////////////////////////////////
	
	//ウィンドウの×ボタンが押されるまでループ
	while (winApp->ProcessMessage() == 0) {
		
		//描画前処理
		directXCommon->PreDraw();

		triangle1->InitializeCommandList(directXCommon, texture);
		triangle2->InitializeCommandList(directXCommon, texture);
		sprite->InitializeCommandList(directXCommon, texture);

		//三角形
		triangle1->Update(
#ifdef _DEBUG
		1
#endif // _DEBUG

		);
		triangle2->Update(
#ifdef _DEBUG
			2
#endif // _DEBUG
		);

		//スプライト
		sprite->Update();


		//描画後処理
		directXCommon->PostDraw();
	}

	directXCommon->Finalize();
	winApp->Finalize();
	texture->Finalize();
	

	leakCheck.~D3DResourceLeakChecker();

	return 0;
}




