#pragma once

#include "targetver.h"

#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include <signal.h>

#define STEAMWORKS_CLIENT_INTERFACES
#include "Open Steamworks\Open Steamworks\Steamworks.h"

#include "token_generator\token_generator.h"

#ifdef _MSC_VER
	#pragma comment( lib, "Open Steamworks\\Resources\\Libs\\Win32\\steamclient.lib" )
#endif
