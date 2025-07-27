#include "ui.h"

void draw_header(WINDOW *header, int width, double cpu_usage, double mem_usage, double mem_free) {
    werase(header);
    char cpu_bar[BAR_WIDTH + 1];
    char mem_usage_bar[BAR_WIDTH + 1];
    char mem_free_bar[BAR_WIDTH + 1];

    make_bar(cpu_usage, BAR_WIDTH, cpu_bar);
    make_bar(mem_usage, BAR_WIDTH, mem_usage_bar);
    make_bar(mem_free, BAR_WIDTH, mem_free_bar);

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

void draw_body(WINDOW *body, int selected_index, int scroll_offset, unsigned long long *pid) {
    werase(body);
    box(body, 0, 0);

    struct process_list *plist = list_all_process(0);
    if (!plist || plist->count == 0) {
        mvwprintw(body, 1, 2, "No processes available.");
        wrefresh(body);
        return;
    }

    int body_height = getmaxy(body);
    int max_display = body_height - 3;
    int total = plist->count;

    if (selected_index >= total)
        selected_index = total - 1;
    if (selected_index < 0)
        selected_index = 0;

    // Adjust scroll offset
    if (selected_index < scroll_offset)
        scroll_offset = selected_index;
    else if (selected_index >= scroll_offset + max_display)
        scroll_offset = selected_index - max_display + 1;

    // Draw header
    wattron(body, A_BOLD | COLOR_PAIR(2));
    mvwprintw(body, 1, 1,
        "%-8s %-8s %-5s %-5s %-5s %-5s %-10s %-10s %-10s %-20s",
        "PID", "PPID", "THR", "STAT", "PRI", "NI", "VmSize", "VmRSS", "USER", "CMD");
    wattroff(body, A_BOLD | COLOR_PAIR(2));

    for (int i = 0; i < max_display && (i + scroll_offset) < total; i++) {
        int index = i + scroll_offset;
        struct process_info *p = &plist->data[index];

        if (index == selected_index){
            *pid = p->pid;
            wattron(body, A_REVERSE);
        }

        mvwprintw(body, i + 2, 1,
            "%-8llu %-8llu %-5d %-5s %-5d %-5d %-10lu %-10lu %-10s %-20s",
            p->pid, p->ppid, p->threads, p->state,
            p->priority, p->nice, p->vm_size, p->vm_rss,
            p->username, p->comm);

        if (index == selected_index)
            wattroff(body, A_REVERSE);
    }

    free(plist->data);
    free(plist);
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
void draw_process_details(WINDOW *body, unsigned long long pid) {
    werase(body);
    box(body, 0, 0);

    struct process_list *plist = list_all_process(pid);
    if (!plist || plist->count == 0) {
        mvwprintw(body, 1, 2, "No process data available.");
        wrefresh(body);
        getch();
        return;
    }

    struct process_info *p = NULL;
    for (size_t i = 0; i < plist->count; i++) {
        if (plist->data[i].pid == pid) {
            p = &plist->data[i];
            break;
        }
    }

    if (!p) {
        mvwprintw(body, 1, 2, "PID %llu not found.", pid);
        wrefresh(body);
        getch();
        free(plist->data);
        free(plist);
        return;
    }

    wattron(body, A_BOLD | COLOR_PAIR(2));
    mvwprintw(body, 1, 2, "Detailed View for PID: %llu", p->pid);
    wattroff(body, A_BOLD | COLOR_PAIR(2));

    mvwprintw(body, 3, 2, "PID       : %llu", p->pid);
    mvwprintw(body, 4, 2, "PPID      : %llu", p->ppid);
    mvwprintw(body, 5, 2, "Threads   : %d",   p->threads);
    mvwprintw(body, 6, 2, "State     : %s",   p->state);
    mvwprintw(body, 7, 2, "Priority  : %d",   p->priority);
    mvwprintw(body, 8, 2, "Nice      : %d",   p->nice);
    mvwprintw(body, 9, 2, "VmSize    : %lu KB", p->vm_size);
    mvwprintw(body, 10, 2, "VmRSS     : %lu KB", p->vm_rss);
    mvwprintw(body, 11, 2, "Username  : %s",   p->username);
    mvwprintw(body, 12, 2, "Command   : %s",   p->comm);

    mvwprintw(body, getmaxy(body) - 2, 2, "[Press any key to return]");
    wrefresh(body);

    free(plist->data);
    free(plist);
}