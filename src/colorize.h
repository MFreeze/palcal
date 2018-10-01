#ifndef PAL_COLORIZE_H
#define PAL_COLORIZE_H

/* pal
 *
 * Copyright (C) 2004, Scott Kuhl
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <glib.h>

/* attributes */
#define RESET	   0
#define BRIGHT 	   1
#define DIM	   2
#define UNDERLINE  3
#define BLINK      4
#define REVERSE    7
#define HIDDEN     8

/* colors */
#define BLACK      0
#define RED        1
#define GREEN      2
#define YELLOW     3
#define BLUE       4
#define MAGENTA    5
#define CYAN       6
#define	WHITE      7


void colorize_xterm_title(gchar *title);
void set_colorize(const int in);
void colorize_fg(const int attribute, const int foreground);
void colorize_reset(void);
void colorize_bright(void);
void colorize_error(void);
gchar* string_color_of(const int color);
int int_color_of(gchar* string);

#ifdef CURSES
extern WINDOW *pal_curwin;
#endif

#endif
