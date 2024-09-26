#pragma warning(push)
//C4023の警告を見なかったことにする
#pragma warning(disable:4023)
//Include
#include "D3DResourceLeakChecker/D3DResourceLeakChecker.h"
#include "Include/WinApp.h"
#include "Include/Logger.h"
#include "Include/DirectXCommon.h"
#include "Include/Input.h"
#include "TextureManager.h"

#include "MyMath/Transform.h"
#include "MyMath/MatrixMath.h"
#include "Sprite/SpriteCommon.h"
#include "Sprite/Sprite.h"
#include "Object3dCommon.h"
#include "Object3d.h"
#include "ModelManager.h"
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
	WinApp* winApp = new WinApp();
	winApp->Initialize(L"CG2_06_02");

	//DirectX初期化
	DirectXCommon* directXCommon = new DirectXCommon();
	directXCommon->Initialize(winApp);

	//入力初期化
	Input* input = Input::GetInstance();
	input->Initialize(winApp);


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


	//SpriteCommon初期化
	SpriteCommon* spriteCommon = SpriteCommon::GetInstance();
	spriteCommon->Initialize(directXCommon);

	//Object3dCommon初期化
	Object3dCommon* object3dCommon = Object3dCommon::GetInstance();
	object3dCommon->Initialize(directXCommon);

	//ModelManager初期化
	ModelManager::GetInstance()->Initialize(directXCommon);


	//テクスチャマネージャ初期化
	TextureManager::GetInstance()->Initialize(directXCommon);
	
	//カメラ
	Transform cameraTransform{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,-10.0f} };
	Matrix4x4 cameraMatrix = MatrixMath::MakeAffineMatrix(
		cameraTransform.scale,
		cameraTransform.rotate,
		cameraTransform.translate
	);

	//スプライト
	//std::vector<Sprite*> sprites;
	//for (uint32_t i = 0; i < 5; i++) {
	//	Sprite* sprite = new Sprite();
	//	if (i % 2 == 0) {
	//		sprite->Initialize(spriteCommon, "Resources/uvChecker.png");
	//	} else {
	//		sprite->Initialize(spriteCommon, "Resources/monsterBall.png");
	//	}
	//	sprites.push_back(sprite);
	//}

	//Model読み込み
	ModelManager::GetInstance()->LoadModel("axis.obj");
	ModelManager::GetInstance()->LoadModel("plane.obj");


	//3dObject
	Object3d* object3d = new Object3d();
	object3d->Initialize(object3dCommon,cameraMatrix,"axis.obj");
	object3d->SetScale({0.5f,0.5f,0.5f});
	Object3d* object3d1 = new Object3d();
	object3d1->Initialize(object3dCommon, cameraMatrix,"plane.obj");
	object3d1->SetScale({ 0.5f,0.5f,0.5f });
	

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
	/*	for (int i = 0; i < 5; i++) {
			sprites[i]->Update(i);
		}*/
		object3d->Update(0);
		object3d1->Update(1);

		//========================== 描画処理　==========================//

		
		directXCommon->PreDraw(); //描画前処理
		spriteCommon->PreDraw();  //Spriteの描画前処理

		
		object3dCommon->PreDraw(); //Object3dの描画前処理

		//for (int i = 0; i < 5; i++) {
		//	sprites[i]->DrawCall(); //スプライトの描画
		//}

		
		object3d->Draw(); //3Dオブジェクトの描画
		object3d1->Draw(); //3Dオブジェクトの描画

		//描画後処理
		directXCommon->PostDraw();
	}


	/*==========ImGuiの開放==========*/
#ifdef _DEBUG
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
#endif // DEBUG

	//テクスチャマネージャの開放
	TextureManager::GetInstance()->Finalize();

	//ModelManagerの開放
	ModelManager::GetInstance()->Finalize();

	//SpriteCommonの開放
	spriteCommon->Finalize();

	//Object3dCommonの開放
	object3dCommon->Finalize();

	//入力の開放
	input->Finalize();
	
	//directXCommonの開放
	directXCommon->Finalize();
	delete directXCommon;

	//winAppの開放
	winApp->Finalize();
	delete winApp;
	
	//delete model;
	//delete sphere;
	
	//spritesの開放
	//for (auto& sprite : sprites) {
	//	delete sprite;
	//	sprite = nullptr;
	//}

	//Object3dの開放
	delete object3d;
	object3d = nullptr;
	delete object3d1;
	object3d1 = nullptr;

	delete leakCheck;
	
	return 0;
}



