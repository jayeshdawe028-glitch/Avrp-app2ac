//
// Created by viski on 03.06.2025.
//

#include "MainMenuScreen.h"
#include "textures/TextureDatabaseRuntime.h"
#include "patch.h"
#include "Messages.h"

void MainMenuScreen::OnResume()
{
    CHook::CallFunction<void>("_ZN14MainMenuScreen10OnSettingsEv");
  /*  unsigned int v0; // w8
    MenuScreen *v1; // x0
    unsigned int numEntries; // w8
    MenuScreen *v3; // x0

    AddMenuStat(eMenuResume, 1);
    if ( Menu_IsPlayingGame(1) || !CGenericGameStorage::CheckSlotDataValid(lastSaveForResume, 1) )
    {
        MobileMenu::Unload(&gMobileMenu);
        numEntries = gMobileMenu.screenStack.numEntries;
        gMobileMenu.isMapMode = 0;
        gMobileMenu.pendingScreen = 0LL;
        while ( numEntries )
        {
            v3 = gMobileMenu.screenStack.dataPtr[--numEntries];
            gMobileMenu.screenStack.numEntries = numEntries;
            if ( v3 )
            {
                (*((void (__fastcall **)(MenuScreen *))v3->_vptr$MenuScreen + 1))(v3);
                numEntries = gMobileMenu.screenStack.numEntries;
            }
        }
    }
    else
    {
        Menu_LoadSlot(lastSaveForResume);
        MobileMenu::Unload(&gMobileMenu);
        v0 = gMobileMenu.screenStack.numEntries;
        gMobileMenu.isMapMode = 0;
        gMobileMenu.pendingScreen = 0LL;
        while ( v0 )
        {
            v1 = gMobileMenu.screenStack.dataPtr[--v0];
            gMobileMenu.screenStack.numEntries = v0;
            if ( v1 )
            {
                (*((void (__fastcall **)(MenuScreen *))v1->_vptr$MenuScreen + 1))(v1);
                v0 = gMobileMenu.screenStack.numEntries;
            }
        }
    }
    Menu_SwitchOffToGame();
    if ( Menu_IsPlayingGame(1) )
        skipFrame = 2;
    gMobileMenu.CurrentGameNotResumable = 0;*/
}

void MainMenuScreen::OnSettings() {
    CHook::CallFunction<void>("_ZN14MainMenuScreen10OnSettingsEv");
}

void MainMenuScreen::OnStartGame() {
    CHook::CallFunction<void>("_ZN14MainMenuScreen11OnStartGameEv");
}

bool MainMenuScreen::HasCPSave(MainMenuScreen *thiz) {
    return CHook::CallFunction<bool>("_ZN14MainMenuScreen9HasCPSaveEv", thiz);
}

void MainMenuScreen::OnExit()
{
    RsGlobal->quit = 1;
}

void MainMenuScreen::AddAllItems(MainMenuScreen *thiz)
{
  RwTexture *Texture; // x20
  unsigned int *p_numEntries; // x23
  uint64_t numEntries; // x25
  OSArray<FlowScreen::MenuItem> *p_items; // x24
  unsigned int v6; // w26
  FlowScreen::MenuItem *v7; // x0
  FlowScreen::MenuItem *v8; // x22
  FlowScreen::MenuItem *dataPtr; // x21
  RwTexture *v10; // x20
  uint64_t v11; // x25
  unsigned int v12; // w26
  FlowScreen::MenuItem *v13; // x0
  FlowScreen::MenuItem *v14; // x22
  FlowScreen::MenuItem *v15; // x21
  FlowScreen::MenuItem *v16; // x10
  RwTexture *v17; // x20
  uint64_t v18; // x25
  unsigned int v19; // w26
  FlowScreen::MenuItem *v20; // x0
  FlowScreen::MenuItem *v21; // x22
  FlowScreen::MenuItem *v22; // x21
  FlowScreen::MenuItem *v23; // x10
  RwTexture *v24; // x20
  uint64_t v25; // x25
  unsigned int v26; // w26
  FlowScreen::MenuItem *v27; // x0
  FlowScreen::MenuItem *v28; // x22
  FlowScreen::MenuItem *v29; // x21
  FlowScreen::MenuItem *v30; // x8
  RwTexture *v31; // x20
  uint64_t v32; // x25
  unsigned int v33; // w26
  FlowScreen::MenuItem *v34; // x0
  FlowScreen::MenuItem *v35; // x22
  FlowScreen::MenuItem *v36; // x21
  FlowScreen::MenuItem *v37; // x8
  RwTexture *v38; // x20
  uint64_t v39; // x25
  unsigned int v40; // w26
  FlowScreen::MenuItem *v41; // x0
  FlowScreen::MenuItem *v42; // x22
  FlowScreen::MenuItem *v43; // x21
  FlowScreen::MenuItem *v44; // x8
  void (__cdecl **p_selected)(); // x8
  void (__cdecl *v46)(); // x9
  FlowScreen::MenuItem *v47; // x8
  RwTexture *v48; // x20
  uint64_t v49; // x25
  unsigned int v50; // w26
  FlowScreen::MenuItem *v51; // x0
  FlowScreen::MenuItem *v52; // x22
  FlowScreen::MenuItem *v53; // x21
  FlowScreen::MenuItem *v54; // x10
  RwTexture *v55; // x20
  uint64_t v56; // x25
  unsigned int v57; // w26
  FlowScreen::MenuItem *v58; // x0
  FlowScreen::MenuItem *v59; // x22
  FlowScreen::MenuItem *v60; // x21
  FlowScreen::MenuItem *v61; // x8
  RwTexture *v62; // x20
  uint64_t v63; // x25
  unsigned int v64; // w24
  FlowScreen::MenuItem *v65; // x0
  FlowScreen::MenuItem *v66; // x22
  FlowScreen::MenuItem *v67; // x21
  FlowScreen::MenuItem *v68; // x8

  if ( thiz->isPauseScreen )
  {
    Texture = TextureDatabaseRuntime::GetTexture("menu_mainresume");
    ++Texture->refCount;
    p_numEntries = &thiz->items.numEntries;
    numEntries = thiz->items.numEntries;
    p_items = &thiz->items;
    if ( thiz->items.numAlloced >= (unsigned int)(numEntries + 1) )
    {
      dataPtr = thiz->items.dataPtr;
    }
    else
    {
      v6 = 4 * ((int)numEntries + 1) / 3u + 3;
      v7 = (FlowScreen::MenuItem *)malloc(24LL * v6);
      v8 = thiz->items.dataPtr;
      dataPtr = v7;
      if ( v8 )
      {
        memcpy(v7, thiz->items.dataPtr, 24 * numEntries);
        free(v8);
        numEntries = *p_numEntries;
      }
      thiz->items.numAlloced = v6;
      thiz->items.dataPtr = dataPtr;
    }
    v16 = &dataPtr[(unsigned int)numEntries];
    v16->itemTex = Texture;
    v16->text = reinterpret_cast<const uint8_t *>("FEP_RES");
    v16->selected = MainMenuScreen::OnResume;
    ++*p_numEntries;
    v17 = TextureDatabaseRuntime::GetTexture("menu_mainsettings");
    ++v17->refCount;
    v18 = *p_numEntries;
    if ( p_items->numAlloced >= (unsigned int)(v18 + 1) )
    {
      v22 = thiz->items.dataPtr;
    }
    else
    {
      v19 = 4 * ((int)v18 + 1) / 3u + 3;
      v20 = (FlowScreen::MenuItem *)malloc(24LL * v19);
      v21 = thiz->items.dataPtr;
      v22 = v20;
      if ( v21 )
      {
        memcpy(v20, thiz->items.dataPtr, 24 * v18);
        free(v21);
        v18 = *p_numEntries;
      }
      thiz->items.numAlloced = v19;
      thiz->items.dataPtr = v22;
    }
    v23 = &v22[(unsigned int)v18];
    v23->itemTex = v17;
    v23->text = reinterpret_cast<const uint8_t *>("FEP_OPT");
    v23->selected = MainMenuScreen::OnSettings;
    ++*p_numEntries;
    v24 = TextureDatabaseRuntime::GetTexture("menu_mainstats");
    ++v24->refCount;
    v25 = *p_numEntries;
    if ( p_items->numAlloced >= (unsigned int)(v25 + 1) )
    {
      v29 = thiz->items.dataPtr;
    }
    else
    {
      v26 = 4 * ((int)v25 + 1) / 3u + 3;
      v27 = (FlowScreen::MenuItem *)malloc(24LL * v26);
      v28 = thiz->items.dataPtr;
      v29 = v27;
      if ( v28 )
      {
        memcpy(v27, thiz->items.dataPtr, 24 * v25);
        free(v28);
        v25 = *p_numEntries;
      }
      thiz->items.numAlloced = v26;
      thiz->items.dataPtr = v29;
    }
    v30 = &v29[(unsigned int)v25];
    v30->itemTex = v24;
    v30->text = reinterpret_cast<const uint8_t *>("FEH_STA");
    v30->selected = FlowScreen::OnStats;
    ++*p_numEntries;
    if ( CMessages::PreviousBriefs[0].pMessage )
    {
      v31 = TextureDatabaseRuntime::GetTexture("menu_mainbriefs");
      ++v31->refCount;
      v32 = *p_numEntries;
      if ( p_items->numAlloced >= (unsigned int)(v32 + 1) )
      {
        v36 = thiz->items.dataPtr;
      }
      else
      {
        v33 = 4 * ((int)v32 + 1) / 3u + 3;
        v34 = (FlowScreen::MenuItem *)malloc(24LL * v33);
        v35 = thiz->items.dataPtr;
        v36 = v34;
        if ( v35 )
        {
          memcpy(v34, thiz->items.dataPtr, 24 * v32);
          free(v35);
          v32 = *p_numEntries;
        }
        thiz->items.numAlloced = v33;
        thiz->items.dataPtr = v36;
      }
      v37 = &v36[(unsigned int)v32];
      v37->itemTex = v31;
      v37->text = reinterpret_cast<const uint8_t *>("FEH_BRI");
      v37->selected = FlowScreen::OnBriefs;
      ++*p_numEntries;
    }
    v38 = TextureDatabaseRuntime::GetTexture("menu_mainplay");
    ++v38->refCount;
    v39 = *p_numEntries;
    if ( p_items->numAlloced >= (unsigned int)(v39 + 1) )
    {
      v43 = thiz->items.dataPtr;
    }
    else
    {
      v40 = 4 * ((int)v39 + 1) / 3u + 3;
      v41 = (FlowScreen::MenuItem *)malloc(24LL * v40);
      v42 = thiz->items.dataPtr;
      v43 = v41;
      if ( v42 )
      {
        memcpy(v41, thiz->items.dataPtr, 24 * v39);
        free(v42);
        v39 = *p_numEntries;
      }
      thiz->items.numAlloced = v40;
      thiz->items.dataPtr = v43;
    }
    v44 = &v43[(unsigned int)v39];
    v44->itemTex = v38;
    v44->text = reinterpret_cast<const uint8_t *>("FEP_STG");
    p_selected = &v44->selected;
    v46 = MainMenuScreen::OnStartGame;
  }
  else
  {
    if ( MainMenuScreen::HasCPSave(thiz) )
    {
      v10 = TextureDatabaseRuntime::GetTexture("menu_mainresume");
      ++v10->refCount;
      p_numEntries = &thiz->items.numEntries;
      v11 = thiz->items.numEntries;
      p_items = &thiz->items;
      if ( thiz->items.numAlloced >= (unsigned int)(v11 + 1) )
      {
        v15 = thiz->items.dataPtr;
      }
      else
      {
        v12 = 4 * ((int)v11 + 1) / 3u + 3;
        v13 = (FlowScreen::MenuItem *)malloc(24LL * v12);
        v14 = thiz->items.dataPtr;
        v15 = v13;
        if ( v14 )
        {
          memcpy(v13, thiz->items.dataPtr, 24 * v11);
          free(v14);
          v11 = *p_numEntries;
        }
        thiz->items.numAlloced = v12;
        thiz->items.dataPtr = v15;
      }
      v47 = &v15[(unsigned int)v11];
      v47->itemTex = v10;
      v47->text = reinterpret_cast<const uint8_t *>("FEP_RES");
      v47->selected = MainMenuScreen::OnResume;
      ++*p_numEntries;
    }
    else
    {
      p_numEntries = &thiz->items.numEntries;
      p_items = &thiz->items;
    }
    v48 = TextureDatabaseRuntime::GetTexture("menu_mainplay");
    ++v48->refCount;
    v49 = *p_numEntries;
    if ( p_items->numAlloced >= (unsigned int)(v49 + 1) )
    {
      v53 = thiz->items.dataPtr;
    }
    else
    {
      v50 = 4 * ((int)v49 + 1) / 3u + 3;
      v51 = (FlowScreen::MenuItem *)malloc(24LL * v50);
      v52 = thiz->items.dataPtr;
      v53 = v51;
      if ( v52 )
      {
        memcpy(v51, thiz->items.dataPtr, 24 * v49);
        free(v52);
        v49 = *p_numEntries;
      }
      thiz->items.numAlloced = v50;
      thiz->items.dataPtr = v53;
    }
    v54 = &v53[(unsigned int)v49];
    v54->itemTex = v48;
    v54->text = reinterpret_cast<const uint8_t *>("FEP_STG");
    v54->selected = MainMenuScreen::OnStartGame;
    ++*p_numEntries;
    v55 = TextureDatabaseRuntime::GetTexture("menu_mainsettings");
    ++v55->refCount;
    v56 = *p_numEntries;
    if ( p_items->numAlloced >= (unsigned int)(v56 + 1) )
    {
      v60 = thiz->items.dataPtr;
    }
    else
    {
      v57 = 4 * ((int)v56 + 1) / 3u + 3;
      v58 = (FlowScreen::MenuItem *)malloc(24LL * v57);
      v59 = thiz->items.dataPtr;
      v60 = v58;
      if ( v59 )
      {
        memcpy(v58, thiz->items.dataPtr, 24 * v56);
        free(v59);
        v56 = *p_numEntries;
      }
      thiz->items.numAlloced = v57;
      thiz->items.dataPtr = v60;
    }
    v61 = &v60[(unsigned int)v56];
    v61->itemTex = v55;
    v61->text = reinterpret_cast<const uint8_t *>("FEP_OPT");
    p_selected = &v61->selected;
    v46 = MainMenuScreen::OnSettings;
  }
  *p_selected = v46;
  ++*p_numEntries;
  v62 = TextureDatabaseRuntime::GetTexture("menu_mainquit");
  ++v62->refCount;
  v63 = *p_numEntries;
  if ( p_items->numAlloced >= (unsigned int)(v63 + 1) )
  {
    v67 = thiz->items.dataPtr;
  }
  else
  {
    v64 = 4 * ((int)v63 + 1) / 3u + 3;
    v65 = (FlowScreen::MenuItem *)malloc(24LL * v64);
    v66 = thiz->items.dataPtr;
    v67 = v65;
    if ( v66 )
    {
      memcpy(v65, thiz->items.dataPtr, 24 * v63);
      free(v66);
      v63 = *p_numEntries;
    }
    thiz->items.numAlloced = v64;
    thiz->items.dataPtr = v67;
  }
  v68 = &v67[(unsigned int)v63];
  v68->itemTex = v62;
  v68->text = reinterpret_cast<const uint8_t *>("FEP_QUI");
  v68->selected = MainMenuScreen::OnExit;
  ++*p_numEntries;
}

void MainMenuScreen::InjectHooks() {
    //CHook::Redirect("_ZN14MainMenuScreen11AddAllItemsEv", &MainMenuScreen::AddAllItems);
}