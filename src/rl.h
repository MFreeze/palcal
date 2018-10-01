#ifndef PAL_RL_H
#define PAL_RL_H

/* pal
 *
 * Copyright (C) 2004--2006, Scott Kuhl
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

#include <readline/readline.h>

/* extern char* pal_rl_default_text; */

char* pal_rl_no_match(void);

/* can't return blank line */
gchar* pal_rl_get_line(const char* prompt, const int row, const int col); 
gchar* pal_rl_get_line_default(const char* prompt, const int row, const int col, const char* default_text);

/* can return blank line */
gchar* pal_rl_get_raw_line(const char* prompt, const int row, const int col); 

gboolean pal_rl_get_y_n(const char* prompt);


/* void pal_rl_default_text_fn(void); */
void pal_rl_completions_output(char **matches, int num_matches, int max_length );
PalEvent* pal_rl_get_event(GDate** d, gboolean allow_global);
void pal_rl_ncurses_hack(void);
#endif
