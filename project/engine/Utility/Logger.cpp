#include "Logger.h"

#include <debugapi.h>
#include <iostream>

//============================================================================
//出力ウィンドウに文字を出す関数
//============================================================================
void Logger::Log(const std::string& message) {
	 OutputDebugStringA(message.c_str());
}

