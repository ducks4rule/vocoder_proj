#include "ui/tui.h"
#include <ncurses.h>
#include <algorithm>

namespace {
    const int METER_WIDTH = 20;
    const float MIN_DB = -60.0f;
    const float MAX_DB = 0.0f;
    const float YELLOW_THRESH = -12.0f;
    const float RED_THRESH = -3.0f;
    const int MASTER_HEIGHT = 10;

    int db_to_bar(float db) {
        int bar = static_cast<int>((db - MIN_DB) / (MAX_DB - MIN_DB) * METER_WIDTH);
        return std::max(0, std::min(bar, METER_WIDTH));
    }

    int get_color_for_db(float db) {
        if (db > RED_THRESH) return 3;
        if (db > YELLOW_THRESH) return 2;
        return 1;
    }

    void draw_bar(int row, int col, const char* label, int filled, float db) {
        mvprintw(row, col, "%s", label);
        for (int i = 0; i < METER_WIDTH; i++) {
            if (i < filled) {
                attron(COLOR_PAIR(get_color_for_db(db)));
                addstr("#");
                attroff(COLOR_PAIR(get_color_for_db(db)));
            } else {
                addstr("-");
            }
        }
        attron(COLOR_PAIR(4));
        printw(" %5.1f dB", db);
        attroff(COLOR_PAIR(4));
    }

    void draw_vertical_meter(int row, int col, int percent, float output_db) {
        int filled = (percent + 5) / 10;
        for (int i = 0; i < MASTER_HEIGHT; i++) {
            int level = MASTER_HEIGHT - 1 - i;
            int r = row + i;
            int threshold = (level + 1) * 10;
            mvprintw(r, col, "%3d%% ", threshold);
            if (filled > level) {
                attron(COLOR_PAIR(get_color_for_db(output_db)));
                addstr("#");
                attroff(COLOR_PAIR(get_color_for_db(output_db)));
            } else {
                addstr("-");
            }
        }
    }
}

TUI::TUI() : initialized_(false), width_(80), height_(24) {
}

TUI::~TUI() {
    shutdown();
}

void TUI::init() {
    initscr();
    start_color();
    use_default_colors();

    init_pair(0, COLOR_WHITE, COLOR_BLACK);
    init_pair(1, COLOR_GREEN, COLOR_BLACK);
    init_pair(2, COLOR_YELLOW, COLOR_BLACK);
    init_pair(3, COLOR_RED, COLOR_BLACK);
    init_pair(4, COLOR_WHITE, COLOR_BLACK);
    init_pair(5, COLOR_CYAN, COLOR_BLACK);

    bkgd(COLOR_PAIR(0));
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
    if (!initialized_) return;

    werase(stdscr);

    int in_bar = db_to_bar(stats.input_level);
    int out_bar = db_to_bar(stats.output_level);

    draw_bar(1, 2, "IN:", in_bar, stats.input_level);
    draw_bar(3, 2, "OUT:", out_bar, stats.output_level);

    int vol_percent = static_cast<int>(stats.volume * 100);
    draw_vertical_meter(6, 2, vol_percent, stats.output_level);
    mvprintw(5, 2, "MASTER");

    if (stats.muted) {
        attron(COLOR_PAIR(3));
        mvprintw(8, 2, "MUTED");
        attroff(COLOR_PAIR(3));
    }

    attron(COLOR_PAIR(5));
    mvprintw(23, 2, "[q:quit] [m:mute] [ [/]:vol ]  [+/-:adj] [=/_:fine steps] [r:reset]");
    attroff(COLOR_PAIR(5));

    refresh();
}

int TUI::get_key_input() {
    if (!initialized_) return 0;
    
    int ch = getch();
    if (ch == ERR) {
        return 0;
    }
    return ch;
}
