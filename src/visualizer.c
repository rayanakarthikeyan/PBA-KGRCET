/* ncurses-based visualizer (simple) */
#include "../include/hashtable.h"
#include <ncurses.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* a simple visualizer wrapper: shows a few bars and counters */
void vis_init() {
    initscr();
    cbreak();
    noecho();
    nodelay(stdscr, TRUE);
    curs_set(FALSE);
    if (has_colors()) {
        start_color();
        init_pair(1, COLOR_GREEN, COLOR_BLACK);
        init_pair(2, COLOR_YELLOW, COLOR_BLACK);
        init_pair(3, COLOR_RED, COLOR_BLACK);
        init_pair(4, COLOR_CYAN, COLOR_BLACK);
    }
}

void vis_draw_header(const char *title) {
    attron(A_BOLD);
    mvprintw(0,0,"%s", title);
    attroff(A_BOLD);
    mvhline(1,0, '-', COLS);
}

void vis_draw_status(size_t inserted, size_t probes, double load) {
    mvprintw(2,0,"Inserted: %zu   Probes: %zu   Load factor: %.3f", inserted, probes, load);
}

void vis_draw_bars(size_t *buckets, size_t m) {
    int start_row = 4;
    int maxcols = COLS / 10;
    if (maxcols < 1) maxcols = 1;
    size_t maxv = 1;
    for (size_t i=0;i<m;i++) if (buckets[i] > maxv) maxv = buckets[i];
    for (size_t i=0;i<m;i++) {
        int col = (i % maxcols);
        int row = start_row + (int)(i / maxcols);
        if (row >= LINES-1) break;
        int base_col = col * 10;
        mvprintw(row, base_col, "[%03zu]", buckets[i]);
        int bar_len = (int)((double)buckets[i] / (double)maxv * 6.0);
        for (int k=0;k<bar_len;k++) {
            attron(COLOR_PAIR(1)); mvprintw(row, base_col + 6 + k, "#"); attroff(COLOR_PAIR(1));
        }
    }
}

void vis_refresh() { refresh(); }
void vis_finish() { nodelay(stdscr, FALSE); mvprintw(LINES-1, 0, "Press any key to exit..."); getch(); endwin(); }
