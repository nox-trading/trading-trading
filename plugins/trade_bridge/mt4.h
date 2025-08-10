#pragma once

#include <windows.h>
#include <ctime>

#pragma warning(push)
#pragma warning(disable: 4828)
#include "..\include\MT4ServerAPI.h"
#pragma warning(pop)

#define TIME_RATE				((double)1.6777216)
#define STDTIME(custom_time)	((DWORD)((double)(custom_time)*TIME_RATE))