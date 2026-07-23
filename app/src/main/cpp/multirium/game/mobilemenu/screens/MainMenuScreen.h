//
// Created by viski on 03.06.2025.
//

#ifndef MULTIRIUM_MAINMENUSCREEN_H
#define MULTIRIUM_MAINMENUSCREEN_H

#include "FlowScreen.h"
#include "../main.h"

class MainMenuScreen : FlowScreen {
public:
    static void AddAllItems(MainMenuScreen *thiz);

    static void OnResume();
    static void OnSettings();
    static void OnStartGame();
    static void OnExit();

    static bool HasCPSave(MainMenuScreen *thiz);

    static void InjectHooks();
};


#endif //MULTIRIUM_MAINMENUSCREEN_H
