#include "TextureManager.h"

TextureManager* TextureManager::GetInstance() {
	if (instance_ == nullptr) {
		instance_ = new TextureManager();
	}
	return instance_;
}

void TextureManager::Finalize() {
	delete instance_;
	instance_ = nullptr;
}
