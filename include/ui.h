#ifndef UI_H
#define UI_H

#include "main.h"

/* Width of the usage bar in characters */
#define BAR_WIDTH 20

/**
 * @brief draw_header - Draw the header section of the UI window.
 * 
 * @param header: Pointer to the ncurses WINDOW for the header.
 * @param width: Width of the header window.
 * @param cpu_usage: Current CPU usage percentage.
 * @param mem_usage: Current memory usage percentage.
 * @param mem_free: Amount of free memory.
 *
 * This function renders the header with system statistics.
 */
void draw_header(WINDOW *header, int width, double cpu_usage, double mem_usage, double mem_free);

/**
 * @brief draw_body - Draw the body section of the UI window.
 * 
 * @param body: Pointer to the ncurses WINDOW for the Body.
 *
 * This function renders the Body For listing all the Process.
 */
void draw_body(WINDOW *body);

/**
 * @brief draw_footer - Draw the Footer section of the UI window.
 *
 * @param footer  Pointer to the ncurses WINDOW for the Footer.
 * @param width Width For the Footer Size
 * @param footer_text Footer Text Content to Show.
 * 
 * This function renders the Footer For showing the Shorcuts for the Procspy.
 */
void draw_footer(WINDOW *footer, int width, const char *footer_text);

#endif
