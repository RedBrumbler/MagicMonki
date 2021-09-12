#include "MagicMonkiWatchView.hpp"
#include "config.hpp"
#include "monkecomputer/shared/ViewLib/MonkeWatch.hpp"
#include "beatsaber-hook/shared/utils/il2cpp-functions.hpp"

DEFINE_TYPE(MagicMonki, MagicMonkiWatchView);

using namespace GorillaUI;
using namespace UnityEngine;

extern bool inRoom;

namespace MagicMonki
{
    void MagicMonkiWatchView::Awake()
    {
        settingSelector = new UISelectionHandler(EKeyboardKey::Up, EKeyboardKey::Down, EKeyboardKey::Enter, true, false);
        ctrlSelector = new UISelectionHandler(EKeyboardKey::Left, EKeyboardKey::Right, EKeyboardKey::Enter, false, true);

        settingSelector->max = 2;
        ctrlSelector->max = 2;
    }

    void MagicMonkiWatchView::DidActivate(bool firstActivation)
    {
        std::function<void(bool)> fun = std::bind(&MagicMonkiWatchView::OnEnter, this, std::placeholders::_1);
        settingSelector->selectionCallback = fun;
        Redraw();
    }

    void MagicMonkiWatchView::OnEnter(int index)
    {
        if (settingSelector->currentSelectionIndex == 0) 
        {
            config.enabled ^= 1;
            SaveConfig();
        }
    }

    void MagicMonkiWatchView::Redraw()
    {
        text = "";

        DrawHeader();
        DrawBody();

        MonkeWatch::Redraw();
    }

    void MagicMonkiWatchView::DrawHeader()
    {
        text += "<size=80><b><color=#c53ab8> Teleportin Monke:</color></b>\n</size>";
    }

    void MagicMonkiWatchView::DrawBody()
    {
        text += settingSelector->currentSelectionIndex == 0 ? "<size=60><color=#FF5115>></color></size>" : "<size=60> </size>";
        if (inRoom) {
        text += config.enabled ? "<size=60><color=#00ff00> Enabled</color></size>" : "<size=60><color=#ff0000> Disabled</color></size>";
        } else {
        text += "<size=60><color=#555555> Disabled</color></size>";
        }
        text += "<size=60>\n</size>";
        text += settingSelector->currentSelectionIndex == 1 ? "<size=60><color=#FF5115>></color></size>" : "<size=60> </size>";
        text += "<size=60> Controller: </size>";
        switch (ctrlSelector->currentSelectionIndex)
        {
            case 0:
                text += "<size=60>Right</size>";
                break;
            case 1:
                text += "<size=60>Left</size>";
                break;
            default:
                break;
        }
        text += "<size=60>\n\n</size>";
        text += "<size=60> Made by <color=#d827d0>bay30</color></size>";

        if (config.enabled && !inRoom)
        {
            text += "<size=40><color=#b31e38>\n This mod requires you to be in a room.</color></size>";
        }
    }

    void MagicMonkiWatchView::OnKeyPressed(int value)
    {
        EKeyboardKey key = (EKeyboardKey)value;
        if (!settingSelector->HandleKey(key)) // if it was not up/down/enter
        {
            switch (settingSelector->currentSelectionIndex)
            {
                case 0:
                    break;
                case 1:
                    ctrlSelector->HandleKey(key);
                    break;
                default:
                    break;
            }
            config.ctrl = ctrlSelector->currentSelectionIndex;
            SaveConfig();
        }
        Redraw();
    }
}