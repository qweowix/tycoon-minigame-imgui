#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
#define DIRECTINPUT_VERSION 0x0800
#include "main.h"
#include "menu.h"

extern void SetMainIconPath(const std::string& path);
extern void ReleaseAllLoadedTextures();

const char* AppClass = "by qweowix";
const char* AppName = "Tycoon Minigame by qweowix";
HWND hwnd = NULL;
LPDIRECT3D9              g_pD3D = NULL;
LPDIRECT3DDEVICE9        g_pd3dDevice = NULL;
D3DPRESENT_PARAMETERS    g_d3dpp = {};
ImFont* DefaultFont = nullptr;
ImFont* BigFont = nullptr;
ImFont* MiniFont = nullptr;


float dynamicWindowWidth = 750.0f;
float dynamicWindowHeight = 500.0f;

//float dynamicWindowWidth = 1920.0f;
//float dynamicWindowHeight = 1080.0f;

int active_tab = 0;
bool showConfirmReset = false;
bool initialized = false;
int points = 0, manual = 1, iauto = 0, miner = 0, hacker = 0;
int hackerReward = 0;

int minerCountdown[3] = { 0, 15, 10 };
int hackerCountdown[3] = { 0, 25, 20 };

const int numParticles = 70;
ImVec2 particlePositions[numParticles];
ImVec2 particleDistance;
ImVec2 particleVelocities[numParticles];

std::mt19937 minerRng((unsigned)std::chrono::steady_clock::now().time_since_epoch().count());
std::mt19937 hackerRng((unsigned)std::chrono::steady_clock::now().time_since_epoch().count());


int main(int, char**)
{
    int size = MultiByteToWideChar(CP_UTF8, 0, AppClass, -1, NULL, 0);
    wchar_t* AppClass2 = new wchar_t[size];
    MultiByteToWideChar(CP_UTF8, 0, AppClass, -1, AppClass2, size);


    int size2 = MultiByteToWideChar(CP_UTF8, 0, AppName, -1, NULL, 0);
    wchar_t* AppName2 = new wchar_t[size2];
    MultiByteToWideChar(CP_UTF8, 0, AppClass, -1, AppName2, size);



    RECT desktop;
    GetWindowRect(GetDesktopWindow(), &desktop);
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, AppClass2, NULL };
    RegisterClassEx(&wc);
    hwnd = CreateWindowEx(WS_EX_TOPMOST | WS_EX_LAYERED, AppClass2, AppName2, WS_POPUP, (desktop.right / 2) - (dynamicWindowWidth / 2), (desktop.bottom / 2) - (dynamicWindowHeight / 2), dynamicWindowWidth, dynamicWindowHeight, 0, 0, wc.hInstance, 0);

    //SetLayeredWindowAttributes(hwnd, 0, 255, LWA_ALPHA);
    SetLayeredWindowAttributes(hwnd, RGB(255,0,255), 0, ULW_COLORKEY);

    if (CreateDeviceD3D(hwnd) < 0)
    {
        CleanupDeviceD3D();
        UnregisterClass(wc.lpszClassName, wc.hInstance);
        return 1;
    }


    ShowWindow(hwnd, SW_SHOWDEFAULT);
    UpdateWindow(hwnd);
    SetWindowPos(hwnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    
    //font
    auto GetExeDirW = []() -> std::wstring {
        wchar_t wbuf[MAX_PATH];
        DWORD len = GetModuleFileNameW(NULL, wbuf, MAX_PATH);
        if (len == 0) return L".";
        std::wstring wpath(wbuf, wbuf + len);
        size_t pos = wpath.find_last_of(L"\\/");
        return (pos == std::wstring::npos) ? wpath : wpath.substr(0, pos);
        };

    std::wstring fontPathW = GetExeDirW() + L"\\Local\\font\\web_ibm_mda.ttf";

    FILE* f = nullptr;
    if (_wfopen_s(&f, fontPathW.c_str(), L"rb") == 0 && f)
    {
        fseek(f, 0, SEEK_END);
        long fsize = ftell(f);
        fseek(f, 0, SEEK_SET);
        if (fsize > 0)
        {
            std::vector<unsigned char> buf;
            buf.resize((size_t)fsize);
            size_t read = fread(buf.data(), 1, (size_t)fsize, f);
            fclose(f);
            if (read == (size_t)fsize)
            {
                ImFontConfig cfg1;
                cfg1.FontDataOwnedByAtlas = true;
                unsigned char* mem1 = (unsigned char*)IM_ALLOC(fsize);
                memcpy(mem1, buf.data(), (size_t)fsize);
                DefaultFont = io.Fonts->AddFontFromMemoryTTF(mem1, (int)fsize, 16.0f, &cfg1, io.Fonts->GetGlyphRangesCyrillic());

                ImFontConfig cfg2;
                cfg2.FontDataOwnedByAtlas = true;
                unsigned char* mem2 = (unsigned char*)IM_ALLOC(fsize);
                memcpy(mem2, buf.data(), (size_t)fsize);
                BigFont = io.Fonts->AddFontFromMemoryTTF(mem2, (int)fsize, 28.0f, &cfg2, io.Fonts->GetGlyphRangesCyrillic());

                ImFontConfig cfg3;
                cfg3.FontDataOwnedByAtlas = true;
                unsigned char* mem3 = (unsigned char*)IM_ALLOC(fsize);
                memcpy(mem3, buf.data(), (size_t)fsize);
                MiniFont = io.Fonts->AddFontFromMemoryTTF(mem3, (int)fsize, 13.0f, &cfg3, io.Fonts->GetGlyphRangesCyrillic());
            }
        }
    }
    else
    {
        //fallback
        DefaultFont = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\segoeui.ttf", 16.0f, NULL, io.Fonts->GetGlyphRangesCyrillic());
        BigFont = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\segoeui.ttf", 28.0f, NULL, io.Fonts->GetGlyphRangesCyrillic());
        MiniFont = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\segoeui.ttf", 13.0f, NULL, io.Fonts->GetGlyphRangesCyrillic());
    }
    {
        std::string exeDir = GetExeDirUtf8();\

        //get icons paths

        SetIconPath("menu_main", exeDir + "\\Local\\icons\\menu_main.png");
        SetIconPath("menu_manual", exeDir + "\\Local\\icons\\menu_manual.png");
        SetIconPath("menu_auto", exeDir + "\\Local\\icons\\menu_auto.png");
        SetIconPath("menu_miner", exeDir + "\\Local\\icons\\menu_miner.png");
        SetIconPath("menu_hacker", exeDir + "\\Local\\icons\\menu_hacker.png");
        SetIconPath("menu_pets", exeDir + "\\Local\\icons\\menu_pets.png");
        SetIconPath("menu_credits", exeDir + "\\Local\\icons\\menu_credits.png");
        SetIconPath("menu_changelog", exeDir + "\\Local\\icons\\menu_changelog.png");
        SetIconPath("menu_settings", exeDir + "\\Local\\icons\\menu_settings.png");
        SetIconPath("pickaxe1", exeDir + "\\Local\\icons\\pickaxe1.png");
        SetIconPath("pickaxe2", exeDir + "\\Local\\icons\\pickaxe2.png");
        SetIconPath("binar1", exeDir + "\\Local\\icons\\binar1.png");
        SetIconPath("binar2", exeDir + "\\Local\\icons\\binar2.png");
        SetIconPath("binar3", exeDir + "\\Local\\icons\\binar3.png");
        SetIconPath("binar4", exeDir + "\\Local\\icons\\binar4.png");
        OutputDebugStringA(("Icons registered: " + exeDir + "\\Local\\icons\\menu_main.png\n").c_str());
    }
    // ------------------------------------------------------------------------------
	// menu initialization
    ImGui::StyleColorsDark();
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX9_Init(g_pd3dDevice);
    SetImguiStyle();
    LoadGame();

    MSG msg;
    ZeroMemory(&msg, sizeof(msg));
    static bool open = true;

    char somelogin[25] = "";
    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            continue;
        }
        if (!open)
        {
            SaveGame();
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(20));

        ImGui_ImplDX9_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        RenderMenu(&open);


        ImGui::EndFrame();

        g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0, 1.0f, 0);
        if (g_pd3dDevice->BeginScene() >= 0)
        {
            ImGui::Render();
            ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
            g_pd3dDevice->EndScene();
        }
        HRESULT result = g_pd3dDevice->Present(NULL, NULL, NULL, NULL);
        if (result == D3DERR_DEVICELOST && g_pd3dDevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET) ResetDevice();
    }

  
    SaveGame();

    ImGui_ImplDX9_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    ReleaseAllLoadedTextures();
    ReleaseIcons();
    CleanupDeviceD3D();
    DestroyWindow(hwnd);
    UnregisterClass(wc.lpszClassName, wc.hInstance);

    return 0;
}
