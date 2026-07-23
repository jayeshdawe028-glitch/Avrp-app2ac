//
// Created by viski on 03.06.2025.
//

#include "FlowScreen.h"
#include "patch.h"

void FlowScreen::OnStats() {
    CHook::CallFunction<void>("_ZN10FlowScreen7OnStatsEv");
}

void FlowScreen::OnBriefs() {
    CHook::CallFunction<void>("_ZN10FlowScreen7OnStatsEv");
}
