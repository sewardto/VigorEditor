// stdafx.cpp : 只包括标准包含文件的源文件
// MiniWord.pch 将作为预编译标头
// stdafx.obj 将包含预编译类型信息

#include "stdafx.h"

// TODO: 在 STDAFX.H 中引用任何所需的附加头文件，
//而不是在此文件中引用


/*复制函数,所有wcsncpy改为wcsmove*/
void wcsnmove(wchar_t* Dst, const wchar_t* Src,int p) {
	memmove(Dst, Src, sizeof(wchar_t)*p);
};
/*复制函数,所有wcscpy改为wcsmove*/
void wcsmove(wchar_t* Dst, const wchar_t* Src) {
	memmove(Dst, Src, sizeof(wchar_t)*wcslen(Src));
}

