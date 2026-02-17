#include "main.h"
#include "menu.h"

void SaveGame()
{
    std::wstring wpath = GetSaveFilePathW();
    FILE* f = nullptr;
    errno_t err = _wfopen_s(&f, wpath.c_str(), L"wb");
    if (err != 0 || !f)
    {
        std::wstring msg = L"SaveGame: failed to open file: " + wpath + L"\n";
        OutputDebugFmtW(msg);
        return;
    }
    fprintf(f, "points=%d\n", points);
    fprintf(f, "manual=%d\n", manual);
    fprintf(f, "iauto=%d\n", iauto);
    fprintf(f, "miner=%d\n", miner);
    fprintf(f, "hacker=%d\n", hacker);
    for (size_t i = 0; i < pets.size(); ++i) {
        fprintf(f, "pet_%zu_owned=%d\n", i, pets[i].owned ? 1 : 0);
    }
    fprintf(f, "equiped_pet=%d\n", equiped_pet);
    fflush(f);
    fclose(f);

    std::wstring msg = L"SaveGame: wrote " + wpath + L"\n";
    OutputDebugFmtW(msg);
}
void LoadGame()
{
    std::wstring wpath = GetSaveFilePathW();
    FILE* f = nullptr;
    errno_t err = _wfopen_s(&f, wpath.c_str(), L"rb");
    if (err != 0 || !f)
    {
        std::wstring msg = L"LoadGame: file not found (ok if first run): " + wpath + L"\n";
        OutputDebugFmtW(msg);
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), f))
    {
        char key[64] = { 0 };
        int value = 0;
        if (sscanf_s(line, "%63[^=]=%d", key, (unsigned)sizeof(key), &value) == 2)
        {
            if (strcmp(key, "points") == 0) points = value;
            if (strcmp(key, "manual") == 0) manual = value;
            if (strcmp(key, "iauto") == 0) iauto = value;
            if (strcmp(key, "miner") == 0) miner = value;
            if (strcmp(key, "hacker") == 0) hacker = value;
            int petIndex = -1;
            if (sscanf_s(key, "pet_%d_owned", &petIndex) == 1) {
                if (petIndex >= 0 && petIndex < (int)pets.size()) {
                    pets[petIndex].owned = (value != 0);
                }
            }
            if (strcmp(key, "equiped_pet") == 0) equiped_pet = value;
        }
    }
    fclose(f);

    if (manual <= 0) manual = 1;
    if (points < 0) points = 0;

    std::wstring msg = L"LoadGame: loaded points=" + std::to_wstring(points) + L" manual=" + std::to_wstring(manual) + L" from " + wpath + L"\n";
    OutputDebugFmtW(msg);
}


void Upgrade(int& name, int price, int level) {
    if (points >= price)
    {
        points -= price;
        name = level;
    }
    else {
        cantafford = true;
    }
}

void InitAudio() {
    if (ma_engine_init(NULL, &engine) == MA_SUCCESS) {
        isAudioInitialized = true;
    }
}

void CleanupAudio() {
    if (isAudioInitialized) {
        ma_engine_uninit(&engine);
    }
}

bool PlaySoundFromWidePath(const std::wstring& wpath)
{
    if (!isAudioInitialized) return false;
    std::string utf8Path = WideToUtf8(wpath);
    if (!utf8Path.empty()) {
        ma_result r = ma_engine_play_sound(&engine, utf8Path.c_str(), NULL);
        if (r == MA_SUCCESS) return true;
    }

    char ansiPath[MAX_PATH] = { 0 };
    WideCharToMultiByte(CP_ACP, 0, wpath.c_str(), -1, ansiPath, MAX_PATH, NULL, NULL);
    if (ansiPath[0] != '\0') {
        ma_result r2 = ma_engine_play_sound(&engine, ansiPath, NULL);
        if (r2 == MA_SUCCESS) return true;
    }

    return false;
}

LPDIRECT3DTEXTURE9 GetIconTexture(const std::string& path)
{
    auto it = g_IconCache.find(path);
    if (it != g_IconCache.end())
        return it->second;

    LPDIRECT3DTEXTURE9 tex = LoadTextureFromFile_DX9(path, nullptr, nullptr);
    if (tex)
        g_IconCache[path] = tex;

    return tex;
}
void ReleaseIcons()
{
    g_IconCache.clear();
}

void SetMainIconPath(const std::string& path)
{
    g_mainIconPath = path;
    g_IconPaths["menu_main"] = path;
}
void SetIconPath(const std::string& name, const std::string& path)
{
    g_IconPaths[name] = path;
}
std::string GetIconPath(const std::string& name)
{
    auto it = g_IconPaths.find(name);
    if (it == g_IconPaths.end()) return std::string();
    return it->second;
}
LPDIRECT3DTEXTURE9 LoadTextureFromFile_DX9(const std::string& fullpath, int* out_w, int* out_h)
{
    auto itTex = g_ImgTextures.find(fullpath);
    if (itTex != g_ImgTextures.end()) {
        if (out_w) *out_w = 0;
        if (out_h) *out_h = 0;
        return itTex->second;
    }

    std::string dbg = "LoadTextureFromFile_DX9: try path: " + fullpath + "\n";
    OutputDebugStringA(dbg.c_str());

    int w = 0, h = 0, channels = 0;
    unsigned char* data = stbi_load(fullpath.c_str(), &w, &h, &channels, 4);

    if (!data)
    {
        OutputDebugStringA("LoadTextureFromFile_DX9: stbi_load failed, trying wide-open + stbi_load_from_memory\n");
        int wlen = MultiByteToWideChar(CP_UTF8, 0, fullpath.c_str(), -1, nullptr, 0);
        if (wlen > 0)
        {
            std::wstring wpath;
            wpath.resize(wlen);
            MultiByteToWideChar(CP_UTF8, 0, fullpath.c_str(), -1, &wpath[0], wlen);
            if (!wpath.empty() && wpath.back() == L'\0') wpath.pop_back();
            FILE* f = nullptr;
            if (_wfopen_s(&f, wpath.c_str(), L"rb") == 0 && f)
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
                        data = stbi_load_from_memory(buf.data(), (int)buf.size(), &w, &h, &channels, 4);
                        if (!data)
                        {
                            OutputDebugStringA("LoadTextureFromFile_DX9: stbi_load_from_memory failed\n");
                        }
                    }
                    else
                    {
                        fclose(f);
                        OutputDebugStringA("LoadTextureFromFile_DX9: fread size mismatch\n");
                    }
                }
                else
                {
                    fclose(f);
                    OutputDebugStringA("LoadTextureFromFile_DX9: file size <= 0\n");
                }
            }
            else
            {
                OutputDebugStringW((std::wstring(L"LoadTextureFromFile_DX9: _wfopen_s failed for path: ") + wpath + L"\n").c_str());
            }
        }
        else
        {
            OutputDebugStringA("LoadTextureFromFile_DX9: MultiByteToWideChar failed\n");
        }
    }

    if (!data)
    {
        OutputDebugStringA(("LoadTextureFromFile_DX9: failed to load image: " + fullpath + "\n").c_str());
        return nullptr;
    }
    if (w <= 0 || h <= 0)
    {
        stbi_image_free(data);
        OutputDebugStringA("LoadTextureFromFile_DX9: invalid image dimensions\n");
        return nullptr;
    }

    LPDIRECT3DTEXTURE9 tex = nullptr;
    HRESULT hr = g_pd3dDevice->CreateTexture(w, h, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &tex, NULL);
    if (FAILED(hr) || !tex)
    {
        stbi_image_free(data);
        OutputDebugStringA("LoadTextureFromFile_DX9: CreateTexture failed\n");
        return nullptr;
    }

    D3DLOCKED_RECT rect;
    if (SUCCEEDED(tex->LockRect(0, &rect, NULL, 0)))
    {
        for (int yy = 0; yy < h; ++yy)
        {
            unsigned char* dstRow = (unsigned char*)rect.pBits + (size_t)rect.Pitch * yy;
            unsigned char* srcRow = data + (size_t)w * 4 * yy;
            for (int xx = 0; xx < w; ++xx)
            {
                unsigned char r = srcRow[xx * 4 + 0];
                unsigned char g = srcRow[xx * 4 + 1];
                unsigned char b = srcRow[xx * 4 + 2];
                unsigned char a = srcRow[xx * 4 + 3];
                dstRow[xx * 4 + 0] = b;
                dstRow[xx * 4 + 1] = g;
                dstRow[xx * 4 + 2] = r;
                dstRow[xx * 4 + 3] = a;
            }
        }
        tex->UnlockRect(0);
    }
    else
    {
        tex->Release();
        stbi_image_free(data);
        OutputDebugStringA("LoadTextureFromFile_DX9: LockRect failed\n");
        return nullptr;
    }

    stbi_image_free(data);

    g_ImgTextures.emplace(fullpath, tex);
    if (out_w) *out_w = w;
    if (out_h) *out_h = h;
    OutputDebugStringA(("LoadTextureFromFile_DX9: loaded OK: " + fullpath + "\n").c_str());
    return tex;
}
void ReleaseAllLoadedTextures()
{
    for (auto& p : g_ImgTextures)
    {
        if (p.second)
        {
            p.second->Release();
        }
    }
    g_ImgTextures.clear();
}
void RenderImageFullPath(const std::string& fullpath, const ImVec2& size)
{
    LPDIRECT3DTEXTURE9 tex = LoadTextureFromFile_DX9(fullpath);
    if (!tex) return;
    ImGui::Image((ImTextureID)tex, size);
}
void RenderIcon(const std::string& name, const ImVec2& size)
{
    std::string path = GetIconPath(name);
    if (path.empty())
        return;

    LPDIRECT3DTEXTURE9 tex = GetIconTexture(path);
    if (!tex) return;
    ImGui::Image((ImTextureID)tex, size);
}
void CenteredText(const char* text, ImFont* font)
{
    if (font) ImGui::PushFont(font);
    ImVec2 ts = ImGui::CalcTextSize(text);
    float winW = ImGui::GetWindowSize().x;
    float x = (winW - ts.x) * 0.5f;
    if (x < 0.0f) x = 0.0f;
    ImGui::SetCursorPosX(x);
    ImGui::Text("%s", text);
    if (font) ImGui::PopFont();
}


HRESULT CreateDeviceD3D(HWND hWnd)
{
    if ((g_pD3D = Direct3DCreate9(D3D_SDK_VERSION)) == NULL) return E_FAIL;

    ZeroMemory(&g_d3dpp, sizeof(g_d3dpp));
    g_d3dpp.Windowed = TRUE;
    g_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    g_d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
    g_d3dpp.EnableAutoDepthStencil = TRUE;
    g_d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
    g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
    if (g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &g_d3dpp, &g_pd3dDevice) < 0) return E_FAIL;

    return S_OK;
}

void CleanupDeviceD3D()
{
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = NULL; }
    if (g_pD3D) { g_pD3D->Release(); g_pD3D = NULL; }
}

void ResetDevice()
{
    ImGui_ImplDX9_InvalidateDeviceObjects();
    HRESULT hr = g_pd3dDevice->Reset(&g_d3dpp);
    if (hr == D3DERR_INVALIDCALL) IM_ASSERT(0);
    ImGui_ImplDX9_CreateDeviceObjects();
}

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_SIZE:
        if (g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED)
        {
            g_d3dpp.BackBufferWidth = LOWORD(lParam);
            g_d3dpp.BackBufferHeight = HIWORD(lParam);
            ResetDevice();
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) return 0;
        break;
    case WM_NCHITTEST:
    {
        ImVec2 Shit = ImGui::GetMousePos();
        if (Shit.y < 25 && Shit.x < dynamicWindowWidth - 25)
        {
            LRESULT hit = DefWindowProc(hWnd, msg, wParam, lParam);
            if (hit == HTCLIENT) hit = HTCAPTION;
            return hit;
        }
        else break;
    }
    case WM_DESTROY:
        SaveGame();
        ReleaseIcons();
        PostQuitMessage(0);
        return 0;
    default:
        ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

std::wstring GetSaveFilePathW()
{
    PWSTR path = nullptr;
    std::wstring doc;
    HRESULT hr = SHGetKnownFolderPath(FOLDERID_Documents, KF_FLAG_CREATE, NULL, &path);
    if (SUCCEEDED(hr) && path)
    {
        doc = path;
        CoTaskMemFree(path);
    }
    if (doc.empty())
    {
        wchar_t* appdata = nullptr;
        size_t sz = 0;
        if (_wdupenv_s(&appdata, &sz, L"APPDATA") == 0 && appdata)
        {
            doc = std::wstring(appdata);
            free(appdata);
        }
        else
        {
            doc = L".";
        }
    }
    if (!doc.empty() && doc.back() != L'\\' && doc.back() != L'/') doc += L'\\';
    doc += L"TycoonSave.txt";
    return doc;
}

void OutputDebugFmtW(const std::wstring& s)
{
    OutputDebugStringW(s.c_str());
}

std::string WideToUtf8(const std::wstring& w)
{
    if (w.empty()) return std::string();
    int size = WideCharToMultiByte(CP_UTF8, 0, w.c_str(), -1, nullptr, 0, nullptr, nullptr);
    if (size <= 0) return std::string();
    std::string out(size, '\0');
    WideCharToMultiByte(CP_UTF8, 0, w.c_str(), -1, &out[0], size, nullptr, nullptr);
    if (!out.empty() && out.back() == '\0') out.pop_back();
    return out;
}




bool IconButtonWithLabel(const char* id, const char* label, const char* iconName, const ImVec2& size, float iconSize)
{
    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImGui::InvisibleButton(id, size);
    bool clicked = ImGui::IsItemClicked();

    ImU32 col = ImGui::GetColorU32(ImGuiCol_Button);
    if (ImGui::IsItemHovered()) col = ImGui::GetColorU32(ImGuiCol_ButtonHovered);
    if (ImGui::IsItemActive())  col = ImGui::GetColorU32(ImGuiCol_ButtonActive);
    ImDrawList* dl = ImGui::GetWindowDrawList();
    dl->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y), col, ImGui::GetStyle().FrameRounding);

    std::string path = GetIconPath(iconName);
    bool hasIcon = false;
    if (!path.empty())
    {
        LPDIRECT3DTEXTURE9 tex = GetIconTexture(path);
        if (tex)
        {
            hasIcon = true;
            ImVec2 icoPos(pos.x + 6.0f, pos.y + (size.y - iconSize) * 0.5f);
            dl->AddImage((ImTextureID)tex, icoPos, ImVec2(icoPos.x + iconSize, icoPos.y + iconSize));
        }
    }

    float textOffsetX = pos.x + (hasIcon ? (6.0f + iconSize + 6.0f) : 12.0f);
    ImVec2 textPos = ImVec2(textOffsetX, pos.y + (size.y - ImGui::GetFontSize()) * 0.5f);
    dl->AddText(textPos, ImGui::GetColorU32(ImGuiCol_Text), label);

    dl->AddRect(pos, ImVec2(pos.x + size.x, pos.y + size.y), ImGui::GetColorU32(ImGuiCol_Border), ImGui::GetStyle().FrameRounding);

    return clicked;
}

std::string GetExeDirUtf8()
{
    wchar_t wbuf[MAX_PATH];
    DWORD len = GetModuleFileNameW(NULL, wbuf, MAX_PATH);
    if (len == 0) return std::string(".");
    std::wstring wpath(wbuf, wbuf + len);
    size_t pos = wpath.find_last_of(L"\\/");
    std::wstring wdir = (pos == std::wstring::npos) ? wpath : wpath.substr(0, pos);
    if (wdir.empty()) wdir = L".";
    int size = WideCharToMultiByte(CP_UTF8, 0, wdir.c_str(), -1, NULL, 0, NULL, NULL);
    if (size <= 0) return std::string(".");
    std::string dir(size, '\0');
    WideCharToMultiByte(CP_UTF8, 0, wdir.c_str(), -1, &dir[0], size, NULL, NULL);
    if (!dir.empty() && dir.back() == '\0') dir.pop_back();
    return dir;
}
