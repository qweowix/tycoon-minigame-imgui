#define MINIAUDIO_IMPLEMENTATION
#include "../menu.h"
#include "imgui.h"
#pragma comment(lib, "Shell32.lib")

static float clickVolume = 0.5f;

int idx = 0;

bool cantafford = false;

// get cooldowns and upgrade values

int GetminerCountdown(int miner) {
    if (miner <= 0) return 0;
    int base = (miner == 1) ? 20 : 15;
    int petReduction = (equiped_pet == 2) ? 7 : 0;
    int result = base - petReduction;
    return std::max(1, result);
}

int GethackerCountdown(int hacker) {
    if (hacker <= 0) return 0;
    int base = (hacker == 1) ? 25 : 20;
    int petReduction = (equiped_pet == 3) ? 10 : 0;
    int result = base - petReduction;
    return std::max(1, result);
}

int GetPPC(int manual) {
    switch (manual)
    {
    case 1: return 1;
    case 2: return 2;
    case 3: return 5;
    case 4: return 12;
    case 5: return 24;
	case 6: return 40;
    default: return 0;
    }
}
int GetGPPS(int iauto) {
    switch (iauto)
    {
    case 1: return 3;
    case 2: return 8;
    case 3: return 16;
    case 4: return 33;
    case 5: return 67;
    case 6: return 100;
    default: return 0;
    }
}

int equiped_pet = -1;

std::vector<Pet> pets = {
    {"Dog", 1500, false},
    {"HelloWorld()", 5000, false},
    {"Brooo", 39997, false},
    {"Plushie Chance", 77777, false},
    {"Sponge Bob Square Pants", 500000, false}
};
bool buyPet(std::vector<Pet>& pets, int index, int& money) {
    if (pets[index].owned) return false;
    if (money < pets[index].price) {
        cantafford = true;
		return false;
    }
    money -= pets[index].price;
    pets[index].owned = true;
    return true;
}



static std::chrono::steady_clock::time_point minerStartTime = std::chrono::steady_clock::now();
static float minerDispFraction = 0.0f;
static std::chrono::steady_clock::time_point hackerStartTime = std::chrono::steady_clock::now();
static float hackerDispFraction = 0.0f;

float pointsVisual = 0.0f;
static int petsCooldownRemaining = 0;
static std::chrono::steady_clock::time_point lastPetsTick = std::chrono::steady_clock::now();
static std::wstring petsMessage;
static std::chrono::steady_clock::time_point petsMessageTime;

//menu style (colors)

void SetImguiStyle() {
    ImGuiStyle& style = ImGui::GetStyle();
    style.Colors[ImGuiCol_TitleBg] = ImVec4(0.9f, 0.1f, 0.0f, 1.0f);
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.9f, 0.1f, 0.0f, 1.0f);
    style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.1f, 0.1f, 0.1f, 1.0f);
    style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.6f, 0.3f, 0.3f, 1.0f);  // Red color (RGBA: 1, 0, 0, 1)
    style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(1.0f, 1.0f, 1.0f, 0.6f);
    style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.5f, 0.2f, 0.2f, 1.0f);
    style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
    style.Colors[ImGuiCol_Border] = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
    style.Colors[ImGuiCol_BorderShadow] = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
    style.Colors[ImGuiCol_Button] = ImVec4(0.9f, 0.15f, 0.15f, 1.0f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.5f, 0.2f, 0.2f, 1.0f);
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.5f, 0.2f, 0.2f, 1.0f);
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.01f, 0.01f, 0.01f, 1.0f);\
    style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.7f);
	style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.9f, 0.1f, 0.0f, 1.0f);
	style.Colors[ImGuiCol_FrameBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.95f);
    style.Colors[ImGuiCol_PopupBg] = ImVec4(0.06f, 0.06f, 0.06f, 0.95f);
    style.Colors[ImGuiCol_Header] = ImVec4(0.85f, 0.15f, 0.15f, 1.00f);
    style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.95f, 0.30f, 0.30f, 1.00f);
    style.Colors[ImGuiCol_HeaderActive] = ImVec4(1.00f, 0.45f, 0.45f, 1.00f);
    

    ImGui::GetStyle().Colors[ImGuiCol_WindowBg].w = 1.0f;
    ImGui::GetStyle().Colors[ImGuiCol_ChildBg].w = 1.0f;
    ImGui::GetStyle().Colors[ImGuiCol_PopupBg].w = 1.0f;

    ImGui::GetStyle().FrameRounding = 7.0f;
    ImGui::GetStyle().GrabRounding = 4.0f;
    ImGui::GetStyle().ScrollbarRounding = 10.0f;
    ImGui::GetStyle().FramePadding = ImVec2(7, 3);
    ImGui::GetStyle().WindowRounding = 7.0f;
	ImGui::GetStyle().PopupRounding = 7.0f;
    ImGui::GetStyle().PopupBorderSize = 0.3f;
}

//------------------------------
// main menu rendering
//------------------------------

void RenderMenu(bool* open)
{



    DWORD dwFlag = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;

    ImGui::SetNextWindowSize(ImVec2(dynamicWindowWidth, dynamicWindowHeight), ImGuiCond_Once);
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Once);

    ImGui::Begin(AppName, open, dwFlag);

    ma_engine_set_volume(&engine, clickVolume);

    if (!initialized) {
        

        minerCountdown[miner] = GetminerCountdown(miner);
        hackerCountdown[hacker] = GethackerCountdown(hacker);

        for (int i = 0; i < numParticles; ++i)
        {

            particlePositions[i] = ImVec2(
                ImGui::GetWindowPos().x + ImGui::GetWindowSize().x * static_cast<float>(rand()) / RAND_MAX,
                ImGui::GetWindowPos().y + ImGui::GetWindowSize().y * static_cast<float>(rand()) / RAND_MAX
            );

            particleVelocities[i] = ImVec2(
                static_cast<float>((rand() % 11) - 5),
                static_cast<float>((rand() % 11) - 5)
            );

        }
        
		initialized = true;
    }
   

    int currentPPC = GetPPC(manual);
	int currentPPS = GetGPPS(iauto);

    if (equiped_pet == -1) {
        // nothing
    }
    else if (equiped_pet == 0) {
        currentPPC += 3;
    }
    else if (equiped_pet == 1) {
		currentPPS += 5;
    }
    else if (equiped_pet == 2) {
		//
    }
    else if (equiped_pet == 3) {
        currentPPC -= currentPPC;
        currentPPS -= currentPPS;
    }
    else if (equiped_pet == 4) {
		currentPPC += 25;
        currentPPS += 25;
    }

    if (cantafford) {
        ImGui::OpenPopup("Wait!");
        cantafford = false;
    }
    if (ImGui::BeginPopupModal("Wait!", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("You can not afford this upgrade!");
        ImGui::Separator();
        if (ImGui::Button("OK", ImVec2(120, 0)))
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

  
    using namespace std::chrono;
    static steady_clock::time_point lastAutoTick = steady_clock::now();
    static steady_clock::time_point lastMinerTick = steady_clock::now(); 
    static steady_clock::time_point lastHackerTick = steady_clock::now();

    steady_clock::time_point now = steady_clock::now();

    auto elapsedAuto = duration_cast<seconds>(now - lastAutoTick).count();
    if (elapsedAuto > 0)
    {
        points += currentPPS * static_cast<int>(elapsedAuto);
        lastAutoTick += seconds(elapsedAuto);
    }
    auto elapsedMiner = duration_cast<seconds>(now - lastMinerTick).count();

    //miner rng logic

    if (elapsedMiner > 0)
    {
        if (miner == 1)
        {
            minerCountdown[miner] -= static_cast<int>(elapsedMiner);
            if (minerCountdown[miner] <= 0)
            {
                std::uniform_int_distribution<int> dist(1, 100);
                int r = dist(minerRng);

                int reward = 0;
                std::wstring found;
                if (r <= 60) { reward = 150;  found = L"Coal"; }
                else if (r <= 85) { reward = 300; found = L"Iron"; }
                else if (r <= 95) { reward = 600; found = L"Gold"; }
                else if (r <= 99) { reward = 1300; found = L"Diamond"; }
                else { reward = 3000; found = L"Emerald"; }

                points += reward;

                minerMessage = L" Miner found " + found + L" (+" + std::to_wstring(reward) + L"$)";
                minerMessageTime = now;

                minerCountdown[miner] = GetminerCountdown(miner);
                minerStartTime = now;
                minerDispFraction = 0.0f;
            }
        }
        if (miner == 2)
        {
            minerCountdown[miner] -= static_cast<int>(elapsedMiner);
            if (minerCountdown[miner] <= 0)
            {
                std::uniform_int_distribution<int> dist(1, 100);
                int r = dist(minerRng);

                int reward = 0;
                std::wstring found;
                if (r <= 55) { reward = 301;  found = L"Coal"; }
                else if (r <= 83) { reward = 600; found = L"Iron"; }
                else if (r <= 95) { reward = 1000; found = L"Gold"; }
                else if (r <= 99) { reward = 3000; found = L"Diamond"; }
                else { reward = 10000; found = L"Emerald"; }

                points += reward;

                minerMessage = L"Miner found " + found + L" (+" + std::to_wstring(reward) + L"$)";
                minerMessageTime = now;

                minerCountdown[miner] = GetminerCountdown(miner);
                minerStartTime = now;
                minerDispFraction = 0.0f;
            }
        }
        lastMinerTick += seconds(elapsedMiner);
    }

	//hacker rng logic

    auto elapsedHacker = duration_cast<seconds>(now - lastHackerTick).count();
    if (elapsedHacker > 0) {
        if (hacker == 1)
        {
            hackerCountdown[hacker] -= static_cast<int>(elapsedHacker);
            if (hackerCountdown[hacker] <= 0)
            {
                std::uniform_int_distribution<int> dist(1, 100);
                int hr = dist(hackerRng);

                if (hr <= 70) { success = true; }
                else { success = false; }

                if (success) {
                    std::uniform_int_distribution<int> dist(1500, 4500);
                    hackerReward = dist(minerRng);
                    points += hackerReward;

                    hackerMessage = L"Successful hack! (+" + std::to_wstring(hackerReward) + L"$)";
                    hackerMessageTime = now;
                }
                else {
                    hackerMessage = L"Unfortunate";
                    hackerMessageTime = now;
                }

                hackerCountdown[hacker] = GethackerCountdown(hacker);
                hackerStartTime = now;
                hackerDispFraction = 0.0f;

            }
        }
        if (hacker == 2)
        {
            hackerCountdown[hacker] -= static_cast<int>(elapsedHacker);
            if (hackerCountdown[hacker] <= 0)
            {
                std::uniform_int_distribution<int> dist(1, 100);
                int hr = dist(hackerRng);

                if (hr <= 80) { success = true; }
                else { success = false; }

                if (success) {
                    std::uniform_int_distribution<int> dist(3333, 7777);
                    hackerReward = dist(minerRng);
                    points += hackerReward;

                    hackerMessage = L"Successful hack! (+" + std::to_wstring(hackerReward) + L"$)";
                    hackerMessageTime = now;
                }
                else {
                    hackerMessage = L"Unfortunate";
                    hackerMessageTime = now;
                }

                hackerCountdown[hacker] = GethackerCountdown(hacker);
                hackerStartTime = now;
                hackerDispFraction = 0.0f;
            }
        }
        lastHackerTick += seconds(elapsedHacker);
    }

    ImDrawList* drawList = ImGui::GetWindowDrawList();

	// left column with menu buttons

    const float leftWidth = 170.0f;
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.08f, 0.08f, 0.08f, 1.0f)); 
    ImGui::BeginChild("##left_col", ImVec2(leftWidth, 0.0f), false);
    ImGui::Spacing();
    ImVec2 btnSize(leftWidth - 16.0f, 37.0f);
    const float btnOffset = 8.0f;

    auto TabButton = [&](const char* id, const char* label, const char* icon, int tabIndex) -> bool {
        ImVec4 activeBg = ImVec4(0.9f, 0.4f, 0.4f, 1.0f); // цвет для активной кнопки
        ImVec4 normalBg = ImGui::GetStyle().Colors[ImGuiCol_Button];
        bool pushed = false;
        if (active_tab == tabIndex) { ImGui::PushStyleColor(ImGuiCol_Button, activeBg); pushed = true; }
        bool clicked = IconButtonWithLabel(id, label, icon, btnSize);
        if (pushed) ImGui::PopStyleColor();
        return clicked;
        };

    ImGui::SetCursorPosX(btnOffset);
    ImGui::SetCursorPosY(30);

    if (TabButton("btn_main", "Main", "menu_main", 0)) active_tab = 0;
    ImGui::Spacing();
    ImGui::SetCursorPosX(btnOffset);
    if (TabButton("btn_manual", "Manual", "menu_manual", 1)) active_tab = 1;
    ImGui::Spacing();
    ImGui::SetCursorPosX(btnOffset);
    if (TabButton("btn_auto", "Auto", "menu_auto", 2)) active_tab = 2;
    ImGui::Spacing();
    ImGui::SetCursorPosX(btnOffset);
    if (TabButton("btn_miner", "Miner", "menu_miner", 3)) active_tab = 3;
    ImGui::Spacing();
    ImGui::SetCursorPosX(btnOffset);
    if (TabButton("btn_hacker", "Hacker", "menu_hacker", 4)) active_tab = 4;
    ImGui::Spacing();
    ImGui::SetCursorPosX(btnOffset);
    if (TabButton("bth_pets", "Pets", "menu_pets", 5)) active_tab = 5;
    ImGui::Spacing();
    ImGui::SetCursorPosX(btnOffset);
    if (TabButton("btn_credits", "Credits", "menu_credits", 6)) active_tab = 6;
    ImGui::Spacing();
    ImGui::SetCursorPosX(btnOffset);
    if (TabButton("btn_changelog", "Changelog", "menu_changelog", 7)) active_tab = 7;
    ImGui::Spacing();
    ImGui::SetCursorPosX(btnOffset);
    if (TabButton("btn_settings", "Settings", "menu_settings", 8)) active_tab = 8;
    ImGui::EndChild();
    ImGui::PopStyleColor();

    //main menu

    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.01f, 0.01f, 0.01f, 1.0f));
    ImGui::BeginChild("##right_col", ImVec2(0.0f, 0.0f), false);

	// background particle system

    ImVec2 cursorPos = ImGui::GetIO().MousePos;
    for (int i = 0; i < numParticles; ++i)
    {
        // draw lines to particles
        for (int j = i + 1; j < numParticles; ++j)
        {
            float distance = std::hypotf(particlePositions[j].x - particlePositions[i].x, particlePositions[j].y - particlePositions[i].y);
            float opacity = 0.9f - (distance / 55.0f);  

            if (opacity > 0.0f)
            {
                ImU32 lineColor = ImGui::GetColorU32(ImVec4(1.0f, 1.0f, 1.0f, opacity));
                drawList->AddLine(particlePositions[i], particlePositions[j], lineColor);
            }
        }

        // draw lines to cursor
        float distanceToCursor = std::hypotf(cursorPos.x - particlePositions[i].x, cursorPos.y - particlePositions[i].y);
        float opacityToCursor = 1.0f - (distanceToCursor / 52.0f);  

        if (opacityToCursor > 0.0f)
        {
            ImU32 lineColorToCursor = ImGui::GetColorU32(ImVec4(1.0f, 1.0f, 1.0f, opacityToCursor));
            drawList->AddLine(cursorPos, particlePositions[i], lineColorToCursor);
        }
    }
    float deltaTime = ImGui::GetIO().DeltaTime;
    for (int i = 0; i < numParticles; ++i)
    {
        particlePositions[i].x += particleVelocities[i].x * deltaTime;
        particlePositions[i].y += particleVelocities[i].y * deltaTime;

        if (particlePositions[i].x < ImGui::GetWindowPos().x)
            particlePositions[i].x = ImGui::GetWindowPos().x + ImGui::GetWindowSize().x;
        else if (particlePositions[i].x > ImGui::GetWindowPos().x + ImGui::GetWindowSize().x)
            particlePositions[i].x = ImGui::GetWindowPos().x;

        if (particlePositions[i].y < ImGui::GetWindowPos().y)
            particlePositions[i].y = ImGui::GetWindowPos().y + ImGui::GetWindowSize().y;
        else if (particlePositions[i].y > ImGui::GetWindowPos().y + ImGui::GetWindowSize().y)
            particlePositions[i].y = ImGui::GetWindowPos().y;

        ImU32 particleColour = ImGui::ColorConvertFloat4ToU32(settings::particleColour);

        drawList->AddCircleFilled(particlePositions[i], 1.5f, particleColour);
    }

	// points animation

    ImGuiIO& io = ImGui::GetIO();

    float dt = io.DeltaTime;
    if (dt <= 0.0f)
        dt = 1.0f / 60.0f;
    dt = std::clamp(dt, 0.0f, 0.1f);

    float diff = points - pointsVisual;

    if (std::fabs(diff) < 0.5f)
    {
        pointsVisual = static_cast<float>(points);
    }
    else
    {
        float kUp = 11.0f; 
        float kDown = 12.0f; 
        if (diff > 0.0f)
        {
            pointsVisual += diff * kUp * dt;
            if (points - pointsVisual < 0.5f) pointsVisual = static_cast<float>(points);
        }
        else 
        {
            pointsVisual += diff * kDown * dt; 
            if (points - pointsVisual > -0.5f) pointsVisual = static_cast<float>(points);
        }
    }

	// main tab (0)

    if (active_tab == 0)
    {
        
        std::string txtpoints = "Points:" + std::to_string(int(pointsVisual));
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.05f, 0.05f, 0.05f, 0.9f));
        ImGui::SetCursorPos(ImVec2(dynamicWindowWidth / 10, dynamicWindowHeight - 430));
		ImGui::BeginChild("##points", ImVec2(400.0f, 137.0f), false);
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Spacing();
        std::string btnLabel = "Click me to get " + std::to_string(currentPPC) + (currentPPC == 1 ? " point" : " points") + "##click_button";
		CenteredText(txtpoints.c_str(), BigFont);

        ImGui::Spacing();
        ImGui::Spacing();
		ImGui::SetCursorPosX(55);
        {
			// animation on click ( 100% not AI, trust me (pls) )
            std::string visibleText = "Click me to get " + std::to_string(currentPPC) + (currentPPC == 1 ? " point" : " points");
            const char* buttonIdOnly = "##click_button"; //button id
            static std::unordered_map<std::string, std::chrono::steady_clock::time_point> lastClick;
            const float animDur = 0.12f; // duration in secs
			const float minScale = 0.92f; // minimum scale at click moment

            float scale = 1.0f;
            std::string key = "click_button";

            auto it = lastClick.find(key);
            if (it != lastClick.end()) {
                float elapsed = std::chrono::duration_cast<std::chrono::duration<float>>(now - it->second).count();
                if (elapsed < animDur) {
                    float t = elapsed / animDur; // 0..1
                    scale = std::clamp(minScale + (1.0f - minScale) * t, minScale, 1.0f);
                }
                else {
                    lastClick.erase(it);
                }
            }

            ImVec2 baseSize(299, 50);
            ImVec2 animatedSize(baseSize.x * scale, baseSize.y * scale);
            ImVec2 curPos = ImGui::GetCursorPos();
            ImVec2 screenBase = ImGui::GetCursorScreenPos();
            ImVec2 offset((baseSize.x - animatedSize.x) * 0.5f, (baseSize.y - animatedSize.y) * 0.5f);

            ImGui::SetCursorPos(ImVec2(curPos.x + offset.x, curPos.y + offset.y));

            if (ImGui::Button(buttonIdOnly, animatedSize)) {
                if (isAudioInitialized) {
                    if (idx >= 0 && idx < static_cast<int>(clicksounds.size())) {
                        bool played = PlaySoundFromWidePath(clicksounds[idx]);
                    }
                }
                points += currentPPC;
                lastClick[key] = now;
            }
            ImGui::SetCursorPos(ImVec2(curPos.x, curPos.y + baseSize.y));

            ImDrawList* dl = ImGui::GetWindowDrawList();
            ImFont* font = ImGui::GetFont();
            float baseFontSize = ImGui::GetFontSize();
            float textFontSize = baseFontSize * scale; 

            ImVec2 textSize = font->CalcTextSizeA(textFontSize, FLT_MAX, 0.0f, visibleText.c_str());
            ImVec2 textPos = ImVec2(
                screenBase.x + offset.x + (animatedSize.x - textSize.x) * 0.5f,
                screenBase.y + offset.y + (animatedSize.y - textSize.y) * 0.5f
            );
            dl->AddText(font, textFontSize, textPos, ImGui::GetColorU32(ImGuiCol_Text), visibleText.c_str());

            ImGui::SetCursorPos(ImVec2(curPos.x, curPos.y + baseSize.y));
        }
        ImGui::SetCursorPosX(95);
		std::string pps = "Points per second: " + std::to_string(currentPPS);
		CenteredText(pps.c_str());
        ImGui::Spacing();
        ImGui::EndChild();
        ImGui::PopStyleColor();

       
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.05f, 0.05f, 0.05f, 0.9f));
        ImGui::SetCursorPos(ImVec2(dynamicWindowWidth / 50, dynamicWindowHeight - 250));
        ImGui::BeginChild("##miner", ImVec2(500.0f, 80.0f), false);
        if (miner > 0) { 
            float startX = ImGui::GetCursorPosX();
            ImGui::Spacing();
            ImGui::Spacing();
            ImGui::Spacing();
            using namespace std::chrono;
            auto ms = duration_cast<milliseconds>(now.time_since_epoch()).count();
            bool showFirst = ((ms / 500) % 2) == 0;
            const float iconSize = 67.0f;
            const char* iconName = showFirst ? "pickaxe1" : "pickaxe2";
            RenderIcon(iconName, ImVec2(iconSize, iconSize));
            ImGui::SameLine();
            ImGui::Text("Miner's timer: %d", minerCountdown[miner]);
            float indent = startX + iconSize + ImGui::GetStyle().ItemSpacing.x;
            ImGui::SetCursorPos(ImVec2(indent, dynamicWindowHeight - 460));

            int totalSec = GetminerCountdown(miner);
            float targetFraction = 0.0f;
            if (totalSec > 0)
            {
                float elapsedSec = std::chrono::duration_cast<std::chrono::duration<float>>(now - minerStartTime).count();
                targetFraction = std::clamp(elapsedSec / static_cast<float>(totalSec), 0.0f, 1.0f);
            }

            float lerpSpeed = 8.0f;
            minerDispFraction += (targetFraction - minerDispFraction) * (std::min)(dt * lerpSpeed, 1.0f);

            ImGui::ProgressBar(minerDispFraction, ImVec2(ImGui::GetContentRegionAvail().x, 15.0f), " ");
			ImGui::Spacing();
        }
        else {
			ImGui::Text("Miner is locked now :(");
        }
        if (!minerMessage.empty())
        {
            auto shown = duration_cast<seconds>(now - minerMessageTime).count();
            if (shown < minerMessageDurationSec)
            {
                std::string utf = WideToUtf8(minerMessage);
                ImGui::SetCursorPosX(76);
                ImGui::Text("%s", utf.c_str());
                
            }
            else
            {
                minerMessage.clear();
            }
        }
        ImGui::EndChild();
        ImGui::PopStyleColor();

        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.05f, 0.05f, 0.05f, 0.9f));
        ImGui::SetCursorPos(ImVec2(dynamicWindowWidth / 50, dynamicWindowHeight - 150));
        ImGui::BeginChild("##hacker", ImVec2(500.0f, 80.0f), false);

        if (hacker > 0) {
            float startX = ImGui::GetCursorPosX();
            ImGui::Spacing();
            ImGui::Spacing();
            ImGui::Spacing();
            using namespace std::chrono;
            auto ms = duration_cast<milliseconds>(now.time_since_epoch()).count();
            int idx = static_cast<int>((ms / 250) % 4);
            const float iconSize = 55.0f;
            static const char* pickaxeNames[4] = { "binar1", "binar2", "binar3", "binar4" };
            const char* iconName = pickaxeNames[idx];
            RenderIcon(iconName, ImVec2(iconSize, iconSize));
            ImGui::SameLine();
            ImGui::SetCursorPosX(76);
            ImGui::Text("Hacker's timer: %d", hackerCountdown[hacker]);

            float indent = startX + 67 + ImGui::GetStyle().ItemSpacing.x;
            ImGui::SetCursorPos(ImVec2(indent, dynamicWindowHeight - 460));

            int totalSec = GethackerCountdown(hacker);
            float targetFraction = 0.0f;
            if (totalSec > 0)
            {
                float elapsedSec = std::chrono::duration_cast<std::chrono::duration<float>>(now - hackerStartTime).count();
                targetFraction = std::clamp(elapsedSec / static_cast<float>(totalSec), 0.0f, 1.0f);
            }

            float lerpSpeed = 8.0f;
            hackerDispFraction += (targetFraction - hackerDispFraction) * (std::min)(dt * lerpSpeed, 1.0f);

            ImGui::ProgressBar(hackerDispFraction, ImVec2(ImGui::GetContentRegionAvail().x, 15.0f), " ");
        }
        else {
            ImGui::Text("Hacker is locked now :(");
        }
        if (!hackerMessage.empty())
        {
            auto shown2 = duration_cast<seconds>(now - hackerMessageTime).count();
            if (shown2 < hackerMessageDurationSec)
            {
                std::string utf = WideToUtf8(hackerMessage);
                ImGui::SetCursorPosX(76);
                ImGui::Text("%s", utf.c_str());
            }
            else
            {
                hackerMessage.clear();
            }
        }
        ImGui::EndChild();
        ImGui::PopStyleColor();

        ImGuiIO& io = ImGui::GetIO();
        ImGui::PushFont(MiniFont);
		ImGui::SetCursorPos(ImVec2(dynamicWindowWidth / 2.5, dynamicWindowHeight - 52));
        ImGui::Text("Vertices: %d", io.MetricsRenderVertices);
		ImGui::SameLine();
        ImGui::Text("Indices: %d", io.MetricsRenderIndices);
		ImGui::PopFont();
        

    }
/*manual*/	else if (active_tab == 1)
    {

        if (manual == 1) {
            ImGui::Text("Manual click upgrade (50$)");
            ImGui::SameLine();
            if (ImGui::Button("Buy it!"))
            {
                Upgrade(manual, 50, 2);
            }
            ImGui::Text("Manual click 1 -> 2");
        }
        else if (manual == 2) {
            ImGui::Text("Manual click upgrade (500$)");
            ImGui::SameLine();
            if (ImGui::Button("Buy it!"))
            {
                Upgrade(manual, 500, 3);
            }
            ImGui::Text("Manual click 2 -> 5");
		}
        else if (manual == 3) {
            ImGui::Text("Manual click upgrade (1.997$)");
            ImGui::SameLine();
            if (ImGui::Button("Buy it!"))
            {
                Upgrade(manual, 1997, 4);
  
            }
            ImGui::Text("Manual click 5 -> 12");
        }
        else if (manual == 4) {
            ImGui::Text("Manual click upgrade (7.777$)");
            ImGui::SameLine();
            if (ImGui::Button("Buy it!"))
            {
                Upgrade(manual, 7777, 5);
            }
            ImGui::Text("Manual click 12 -> 24");
        }

        else if (manual == 5) {
            ImGui::Text("Manual click upgrade (27.999$)");
            ImGui::SameLine();
            if (ImGui::Button("Buy it!"))
            {
                Upgrade(manual, 27999, 6);
            }
            ImGui::Text("Manual click 24 -> 40");
        }
        else {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
            ImGui::Text("Manual clicks have been fully upgraded!");
            ImGui::PopStyleColor();
        }
        ImGui::SetCursorPos(ImVec2(dynamicWindowWidth / 3.5, dynamicWindowHeight - 70));
        ImGui::Text("Points:%d", static_cast<int>(pointsVisual));
    } 
/*auto*/    else if (active_tab == 2)
    {
        if (iauto == 0) {
            ImGui::Text("Unlock autoclicker (100$)");
            ImGui::SameLine();
            if (ImGui::Button("Buy it!"))
            {
                Upgrade(iauto, 100, 1);
            }
            ImGui::Text("Your first way to AFK farm! (3$ per second)");
		}
        else if (iauto == 1) {
            ImGui::Text("Upgrade autoclicker (777$)");
            ImGui::SameLine();
            if (ImGui::Button("Buy it!"))
            {
                Upgrade(iauto, 777, 2);
            }
            ImGui::Text("Upgrade: 3$ per sec. -> 8$ per sec.");
        }
        else if (iauto == 2) {
            ImGui::Text("Upgrade autoclicker (6.666$)");
            ImGui::SameLine();
            if (ImGui::Button("Buy it!"))
            {
                Upgrade(iauto, 6666, 3);
            }
            ImGui::Text("Upgrade: 8$ per sec. -> 16$ per sec.");
        }
        else if (iauto == 3) {
            ImGui::Text("Upgrade autoclicker (17.000$)");
            ImGui::SameLine();
            if (ImGui::Button("Buy it!"))
            {
                Upgrade(iauto, 17000, 4);
            }
            ImGui::Text("Upgrade: 16$ per sec. -> 33$ per sec.");
        }
        else if (iauto == 4) {
            ImGui::Text("Upgrade autoclicker (70.000$)");
            ImGui::SameLine();
            if (ImGui::Button("Buy it!"))
            {
                Upgrade(iauto, 70000, 5);
            }
            ImGui::Text("Upgrade: 33$ per sec. -> 67$ per sec.");
        }
        else if (iauto == 5) {
            ImGui::Text("Upgrade autoclicker (166.666$)");
            ImGui::SameLine();
            if (ImGui::Button("Buy it!"))
            {
                Upgrade(iauto, 166666, 6);
            }
            ImGui::Text("Upgrade: 67$ per sec. -> 100$ per sec.");
        }
        else {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
            ImGui::Text("Autoclicker has been fully upgraded!");
            ImGui::PopStyleColor();
        }
        ImGui::SetCursorPos(ImVec2(dynamicWindowWidth / 3.5, dynamicWindowHeight - 70));
        ImGui::Text("Points:%d", static_cast<int>(pointsVisual));
    }
/*Miner*/  else if (active_tab == 3)
    {
        if (miner == 0) {
            ImGui::Text("Unlock miner (2.500$)");
            ImGui::SameLine();
            if (ImGui::Button("Buy it!")) {
                if (points >= 2500)
                {
                    points -= 2500;
                    miner = 1;
                    minerCountdown[miner] = GetminerCountdown(miner);
                    minerStartTime = now;
				}
                else {
					cantafford = true;
                }
            }
            ImGui::Text("Miner stats:\nOne ore every 20 seconds!\nCoal - 60%% (150 points)\nIron - 25%% (300 points)\nGold - 10%% (600 points)\nDiamond - 4%% (1300 points)\nEmerald - 1%% (3000 points)");
        }
        else if (miner == 1) {
            ImGui::Text("Upgrade miner (27.000$)");
            ImGui::SameLine();
            if (ImGui::Button("Buy it!")) {
                if (points >= 27000)
                {
                    points -= 27000;
                    miner = 2;
                    minerCountdown[miner] = GetminerCountdown(miner);
                    minerStartTime = now;
                }
                else {
                    cantafford = true;
                }
            }
            ImGui::Text("Miner upgrades:\nOne ore every 20 -> 15 seconds!\nCoal - 60%% -> 55%% (150 -> 301 points)\nIron - 25%% -> 28%% (300 -> 600 points)\nGold - 10%% -> 12%% (600 -> 1000 points)\nDiamond - 4%% -> 4%% (1300 -> 3000 points)\nEmerald - 1%% -> 1%% (3000 -> 10000 points)");
        }
        else {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
			ImGui::Text("Miner has been fully upgraded!");
            ImGui::PopStyleColor();
        }
        ImGui::SetCursorPos(ImVec2(dynamicWindowWidth / 3.5, dynamicWindowHeight - 70));
        ImGui::Text("Points:%d", static_cast<int>(pointsVisual));
    }
/*Hacker*/ else if (active_tab == 4)
    {
        if (hacker == 0) {
            ImGui::Text("Unlock hacker (30.777$)");
            ImGui::SameLine();
            if (ImGui::Button("Buy it!")) {
                if (points >= 30777)
                {
                    points -= 30777;
                    hacker = 1;
                    hackerCountdown[hacker] = GethackerCountdown(hacker);
					hackerStartTime = now;
                }
                else {
                    cantafford = true;
                }
            }
            ImGui::Text("Hacker stats:\nOne Hack attemp every 25 seconds\nChance of a successful hack -> 70%%\nPoints pool upon success -> 1.500$ - 4.500$");
        }
        else if (hacker == 1) {
            ImGui::Text("Upgrade hacker (77.777$)");
            ImGui::SameLine();
            if (ImGui::Button("Buy it!")) {
                if (points >= 77777)
                {
                    points -= 77777;
                    hacker = 2;
                    hackerCountdown[hacker] = GethackerCountdown(hacker);
                    hackerStartTime = now;
                }
                else {
                    cantafford = true;
                }
            }
            ImGui::Text("Hacker upgrade:\nOne Hack attemp every 25 -> 20 seconds\nChance of a successful hack 70%% -> 80%%\nPoints pool upon success -> 3.333$ - 7.777$");
        }
        else {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
            ImGui::Text("Hacker has been fully upgraded!");
            ImGui::PopStyleColor();
        }
        ImGui::SetCursorPos(ImVec2(dynamicWindowWidth / 3.5, dynamicWindowHeight - 70));
        ImGui::Text("Points:%d", static_cast<int>(pointsVisual));
    }
    /*pets*/    else if (active_tab == 5)
    {

        {
            auto elapsedPets = std::chrono::duration_cast<std::chrono::seconds>(now - lastPetsTick).count();
            if (elapsedPets > 0) {
                if (petsCooldownRemaining > 0) {
                    petsCooldownRemaining = std::max(0, petsCooldownRemaining - static_cast<int>(elapsedPets));
                }
                lastPetsTick += std::chrono::seconds(elapsedPets);
            }
        }

        for (int i = 0; i < pets.size(); i++) {
            Pet& pet = pets[i];

            ImGui::Text("%s - %d$", pet.name.c_str(), pet.price);
            ImGui::SameLine();

            if (pet.owned) {
                ImGui::TextColored(ImVec4(0, 1, 0, 1), "Owned");
                ImGui::SameLine();
                if (equiped_pet == i) {
                    if (ImGui::Button("Unequip")) {
                        equiped_pet = -1;
                    }
                }
                else {
                    if (ImGui::Button(("Equip##" + std::to_string(i)).c_str())) {
                        if (petsCooldownRemaining <= 0) {
                            equiped_pet = i;
                            if (miner > 0 && miner < 3) {
                                int newVal = minerCountdown[miner] - 7;
                                minerCountdown[miner] = (newVal > 1) ? newVal : 1;
                            }
                            if (hacker > 0 && hacker < 3) {
                                int newVal2 = hackerCountdown[hacker] - 10;
                                hackerCountdown[hacker] = (newVal2 > 1) ? newVal2 : 1;
                            }
                            petsCooldownRemaining = 7;
                            petsMessage.clear();
                        }
                        else {
                            petsMessage = L"You can't switch pets so fast! Please wait a bit";
                            showpetmsg = true;
                        }
                    }
                }
            }
            else if (equiped_pet != i) {
                if (ImGui::Button(("Buy it!##" + std::to_string(i)).c_str())) {
                    buyPet(pets, i, points);
                }
            }
        }


        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::Spacing();



        ImGui::Text("Pets stats:");

        
        //dog secription
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.1f, 1.0f), "Dog >"); ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.1f, 1.0f, 0.1f, 1.0f), "+3 points per click"); ImGui::Spacing();

        //HelloWorld() secription
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.1f, 1.0f), "HelloWorld() >"); ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.1f, 1.0f, 0.1f, 1.0f), "+5 points per sec."); ImGui::Spacing();


        //brooo description
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.1f, 1.0f), "Brooo >"); ImGui::SameLine();
        ImGui::Text("Miner's cooldown is"); ImGui::SameLine();
		ImGui::TextColored(ImVec4(0.1f, 1.0f, 0.1f, 1.0f), "reduced by 7 seconds."); ImGui::Spacing();


        //plushie chance description
		ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.1f, 1.0f), "Plushie Chance >"); ImGui::SameLine();
        ImGui::Text("Manual clicks and autoclicker becomes");
        ImGui::TextColored(ImVec4(1.0f, 0.1f, 0.1f, 1.0f), "zero"); ImGui::SameLine();
        ImGui::TextUnformatted(", but hacker's cooldown is"); ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.1f, 1.0f, 0.1f, 1.0f), "reduced by 10 seconds."); ImGui::Spacing();

        //spongebob description
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.1f, 1.0f), "Sponge Bob >"); ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.1f, 1.0f, 0.1f, 1.0f), "+25 points per click, +25 points per sec.");


        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::Spacing();
        if (equiped_pet >= 0 && equiped_pet < (int)pets.size()) {
			ImGui::Text("Equipped pet:"); ImGui::SameLine();
			ImGui::TextColored(ImVec4(0.1f, 1.0f, 0.1f, 1.0f), "%s", pets[equiped_pet].name.c_str());
        }
        else {
			ImGui::Text("Equipped pet:"); ImGui::SameLine();
			ImGui::TextColored(ImVec4(1.0f, 0.1f, 0.1f, 1.0f), "none");
        }
        if (showpetmsg) {
            ImGui::OpenPopup("Wait-wait-wait");
			showpetmsg = false;
        }
        if(ImGui::BeginPopupModal("Wait-wait-wait", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Text("%s", WideToUtf8(petsMessage).c_str());
			ImGui::Separator();
            if (ImGui::Button("OK", ImVec2(120, 0)))
            {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
		}

        ImGui::SetCursorPos(ImVec2(dynamicWindowWidth / 3.5, dynamicWindowHeight - 70));
        ImGui::Text("Points:%d", static_cast<int>(pointsVisual));
    }
/*credits*/    else if (active_tab == 6) {
        ImGui::Text("Developed by:");
		ImGui::SameLine();
		ImGui::TextColored(ImVec4(0.9f, 0.1f, 0.0f, 1.0f), "qweowix");
        ImGui::Text("Assets by:");
		ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.9f, 0.1f, 0.0f, 1.0f), "SimpleAnton");
        ImGui::Text("\n#qweowixGOATED");
        ImGui::SetCursorPos(ImVec2(dynamicWindowWidth / 3.5, dynamicWindowHeight - 70));
        ImGui::Text("Points:%d", static_cast<int>(pointsVisual));
    }
 /*changelog*/   else if (active_tab == 7) {
        ImGui::Text("v0.1 (17.02.2026)\n>>> Release!\n\n");
        ImGui::SetCursorPos(ImVec2(dynamicWindowWidth / 3.5, dynamicWindowHeight - 70));
        ImGui::Text("Points:%d", static_cast<int>(pointsVisual));
    }
  /*settings*/  else if (active_tab == 8) {
        
        static const char* sounds[] = { "Click 1", "Click 2", "Click 3", "Punch", "Boom", "Disable"};
        if (ImGui::Button("Reset all stats", ImVec2(300, 30))) {
            showConfirmReset = true;
        }
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.12f, 0.12f, 0.12f, 0.95f));
        ImGui::PushStyleColor(ImGuiCol_SliderGrab, ImVec4(0.90f, 0.20f, 0.20f, 1.00f));
        ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, ImVec4(1.00f, 0.40f, 0.40f, 1.00f));
        ImGui::SetNextItemWidth(250);
        ImGui::SliderFloat("Click sound volume", &clickVolume, 0.0f, 1.0f, "%.2f", ImGuiSliderFlags_NoInput);
        ImGui::SetNextItemWidth(250);
        ImGui::Combo("Select click sound", &idx, sounds, IM_ARRAYSIZE(sounds));
        ImGui::PopStyleColor(3);
		


        ImGui::SetCursorPos(ImVec2(dynamicWindowWidth / 3.5, dynamicWindowHeight - 70));
        ImGui::Text("Points:%d", static_cast<int>(pointsVisual));
        
    }
    if (showConfirmReset)
    {
        ImGui::OpenPopup("Confirm Reset");
        showConfirmReset = false;         
    }

   
    if (ImGui::BeginPopupModal("Confirm Reset", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Are you sure you want to reset all progress?");
        ImGui::Separator();

        if (ImGui::Button("Yes", ImVec2(120, 0)))
        {
            points = 0;
            pointsVisual = 0.0f;
            manual = 1;
            iauto = 0;
            miner = 0;
            hacker = 0;
            equiped_pet = -1;
            for (auto& p : pets) p.owned = false;
            petsCooldownRemaining = 0;
            petsMessage.clear();
            showpetmsg = false;
            minerStartTime = std::chrono::steady_clock::now();
            hackerStartTime = std::chrono::steady_clock::now();
            minerDispFraction = 0.0f;
            hackerDispFraction = 0.0f;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("No", ImVec2(120, 0)))
        {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    ImGui::EndChild();
    ImGui::PopStyleColor();

    ImGui::End();
}