#include "ui/tui.h"
#include "config.h"
#include <ncurses.h>
#include <algorithm>
#include <cmath>

namespace {
    const int METER_W = METER_WIDTH;
    const float METER_MIN = METER_MIN_DB;
    const float METER_MAX = METER_MAX_DB;
    const float SPEC_MIN = SPECTRUM_MIN_DB;
    const float SPEC_MAX = SPECTRUM_MAX_DB;
    const float YELLOW = METER_YELLOW_DB;
    const float RED = METER_RED_DB;
    const int MASTER_H = MASTER_HEIGHT;
    const int SPECTRUM_B = SPECTRUM_BARS;
    const float MIN_FREQ = SPECTRUM_MIN_FREQ;
    const float MAX_FREQ = SPECTRUM_MAX_FREQ;

    int db_to_bar(float db) {
        int bar = static_cast<int>((db - METER_MIN) / (METER_MAX - METER_MIN) * METER_W);
        return std::max(0, std::min(bar, METER_W));
    }

    int get_color_for_db(float db) {
        if (db > RED) return 3;
        if (db > YELLOW) return 2;
        return 1;
    }

    void draw_bar(int row, int col, const char* label, int filled, float db) {
        mvprintw(row, col, "%s", label);
        for (int i = 0; i < METER_W; i++) {
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

    void draw_vertical_meter(int row, int col, int percent, float output_db, bool muted) {
        int filled = (percent + 5) / 10;
        int color = muted ? 3 : get_color_for_db(output_db);
        for (int i = 0; i < MASTER_H; i++) {
            int level = MASTER_H - 1 - i;
            int r = row + i;
            int threshold = (level + 1) * 10;
            mvprintw(r, col, "%3d%% ", threshold);
            if (filled > level) {
                attron(COLOR_PAIR(color));
                addstr("#");
                attroff(COLOR_PAIR(color));
            } else {
                addstr("-");
            }
        }
    }

    void draw_spectrum(int row, int col, const float* spectrum, size_t num_bins) {
        mvprintw(row, col, "SPECTRUM:");
        
        for (int bar = 0; bar < SPECTRUM_B; bar++) {
            float freq = MIN_FREQ * std::pow(MAX_FREQ / MIN_FREQ, 
                static_cast<float>(bar) / SPECTRUM_B);
            
            int bin = static_cast<int>(freq / SAMPLE_RATE * FFT_SIZE);
            
            float db = (bin >= 0 && static_cast<size_t>(bin) < num_bins) ? spectrum[bin] : SPEC_MIN;
            
            int height = static_cast<int>((db - SPEC_MIN) / (SPEC_MAX - SPEC_MIN) * MASTER_H);
            height = std::max(0, std::min(height, MASTER_H));
            
            int draw_col = col + 10 + bar * 2;
            for (int h = 0; h < height; h++) {
                int draw_row = row + MASTER_H - h;
                attron(COLOR_PAIR(get_color_for_db(db)));
                mvaddch(draw_row, draw_col, '#');
                attroff(COLOR_PAIR(get_color_for_db(db)));
            }
        }
        
        mvprintw(row + MASTER_H + 1, col + 10, "20Hz");
        mvprintw(row + MASTER_H + 1, col + 30, "1kHz");
        mvprintw(row + MASTER_H + 1, col + 50, "10kHz");
        mvprintw(row + MASTER_H + 1, col + 68, "20kHz");
    }
}

TUI::TUI() : initialized_(false), width_(80), height_(24), smoothed_input_(-60.0f), smoothed_output_(-60.0f) {
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

void TUI::draw_spectrum_boxed(const char* label, int row, int col, const float* spectrum, size_t num_bins) {
    if (!spectrum || num_bins == 0) return;
    
    draw_spectrum(row + 1, col, spectrum, num_bins);
    
    attron(COLOR_PAIR(5));
    mvprintw(row, col, "[%s]", label);
    attroff(COLOR_PAIR(5));
    
    int left = col + 8;
    int right = col + 75;
    int top = row;
    int bottom = row + 13;
    
    mvaddch(top, left, ACS_ULCORNER);
    mvaddch(top, right, ACS_URCORNER);
    mvaddch(bottom, left, ACS_LLCORNER);
    mvaddch(bottom, right, ACS_LRCORNER);
    mvhline(top, left + 1, ACS_HLINE, right - left - 1);
    mvhline(bottom, left + 1, ACS_HLINE, right - left - 1);
    mvvline(top + 1, left, ACS_VLINE, bottom - top - 1);
    mvvline(top + 1, right, ACS_VLINE, bottom - top - 1);
}

void TUI::render(const AudioStats& stats) {
    if (!initialized_) return;

    smoothed_input_ = smoothed_input_ * (1.0f - SMOOTHING_FACTOR) + stats.input_level * SMOOTHING_FACTOR;
    smoothed_output_ = smoothed_output_ * (1.0f - SMOOTHING_FACTOR) + stats.output_level * SMOOTHING_FACTOR;

    werase(stdscr);

    int in_bar = db_to_bar(smoothed_input_);
    int out_bar = db_to_bar(smoothed_output_);

    draw_bar(1, 2, "IN:", in_bar, smoothed_input_);
    draw_bar(3, 2, "OUT:", out_bar, smoothed_output_);

    int vol_percent = static_cast<int>(stats.volume * 100);
    draw_vertical_meter(5, 2, vol_percent, stats.output_level, stats.muted);

    attron(COLOR_PAIR(stats.muted ? 3 : 5));
    mvprintw(16, 2, stats.muted ? "MUTE" : "MASTER");
    attroff(COLOR_PAIR(stats.muted ? 3 : 5));

    attron(COLOR_PAIR(5));
    mvprintw(18, 2, "Pitch: %+.1f st (%.2fx)", 
             static_cast<float>(stats.pitch_semitones), stats.pitch_ratio);
    attroff(COLOR_PAIR(5));

    attron(COLOR_PAIR(5));
    if (stats.detected_freq > 0) {
        float output_freq = stats.detected_freq * stats.pitch_ratio;
        mvprintw(19, 2, "In: %.1f Hz -> Out: %.1f Hz", stats.detected_freq, output_freq);
    } else {
        mvprintw(19, 2, "Input: -- Hz");
    }
    if (stats.active_note >= 0) {
        mvprintw(20, 2, "Target: %d semitones", stats.active_note);
    }
    attroff(COLOR_PAIR(5));

    float loop_len = static_cast<float>(stats.loop_recorded_samples) / SAMPLE_RATE;
    float loop_max = static_cast<float>(stats.loop_max_samples) / SAMPLE_RATE;
    attron(COLOR_PAIR(5));
    mvprintw(21, 2, "Loop: [%s] %.1fs / %.1fs", 
             stats.loop_state.c_str(), loop_len, loop_max);
    attroff(COLOR_PAIR(5));

    if (!stats.loop_message.empty()) {
        attron(COLOR_PAIR(3));
        mvprintw(22, 2, "%s", stats.loop_message.c_str());
        attroff(COLOR_PAIR(3));
    }

    if (!stats.spectrum.empty()) {
        draw_spectrum_boxed("INPUT", 4, 30, stats.spectrum.data(), stats.spectrum.size());
    }

    if (!stats.output_spectrum.empty()) {
        draw_spectrum_boxed("OUTPUT", 18, 30, stats.output_spectrum.data(), stats.output_spectrum.size());
    }

    attron(COLOR_PAIR(5));
    mvprintw(32, 2, "[q:quit] [m:mute] [ [/]:vol ]  [+/-:adj] [=/_:fine] [r:reset]");
    mvprintw(33, 2, "[l:rec] [p:play] [o:overdub] [x:clear]");
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
