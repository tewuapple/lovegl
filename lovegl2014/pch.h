#pragma once
#include <sdkddkver.h>
#undef _WIN32_WINNT
#define _WIN32_WINNT _WIN32_WINNT_WIN8

#include <atlbase.h>
#include <atlwin.h>
#include <atlstr.h>

#include <algorithm>
#include <vector>
#include <string>
#include <random>

#include "dx.h"

using namespace std;
using namespace D2D1;

#pragma comment(lib, "winmm")