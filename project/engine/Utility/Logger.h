#pragma once
#include <Windows.h>
#include <string>

//=============================================================================
// Logger namespace
//=============================================================================
namespace Logger {


	//出力ウィンドウに文字を出す関数
	void Log(const std::string& message);
};