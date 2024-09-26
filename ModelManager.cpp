#include "ModelManager.h"
#include "Modeldata/Model.h"
#include "ModelCommon.h"
#include "Include/DirectXCommon.h"

ModelManager* ModelManager::instance_ = nullptr;

ModelManager* ModelManager::GetInstance() {
  
	if (instance_ == nullptr) {
		instance_ = new ModelManager();
	}
	return instance_;
}

void ModelManager::Initialize(DirectXCommon* dxCommon) {

	//ModelCommon初期化
	modelCommon_ = ModelCommon::GetInstance();
	modelCommon_->Initialize(dxCommon);
}

void ModelManager::Finalize() {
	modelCommon_->Finalize();
	
	delete instance_;
	instance_ = nullptr;
}

void ModelManager::LoadModel(const std::string& filePath) {

	//読み込み済みモデルの検索
	if (models_.contains(filePath)) {
		//すでに読み込み済みならreturn
		return;
	}

	//モデルの生成とファイル読み込み、初期化
	std::unique_ptr<Model> model = std::make_unique<Model>();
	model->Initialize(modelCommon_,filePath);

	//モデルをコンテナに追加
	models_.insert(std::make_pair(filePath, std::move(model)));
}

Model* ModelManager::FindModel(const std::string& filePath) {
	
	//読み込み済みモデルを検索
	if(models_.contains(filePath)){
		//読み込みモデルを戻り値としてreturn
		return models_.at(filePath).get();
	}

	//ファイル名一致なし
	return nullptr;
}
