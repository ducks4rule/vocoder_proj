#include "ui/tui.h"
#include <ncurses.h>

TUI::TUI() : initialized_(false), width_(80), height_(24) {
}

TUI::~TUI() {
    shutdown();
}

void TUI::init() {
    initscr();
    start_color();
    curs_set(0);
    nodelay(stdscr, TRUE);
    keypad(stdscr, TRUE);
    initialized_ = true;
}

void TUI::shutdown() {
    if (initialized_) {
        endwin();
    }
    initialized_ = false;
}

void TUI::render(const AudioStats& stats) {
    (void)stats;
}

int TUI::get_key_input() {
    if (!initialized_) return 0;
    
    int ch = getch();
    if (ch == ERR) {
        return 0;
    }
    return ch;
}
