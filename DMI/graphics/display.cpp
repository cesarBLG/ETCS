/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "display.h"
#include "button.h"
#include "text_button.h"
#include "../window/menu.h"
#include "../monitor.h"
#include "../control/control.h"
#include "../state/acks.h"
#include "platform_runtime.h"
std::list<window*> active_windows;
extern Mode mode;
extern uint32_t evc_peer;
bool serieSelected;
bool useImperialSystem;
bool prevUseImperialSystem;

std::shared_ptr<UiPlatform::Image> invalidEvcIndicator;
std::unique_ptr<UiPlatform::Font> indicatorFont;
#if SIMRAIL
std::shared_ptr<UiPlatform::Image> invalidSerieIndicator;
#endif

void displayETCS()
{
    updateAcks();

    if (!indicatorFont) {
        indicatorFont = platform->load_font(10, false, "");
        platform->on_quit().then([]() {
            indicatorFont = nullptr;
        }).detach();
    }

    if (!evc_peer)
    {
        if (!invalidEvcIndicator)
            invalidEvcIndicator = platform->make_text_image(get_text("EVC not connected!"), *indicatorFont, White);
        else
            platform->draw_image(*invalidEvcIndicator, 0, 0);
        return;
    }

#if SIMRAIL

    if (!serieSelected)
    {
        if (invalidSerieIndicator == NULL)
            invalidSerieIndicator = platform->make_text_image(get_text("Train serie not selected!"), *indicatorFont, White);
        else
            platform->draw_image(*invalidSerieIndicator, 0, 20);
        return;
    }

#endif

    std::vector<std::vector<int>> alreadyDrawn;
    for(auto it=active_windows.rbegin(); it!=active_windows.rend(); ++it)
    {
        window *w = *it;
        w->display(alreadyDrawn);
        alreadyDrawn.insert(alreadyDrawn.end(), w->bounds.begin(), w->bounds.end());
    }
    if (prevUseImperialSystem != useImperialSystem) {
        prevUseImperialSystem = useImperialSystem;
    }
}