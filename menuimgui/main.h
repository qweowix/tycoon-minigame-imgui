#define NOMINMAX
#pragma once


//pretty much everything you can imagine XD

#include "imgui.h"
#include "imgui_impl_dx9.h"
#include "imgui_impl_win32.h"
#include "stb_image.h"
#include "miniaudio.h"
#include <shlobj.h>
#include <d3d9.h>
#include <dinput.h>
#include <tchar.h>
#include <Windows.h>
#include <chrono>
#include <thread>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <cstdio>
#include <vector> 
#include <random>
#include <unordered_map>
#include <functional>
#include <algorithm>
#include <cfloat>

extern const char* AppClass;
extern const char* AppName;
extern HWND hwnd;
extern LPDIRECT3D9              g_pD3D;
extern LPDIRECT3DDEVICE9        g_pd3dDevice;
extern D3DPRESENT_PARAMETERS    g_d3dpp;
extern ImFont* DefaultFont;
extern ImFont* BigFont;
extern ImFont* MiniFont;

extern ma_engine engine;
extern bool isAudioInitialized;

void InitAudio();
void CleanupAudio();
bool PlaySoundFromWidePath(const std::wstring& path);

extern bool cantafford;

inline std::vector<std::wstring> clicksounds;

extern float dynamicWindowWidth;
extern float dynamicWindowHeight;
extern int currentPPC;

// Прототипы функций (реализации перенесены в main.cpp)

HRESULT CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void ResetDevice();
void ReleaseIcons();

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

extern std::wstring GetSaveFilePathW();

extern void OutputDebugFmtW(const std::wstring& s);

extern std::string WideToUtf8(const std::wstring& w);

void SaveGame();

void LoadGame();

std::string GetExeDirUtf8();


