#include "main.h"
#include "ui.h"
#include <signal.h>
#include <time.h>

volatile sig_atomic_t resized = 0;

void handle_winch() {
    resized = 1;
}

int run_app(void);

int main() {

    run_app();

    struct process_list *plist = list_all_process();
    if (plist) {
        for (size_t i = 0; i < plist->count; i++) {
            printf("PID: %llu CMD: %s\n", plist->data[i].pid, plist->data[i].comm);
        }
        free(plist->data);
        free(plist);
    }
    
    return 0;
}

int run_app(){
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

    keypad(stdscr, TRUE);
    halfdelay(1);

    struct cpu_stats before = get_cpu_stats();
    struct cpu_stats after;
    struct mem_stats mem = get_memory_usage_stats();
    double cpu_usage = 0.0;

    draw_header(header, width, 0.0, mem.usage, mem.free);
    wrefresh(header);

    time_t last_update = time(NULL);

    int running = 1;

    while (running) {
        int ch = getch();
        switch (ch) {
            case 'q':
            case 'Q':
                running = 0;
                break;
            case KEY_UP:
                // handle scroll up
                break;
            case KEY_DOWN:
                // handle scroll down
                break;
            case 10: // Enter key
                // handle enter
                break;
            default:
                break;
        }

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

        if (difftime(time(NULL), last_update) >= 1.0) {
            after = get_cpu_stats();
            cpu_usage = usage_percent(&before, &after);
            before = after;

            mem = get_memory_usage_stats();
            draw_header(header, width, cpu_usage, mem.usage, mem.free);
            wrefresh(header);

            last_update = time(NULL);
        }

        draw_body(body);
        wrefresh(body);

        draw_footer(footer, width, footer_text);
        wrefresh(footer);

        napms(10);
    }

    delwin(header);
    delwin(body);
    delwin(footer);
    endwin();
    return 0;
}