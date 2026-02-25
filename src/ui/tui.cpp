#include "ui/tui.h"

TUI::TUI() : initialized_(false), width_(80), height_(24) {
}

TUI::~TUI() {
    shutdown();
}

void TUI::init() {
    initialized_ = true;
}

void TUI::shutdown() {
    initialized_ = false;
}

void TUI::render(const AudioStats& stats) {
    (void)stats;
}

int TUI::get_key_input() {
    return 0;
}
