#pragma once

struct config_t {
    int ctrl = 0;
    bool enabled = false;
};

extern config_t config;

void SaveConfig();
bool LoadConfig();