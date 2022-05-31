#pragma once

#include <Windows.h>
#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include <sstream>
#include <string>
#include <corecrt_math.h>
#include <map>
#include <vector>
#include <string.h>
#include <winbase.h>
#include <tchar.h>
#include <WinInet.h>



#include <d3d9.h>

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "imgui/imgui_impl_dx9.h"
#include "imgui/imgui_impl_win32.h"



#pragma comment(lib, "d3d9.lib")

LPDIRECT3DDEVICE9        g_pd3dDevice;
D3DPRESENT_PARAMETERS    g_d3dpp;
LPDIRECT3D9              g_pD3D;

class var_t
{
public:

	bool loader_active = true;
	HWND main_hwnd = nullptr;

} var ;

class dx9_t
{
public:

	bool create_device(HWND hwnd)
	{
        if ((g_pD3D = Direct3DCreate9(D3D_SDK_VERSION)) == NULL)
            return false;

        // Create the D3DDevice
        ZeroMemory(&g_d3dpp, sizeof(g_d3dpp));
        g_d3dpp.Windowed = TRUE;
        g_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
        g_d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
        g_d3dpp.EnableAutoDepthStencil = TRUE;
        g_d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
        g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;           // Present with vsync
        //g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;   // Present without vsync, maximum unthrottled framerate
        if (g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hwnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &g_d3dpp, &g_pd3dDevice) < 0)
            return false;
        return true;
	}

    void cleanup_device()
    {
        if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = NULL; }
        if (g_pD3D) { g_pD3D->Release(); g_pD3D = NULL; }
    }

    void reset_device()
    {
        ImGui_ImplDX9_InvalidateDeviceObjects();
        HRESULT hr = g_pd3dDevice->Reset(&g_d3dpp);
        if (hr == D3DERR_INVALIDCALL)
            IM_ASSERT(0);
        ImGui_ImplDX9_CreateDeviceObjects();
    }

} dx9 ;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED)
        {
            g_d3dpp.BackBufferWidth = LOWORD(lParam);
            g_d3dpp.BackBufferHeight = HIWORD(lParam);
            dx9.reset_device();
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

class gui_t
{
private:

    DWORD WindowFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoDecoration;

    ImFont* f1;
    ImFont* f2;
    ImFont* f3;

    RECT screen_rect;

public:

    int menu_key = VK_INSERT;

    int init(std::vector<const char*> tabs)
    {
        // Create application window
        WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, L"454", NULL };
        RegisterClassEx(&wc);
        var.main_hwnd = CreateWindow(wc.lpszClassName, L"23434265", WS_POPUP, 0, 0, 5, 5, NULL, NULL, wc.hInstance, NULL);

        if (!dx9.create_device(var.main_hwnd)) {
            dx9.cleanup_device();
            UnregisterClass(wc.lpszClassName, wc.hInstance);
            return 1;
        }

        ShowWindow(var.main_hwnd, SW_HIDE);
        UpdateWindow(var.main_hwnd);
        ImGui::CreateContext();

        ImGuiIO& io = ImGui::GetIO(); (void)io;

        ImFont* f1 = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\Tahoma.ttf", 13.3);
        ImFont* f2 = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\Tahoma.ttf", 12.3);
        ImFont* f3 = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\Tahoma.ttf", 14);
        ImFont* f4 = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\Tahoma.ttf", 11.9);

        io.IniFilename = nullptr;
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

        ImGui::StyleColorsDark();

        ImGuiStyle& style = ImGui::GetStyle();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            style.WindowRounding = 0.0f;
            style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        }

        ImGui_ImplWin32_Init(var.main_hwnd);
        ImGui_ImplDX9_Init(g_pd3dDevice);

        DWORD window_flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar;

        RECT screen_rect;
        GetWindowRect(GetDesktopWindow(), &screen_rect);
        auto x = float(screen_rect.right - 300) / 2.f;
        auto y = float(screen_rect.bottom - 400) / 2.f;

        // Main loop
        MSG msg;
        ZeroMemory(&msg, sizeof(msg));
        while (msg.message != WM_QUIT)
        {
            if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
                continue;
            }

            // Start the Dear ImGui frame
            ImGui_ImplDX9_NewFrame();
            ImGui_ImplWin32_NewFrame();
            ImGui::NewFrame();
            {
                ImGui::SetNextWindowSize ( ImVec2 ( 500, 350 ) , ImGuiCond_Always );
                ImGui::Begin ( "playground.cc" , NULL , WindowFlags );

                auto draw = ImGui::GetWindowDrawList ( );
                ImVec2 pos = ImGui::GetWindowPos ( );
                ImVec2 size = ImGui::GetWindowSize ( );

                static int tabb = 0;
                static const char* items[]{ "none", "assault rifle", "mp5", "lr300", "thompson", "customsmg", "m249" };
                static int selectedItem = 0;
                static const char* items2[]{ "none", "simple", "holosight", "8x scope", "16x scope" };
                static int selectedAttach = 0;
                static const char* items3[]{ "none", "suppressor", "muzzle boost" };
                static int selectedMuzzle = 0;
                static int randomisation;
                static bool recoil = false;
                static float sens = 0.65;
                static float aimingSens = 0.5;
                static int xControl = 0;
                static int yControl = 0;
                static int fov = 90;
                static int start = 0;
                static bool hipfire = false;
                static bool afk = false;
                static int afkControl = 100;
                static const char* keybindak[] = { "f1", "f2", "f3", "f4","f5", "f6", "f8", "f9" };
               
                
                static constexpr DWORD color_picker_flags = ImGuiColorEditFlags_AlphaPreview | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoDragDrop | ImGuiColorEditFlags_NoOptions | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_PickerHueBar | ImGuiColorEditFlags_NoBorder ;
                static float menu_color[3] = { 0.345f , 1.f , 0.213f };
                
                //ImGui::ShowDemoWindow();
                
                
               

                /**/
               
                // title 

                ImGui::PushFont(f1);
                draw->AddText({ pos.x + 5, pos.y + 3 }, ImColor(menu_color[0], menu_color[1], menu_color[2]), "midn");
                draw->AddText({ pos.x + 29, pos.y + 3 }, ImColor(255, 255, 0), "ight");
                ImGui::PopFont();
                ImGui::PushFont(f3);
                draw->AddText({ pos.x + 49, pos.y + 3 }, ImColor(188, 141, 194), ".cl");
                draw->AddText({ pos.x + 60, pos.y + 3 }, ImColor(255, 255, 0), "ub");
                ImGui::PopFont();


                 draw->AddRect({ pos.x + 1, pos.y + 1 }, { pos.x + 500 , pos.y + 349 }, ImColor(22, 22, 22));

                 draw->AddRectFilled({ pos.x + 1, pos.y + 20 }, { pos.x + 500, pos.y + 23 }, ImColor(22, 22, 22));
                 
                 

                 
                 draw->AddRectFilledMultiColor({ pos.x + 1, pos.y + 21 }, { pos.x + 500, pos.y + 22 }, ImColor(255, 255, 255), ImColor(55, 255, 255), ImColor(255, 0, 100), ImColor(190, 210, 0));

                 draw->AddRectFilledMultiColor({ pos.x + 1, pos.y + 2 }, { pos.x + 500, pos.y + 22 }, ImColor(255, 255, 255, 3), ImColor(55, 255, 255, 3), ImColor(255, 0, 100, 22), ImColor(190, 210, 0, 22));
               
                 // buttons & tabs

                 ImGui::SetCursorPos(ImVec2(406.000f, 2.000f));
                 if (ImGui::Button("Main", ImVec2(45, 19)))
                     tabb = 0;

                 ImGui::Spacing();
                 ImGui::SetCursorPos(ImVec2(445.000f, 2.000f));
                 if (ImGui::Button("Keybinds", ImVec2(51, 19)))
                     tabb = 1;

                 // main

                 if (tabb == 0)
                 {
                     draw->AddRectFilled({ pos.x + 8, pos.y + 32 }, { pos.x + 257, pos.y + 343 }, ImColor(17, 17, 17, 255));
                     
                     draw->AddRectFilled({ pos.x + 492, pos.y + 32 }, { pos.x + 265, pos.y + 343 }, ImColor(17, 17, 17, 255));

                     draw->AddRectFilledMultiColor({ pos.x + 8, pos.y + 343 }, { pos.x + 256, pos.y + 344 }, ImColor(255, 255, 255), ImColor(55, 255, 255), ImColor(255, 0, 100), ImColor(190, 210, 0));
                     
                     draw->AddRectFilledMultiColor({ pos.x + 493, pos.y + 343 }, { pos.x + 265, pos.y + 344 }, ImColor(255, 255, 255), ImColor(55, 255, 255), ImColor(255, 0, 100), ImColor(190, 210, 0));
                    
                    

                     ImGui::SetNextItemWidth(170.000f);

                     ImGui::SetCursorPos(ImVec2(14.000f, 40.000f));
                     ImGui::Combo("Weapon", &selectedItem, " No Weapon\0 Assault Rifle\0 MP5\0 LR-300 Rifle\0 Thompson\0 Custom-SMG\00 M249");


                     ImGui::SetNextItemWidth(170.f);

                     ImGui::SetCursorPos(ImVec2(14.000f, 70.000f));

                     ImGui::Combo("Sight", &selectedAttach, " No Scope\0 Simple\0 Holosight\0 8x Scope\0 16x Scope");

                     ImGui::SetNextItemWidth(170.f);

                     ImGui::SetCursorPos(ImVec2(14.000f, 100.000f));

                     ImGui::Combo("Muzzle", &selectedMuzzle, " No Muzzle\0 Silencer\0 Muzzle Boost\0");

                     ImGui::SetNextItemWidth(140.0f);

                     ImGui::SetCursorPos(ImVec2(14.000f, 130.000f));
                     ImGui::PushFont(f4);
                     ImGui::SliderInt(" Randomization ", &randomisation, 0, 30);
                     ImGui::PopFont();


                     ImGui::SetNextItemWidth(140.f);

                     ImGui::SetCursorPos(ImVec2(14.000f, 160.000f));
                     ImGui::PushFont(f4);
                     ImGui::SliderInt(" Inaccurate Start ", &start, 0, 100);
                     ImGui::PopFont();

                     ImGui::SetCursorPos(ImVec2(15.000f, 186.000f));

                     ImGui::Checkbox("Enable Recoil Control", &recoil);

                     ImGui::SetCursorPos(ImVec2(15.000f, 210.000f));
                     
                     ImGui::Checkbox("Enable Hipfire Control", &hipfire); 
                     
                     
                     ImGui::SetCursorPos(ImVec2(14.000f, 240.000f));

                     ImGui::SetNextItemWidth(10.0f);
                     ImGui::SetNextItemWidth(2.f);
                     ImGui::ColorEdit3("", menu_color, color_picker_flags);






                     ImGui::SetNextItemWidth(170.000f);

                     ImGui::SetCursorPos(ImVec2(270.000f, 40.000f));
                     ImGui::PushFont(f4);
                     ImGui::SliderInt(" FOV ", &fov, 75, 90);
                     ImGui::PopFont();

                    


                     ImGui::SetNextItemWidth(130.0f);

                     ImGui::SetCursorPos(ImVec2(270.000f, 80.f));
                     ImGui::PushFont(f4);
                     ImGui::SliderFloat("Sensitivity", &sens, .1, 2, "%.2g");
                     ImGui::PopFont();

                    


                     ImGui::SetNextItemWidth(150.0f);

                     ImGui::SetCursorPos(ImVec2(270.000f, 107.f));
                     ImGui::PushFont(f4);
                     ImGui::SliderInt(" X Control ", &xControl, 0, 100);
                     ImGui::PopFont();

                     ImGui::SetNextItemWidth(150.f);

                     ImGui::SetCursorPos(ImVec2(270.000f, 147.f));
                     ImGui::PushFont(f4);
                     ImGui::SliderInt(" Y Control ", &yControl, 0, 100);
                     ImGui::PopFont();

                     ImGui::SetNextItemWidth(115.0f);

                     ImGui::SetCursorPos(ImVec2(270.000f, 187.f));
                     ImGui::Checkbox("Enable Anti-Afk", &afk);

                     ImGui::SetNextItemWidth(115.000f);

                     


                     ImGui::SetCursorPos(ImVec2(270.000f, 227.f));
                     ImGui::PushFont(f4);
                     ImGui::SliderInt(" Afk Interval ", &afkControl, 100, 1500);
                     ImGui::PopFont();
                    
                     
                 }

                 // keybinds

                 if (tabb == 1)
                 {
                     draw->AddRectFilled({ pos.x + 8, pos.y + 32 }, { pos.x + 257, pos.y + 343 }, ImColor(17, 17, 17, 255));

                     draw->AddRectFilled({ pos.x + 492, pos.y + 32 }, { pos.x + 265, pos.y + 343 }, ImColor(17, 17, 17, 255));

                     draw->AddRectFilledMultiColor({ pos.x + 8, pos.y + 343 }, { pos.x + 256, pos.y + 344 }, ImColor(255, 255, 255), ImColor(55, 255, 255), ImColor(255, 0, 100), ImColor(190, 210, 0));

                     draw->AddRectFilledMultiColor({ pos.x + 493, pos.y + 343 }, { pos.x + 265, pos.y + 344 }, ImColor(255, 255, 255), ImColor(55, 255, 255), ImColor(255, 0, 100), ImColor(190, 210, 0));

                     ImGui::Text("Hello World !");

                 }


               

                ImGui::End ( );
            }

            ImGui::EndFrame();

            g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0, 1.0f, 0);
            if (g_pd3dDevice->BeginScene() >= 0)
            {
                ImGui::Render();
                ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
                g_pd3dDevice->EndScene();
            }

            // Update and Render additional Platform Windows
            if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
            {
                ImGui::UpdatePlatformWindows();
                ImGui::RenderPlatformWindowsDefault();
            }

            HRESULT result = g_pd3dDevice->Present(NULL, NULL, NULL, NULL);

            // Handle loss of D3D9 device
            if (result == D3DERR_DEVICELOST && g_pd3dDevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET) {
                dx9.reset_device();
            }
            if (!var.loader_active) {
                msg.message = WM_QUIT;
            }
        }

        ImGui_ImplDX9_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();

        dx9.cleanup_device();
        DestroyWindow(var.main_hwnd);
        UnregisterClass(wc.lpszClassName, wc.hInstance);

        return 0;
    }

} gui ;

