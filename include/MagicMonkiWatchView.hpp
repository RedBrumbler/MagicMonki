#pragma once

#include "custom-types/shared/macros.hpp"
#include "monkecomputer/shared/ViewLib/View.hpp"
#include "monkecomputer/shared/InputHandlers/UIToggleInputHandler.hpp"
#include "monkecomputer/shared/InputHandlers/UISelectionHandler.hpp"

DECLARE_CLASS_CUSTOM(MagicMonki, MagicMonkiWatchView, GorillaUI::Components::View, 
    DECLARE_INSTANCE_METHOD(void, Awake);
    DECLARE_INSTANCE_METHOD(void, DidActivate, bool firstActivation);
    DECLARE_INSTANCE_METHOD(void, Redraw);
    DECLARE_INSTANCE_METHOD(void, DrawHeader);
    DECLARE_INSTANCE_METHOD(void, DrawBody);
    DECLARE_INSTANCE_METHOD(void, OnKeyPressed, int key);
    DECLARE_INSTANCE_METHOD(void, OnEnter, int key);
    public:
        GorillaUI::UISelectionHandler* settingSelector = nullptr;
        GorillaUI::UIToggleInputHandler* toggleHandler = nullptr;
        GorillaUI::UISelectionHandler* ctrlSelector = nullptr;
)