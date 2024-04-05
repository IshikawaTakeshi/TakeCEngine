#pragma once
#include <string>

class Logger {
public:

	//出力ウィンドウに文字を出す関数
	static void Log(const std::string& message);

	//コンバートストリング(wstring型)
	static std::wstring ConvertString(const std::string& str);

	//コンバートストリング(string型)
	static std::string ConvertString(const std::wstring& str);
};

