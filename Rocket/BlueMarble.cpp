﻿// Rocket.cpp : 애플리케이션에 대한 진입점을 정의합니다.
//

#include "framework.h"
#include "BlueMarble.h"
#include "Engine.h"
#include "IfError.h"
#include <string>
#include <iostream>

#ifdef _DEBUG
#ifdef UNICODE                                                                                      
    #pragma comment(linker, "/entry:wWinMainCRTStartup /subsystem:console") 
#else                                                                                                    
    #pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")   
#endif                                                                                                   
#endif                                                                                                   

#pragma comment(lib, "jsoncpp.lib")

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    try
    {
        Engine EngineInstance(hInstance);

        EngineInstance.Initialize();

        EngineInstance.Run();

        return 0;
    }
    catch(const IfError& e)
    {
        MessageBox(nullptr, e.errorMsg.c_str(), nullptr, 0);
        return 0;
    }
}