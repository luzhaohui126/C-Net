#pragma once

#include<string>

extern "C"
{
	int _declspec(dllexport) Add(int a, int b) {
		return a + b;
	}

	typedef void(*CallBack1)(const char* str);

	void _declspec(dllexport) TestCall1(const char* str, CallBack1 cb) {
		std::string s = "Hello ";
		s += str;
		cb(s.c_str());
	}
}
