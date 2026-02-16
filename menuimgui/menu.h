#pragma once
#include "main.h"

//actually i hate this piece of code :crying:

struct Pet { std::string name; int price; bool owned; };
extern std::vector<Pet> pets;

extern int equiped_pet;

extern int minerCountdown[3];
extern int hackerCountdown[3];
static std::wstring minerMessage;
static std::wstring hackerMessage;
static std::chrono::steady_clock::time_point hackerMessageTime;
static std::chrono::steady_clock::time_point minerMessageTime;
static const int minerMessageDurationSec = 7;
static const int hackerMessageDurationSec = 7;

static bool success;
static bool showpetmsg;

extern int active_tab;
extern bool showConfirmReset;
extern int points, manual, iauto, miner, hacker;
extern int hackerReward;
extern bool initialized;

extern const int numParticles;
extern ImVec2 particlePositions[];
extern ImVec2 particleDistance;
extern ImVec2 particleVelocities[];

extern std::mt19937 minerRng;
extern std::mt19937 hackerRng;

extern bool IconButtonWithLabel(const char* id, const char* label, const char* iconName, const ImVec2& size, float iconSize = 45.0f);
static std::unordered_map<std::string, LPDIRECT3DTEXTURE9> g_IconCache;
static LPDIRECT3DTEXTURE9 LoadTextureFromFile_DX9(const std::string& fullpath, int* out_w, int* out_h);

static std::unordered_map<std::string, LPDIRECT3DTEXTURE9> g_ImgTextures;
static std::string g_mainIconPath;

static std::unordered_map<std::string, std::string> g_IconPaths;

void RenderMenu(bool* open);

void SetImguiStyle();
void SaveGame();
void LoadGame();

void SetMainIconPath(const std::string& path);
void SetIconPath(const std::string& name, const std::string& path);
std::string GetIconPath(const std::string& name);
void RenderIcon(const std::string& name, const ImVec2& size);

namespace settings {

	inline ImVec4 particleColour = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

}

void CenteredText(const char* text, ImFont* font = nullptr);
LPDIRECT3DTEXTURE9 GetIconTexture(const std::string& path);

void ReleaseIcons();

void SetMainIconPath(const std::string& path);

void SetIconPath(const std::string& name, const std::string& path);

std::string GetIconPath(const std::string& name);

void RenderIcon(const std::string& name, const ImVec2& size);

static LPDIRECT3DTEXTURE9 LoadTextureFromFile_DX9(const std::string& fullpath, int* out_w = nullptr, int* out_h = nullptr);

void RenderImageFullPath(const std::string& fullpath, const ImVec2& size);

void ReleaseAllLoadedTextures();
