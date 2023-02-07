// Rocket.cpp : 애플리케이션에 대한 진입점을 정의합니다.
//

#include "framework.h"
#include "Rocket.h"
#include "Game.h"
#include <string>

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    try
    {
        Game gameInstance(hInstance);

        gameInstance.Initialize();

        gameInstance.Run();

        return 0;
    }
    catch(const std::string& e)
    {

        return 0;
    }
}