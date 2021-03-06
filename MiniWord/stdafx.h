// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // 从 Windows 头中排除极少使用的资料
#define _CRT_SECURE_NO_WARNINGS			// Windows 头文件:
#include <windows.h>

// C 运行时头文件
#pragma comment(lib, "imm32.lib")
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <imm.h>

// TODO: 在此处引用程序需要的其他头文件
#include "MiniWord.h"
#include "Caret.h"
#include "subeditor.h"
#include "Shellapi.h"
#include <algorithm>
#include <sstream>

/*复制函数,所有wcsncpy与wcscpy改为wcsmove*/
void wcsnmove(wchar_t* Dst, const wchar_t* Src, int p);
void wcsmove(wchar_t* Dst, const wchar_t* Src);
