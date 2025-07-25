#include "main.h"
#include "ui.h"
#include <signal.h>

volatile sig_atomic_t resized = 0;

void handle_winch() {
    resized = 1;
}

int main() {
    initscr();
    noecho();
    curs_set(FALSE);
    cbreak();

    if (has_colors()) {
        start_color();
        init_pair(1, COLOR_CYAN, COLOR_BLACK);   // Header
        init_pair(2, COLOR_GREEN, COLOR_BLACK);  // Stats
        init_pair(3, COLOR_YELLOW, COLOR_BLACK); // Footer
    }

    signal(SIGWINCH, handle_winch);

    int height, width;
    getmaxyx(stdscr, height, width);

    int header_height = 5;
    int footer_height = 3;
    int body_height = height - header_height - footer_height;

    if (body_height < 1) {
        endwin();
        fprintf(stderr, "Terminal too small!\n");
        return 1;
    }

    refresh();

    WINDOW *header = newwin(header_height, width, 0, 0);
    WINDOW *body   = newwin(body_height, width, header_height, 0);
    WINDOW *footer = newwin(footer_height, width, header_height + body_height, 0);

    const char *footer_text = "[ Q ] Quit   |   [ UP / DOWN ] Scroll   |   [ Enter ] View Details";

    nodelay(stdscr, TRUE);

    struct cpu_stats before = get_cpu_stats();
    struct cpu_stats after;

    while (1) {
        int ch = getch();
        if (ch == 'q') break;

        if (resized) {
            resized = 0;
            endwin();
            refresh();
            clear();

            getmaxyx(stdscr, height, width);
            header_height = 5;
            footer_height = 3;
            body_height = height - header_height - footer_height;

            if (body_height < 1) {
                endwin();
                fprintf(stderr, "Terminal too small!\n");
                return 1;
            }

            delwin(header);
            delwin(body);
            delwin(footer);

            header = newwin(header_height, width, 0, 0);
            body   = newwin(body_height, width, header_height, 0);
            footer = newwin(footer_height, width, header_height + body_height, 0);
        }

        after = get_cpu_stats();
        double cpu_usage = usage_percent(&before, &after);
        before = after;

        struct mem_stats mem = get_memory_usage_stats();

        draw_header(header, width, cpu_usage, mem.usage, mem.free);
        draw_body(body);
        draw_footer(footer, width, footer_text);

        napms(1000);
    }

    delwin(header);
    delwin(body);
    delwin(footer);
    endwin();
    return 0;
}
