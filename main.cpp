#pragma warning(push)
//C4023の警告を見なかったことにする
#pragma warning(disable:4023)
//Include
#include "D3DResourceLeakChecker/D3DResourceLeakChecker.h"
#include "Include/WinApp.h"
#include "Include/Logger.h"
#include "Include/DirectXCommon.h"
#include "Include/Input.h"

#include "MyMath/Transform.h"
#include "MyMath/MatrixMath.h"
#include "Texture/Texture.h"
#include "Sprite/SpriteCommon.h"
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


//////////////////////////////////////////////////////////
//Windowsアプリでのエントリーポイント(main関数)
//////////////////////////////////////////////////////////

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {


	D3DResourceLeakChecker* leakCheck = new D3DResourceLeakChecker;

	//タイトルバーの名前の入力
	WinApp* winApp = WinApp::GetInstance();
	winApp->Initialize(L"CG2_06_02");

	//DirectX初期化
	DirectXCommon* directXCommon = new DirectXCommon();
	directXCommon->Initialize(winApp);

	//SpriteCommon初期化
	SpriteCommon* spriteCommon = new SpriteCommon();
	spriteCommon->Initialize(directXCommon);

#pragma region ImGui初期化
#ifdef _DEBUG
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(winApp->GetHwnd());
	ImGui_ImplDX12_Init(directXCommon->GetDevice(),
		directXCommon->GetBufferCount(),
		directXCommon->GetRtvFormat(),
		directXCommon->GetSrvHeap(),
		directXCommon->GetSrvHeap()->GetCPUDescriptorHandleForHeapStart(),
		directXCommon->GetSrvHeap()->GetGPUDescriptorHandleForHeapStart()
	);
#endif // DEBUG
#pragma endregion

	//入力初期化
	Input* input = Input::GetInstance();
	input->Initialize(winApp);

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
	//Model* model = new Model();
	//model->Initialize(directXCommon, cameraMatrix, "Resources", "obj_mtl_blend", "axis.obj");
	//model->SetTextureFilePath("./Resources/uvChecker.png");

	//球
	//Sphere* sphere = new Sphere();
	//sphere->Initialize(directXCommon, cameraMatrix, true, "./Resources/uvChecker.png");

	//スプライト
	Sprite* sprite = new Sprite();
	sprite->Initialize(spriteCommon, "./Resources/uvChecker.png");


	/////////////////////////////////////////////////////////////////////////////////////////////
	//										メインループ
	/////////////////////////////////////////////////////////////////////////////////////////////

	//ウィンドウの×ボタンが押されるまでループ

	while (true) {
		//メッセージ処理
		if (winApp->ProcessMessage()) {
			//ウィンドウの×ボタンが押されたらループを抜ける
			break;
		}


		//========================== 更新処理　==========================//


#ifdef _DEBUG
		//ImGui受付開始
		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		//開発用UIの処理。実際に開発用のUIを出す場合はここをゲーム特有の処理に置き換える
		ImGui::ShowDemoWindow();
#endif // DEBUG

		//入力更新
		input->Update();


		//モデルの更新処理
		//model->Update();

		//球
		//sphere->Update();



		//スプライト
		sprite->Update();


		//========================== 描画処理　==========================//

		//描画前処理
		directXCommon->PreDraw();
		//Spriteの描画前処理
		spriteCommon->PreDraw();

		//if (input->TriggerKey(DIK_SPACE)) {
		//	//モデルのテクスチャを変更
		//	sphere->SetTexture(texture2);
		//}
		//if (input->PushKey(DIK_B)) {
		//	//モデルのテクスチャを変更
		//	sphere->SetTexture(texture1);
		//}
		//model->DrawCall(directXCommon);
		//sphere->DrawCall(directXCommon);
		sprite->DrawCall(directXCommon);

		//描画後処理
		directXCommon->PostDraw();
	}


	/*==========ImGuiの開放==========*/
#ifdef _DEBUG
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
#endif // DEBUG


	directXCommon->Finalize();
	winApp->Finalize();
	
	//delete model;
	//delete sphere;
	delete texture1;
	delete texture2;
	delete sprite;

	delete leakCheck;
	
	return 0;
}



