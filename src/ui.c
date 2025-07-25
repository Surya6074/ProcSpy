#include "ui.h"

void draw_header(WINDOW *header, int width, double cpu_usage, double mem_usage, double mem_free) {
    char cpu_bar[BAR_WIDTH + 1];
    char mem_usage_bar[BAR_WIDTH + 1];
    char mem_free_bar[BAR_WIDTH + 1];

    make_bar(cpu_usage, BAR_WIDTH, cpu_bar);
    make_bar(mem_usage, BAR_WIDTH, mem_usage_bar);
    make_bar(mem_free, BAR_WIDTH, mem_free_bar);

    werase(header);
    box(header, 0, 0);

    wattron(header, COLOR_PAIR(1) | A_BOLD);
    mvwprintw(header, 1, 2, "ProcSpy");
    wattroff(header, COLOR_PAIR(1) | A_BOLD);
    mvwprintw(header, 2, 2, "Detailed Linux Process Viewer");

    int stats_x = width - 42;
    if (stats_x < 20) stats_x = 20;

    wattron(header, COLOR_PAIR(2));
    mvwprintw(header, 1, stats_x, "CPU Usage: [%s] %.2f%%", cpu_bar, cpu_usage);
    mvwprintw(header, 2, stats_x, "Mem Usage: [%s] %.2f%%", mem_usage_bar, mem_usage);
    mvwprintw(header, 3, stats_x, "Mem Free : [%s] %.2f%%", mem_free_bar, mem_free);
    wattroff(header, COLOR_PAIR(2));

    wrefresh(header);
}

void draw_body(WINDOW *body) {
    werase(body);
    box(body, 0, 0);
    mvwprintw(body, 1, 2, "Process list will appear here...");
    wrefresh(body);
}

void draw_footer(WINDOW *footer, int width, const char *footer_text) {
    werase(footer);
    box(footer, 0, 0);

    wattron(footer, COLOR_PAIR(3) | A_BOLD);
    int footer_x = (width - strlen(footer_text)) / 2;
    mvwprintw(footer, 1, footer_x, "%s", footer_text);
    wattroff(footer, COLOR_PAIR(3) | A_BOLD);

    wrefresh(footer);
}