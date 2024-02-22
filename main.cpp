#pragma warning(push)
//C4023の警告を見なかったことにする
#pragma warning(disable:4023)
#include <Windows.h>
#pragma warning(pop)

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	OutputDebugStringA("Hello,DirectX!\n");

	return 0;
}