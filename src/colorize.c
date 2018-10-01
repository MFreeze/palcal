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

#include <stdio.h>
#include <stdlib.h> /* for getenv() */
#include <curses.h>

#include "main.h"
#include "colorize.h"

static int use_colors = -1;  /* -2 = no colors, can't turn them on later if this is set;
			        -1 = don't know;
			         0 = no colors;
			         1 = colors */


/* Sets use_colors variable depending on the terminal type.  This is
   very crude.  */
static void color_term(void)
{
    char *term = getenv("TERM");

    use_colors = 0;  /* don't use colors by default */

    if(term == NULL)
      return;

    /* use colors if TERM variable is one of the following */
    if( getenv("COLORTERM") != NULL ||
	!g_ascii_strncasecmp(term, "xterm", 5) ||
	!g_ascii_strncasecmp(term, "xterm-color", 11) ||
	!g_ascii_strncasecmp(term, "linux", 5) || /* linux console */
	!g_ascii_strncasecmp(term, "ansi", 4) ||
	!g_ascii_strncasecmp(term, "Eterm", 5) ||
	!g_ascii_strncasecmp(term, "dtterm", 6) || /* Solaris */
	!g_ascii_strncasecmp(term, "rxvt", 4) || /* rxvt & aterm */
	!g_ascii_strncasecmp(term, "cygwin", 6) ||
	!g_ascii_strncasecmp(term, "screen", 6))
	use_colors = 1;

    /* make sure TERM=dumb didn't slip by with COLORTERM set */
    if(!g_ascii_strncasecmp(term, "dumb", 4))
	use_colors = 0;

}


/* allows user to manually set use_colors variable */
void set_colorize(const int in)
{
    if(use_colors != -2)
	use_colors = in;
}

static int get_curses_color(const int color)
{

    switch(color)
    {
	case BLACK:   return COLOR_BLACK;   break;
	case RED:     return COLOR_RED;     break;
	case GREEN:   return COLOR_GREEN;   break;
	case YELLOW:  return COLOR_YELLOW;  break;
	case BLUE:    return COLOR_BLUE;    break;
	case MAGENTA: return COLOR_MAGENTA; break;
	case CYAN:    return COLOR_CYAN;    break;
	case WHITE:   return COLOR_WHITE;   break;
	default: return COLOR_GREEN;
    }
}


void colorize_xterm_title(gchar *title)
{
    if(use_colors == -1)
	color_term();

    /* If the terminal doesn't support colors, it probably doesn't
     * support setting the term title */
    if(use_colors == 0 || use_colors == -2)
	return;

    printf("\033]0;%s\007", title);
}


void colorize_fg(const int attribute, const int foreground)
{
    /* determine use_colors variable if it isn't set yet. */
    if(use_colors == -1)
	color_term();

    /* don't do anything if not using colors */
    if(use_colors == 0 || use_colors == -2)
	return;

    /* Command is the control command to the terminal */
    if(settings->curses)
	wattrset(pal_curwin, A_BOLD | COLOR_PAIR(get_curses_color(foreground)));
    else
	printf("%c[%d;%dm", 0x1B, attribute, foreground+30);

}



void colorize_bright(void)
{

    /* determine use_colors variable if it isn't set yet. */
    if(use_colors == -1)
	color_term();

    /* don't do anything if not using colors */
    if(use_colors == 0 || use_colors == -2)
	return;

    /* Command is the control command to the terminal */
    if(settings->curses)
	wattrset(pal_curwin, A_BOLD);
    else
	printf("%c[%dm", 0x1B, BRIGHT);

}

void colorize_error(void)
{
    /* determine use_colors variable if it isn't set yet. */
    if(use_colors == -1)
	color_term();

    /* don't do anything if not using colors */
    if(use_colors == 0 || use_colors == -2)
	return;

    /* Command is the control command to the terminal */
    if(settings->curses)
    	wattrset(pal_curwin, A_BOLD | COLOR_PAIR(COLOR_RED));
    else
	g_printerr("%c[%d;%dm", 0x1B, BRIGHT, RED+30);
}

void colorize_reset(void)
{
    /* determine use_colors variable if it isn't set yet. */
    if(use_colors == -1)
	color_term();

    /* don't do anything if not using colors */
    if(use_colors == 0 || use_colors == -2)
	return;

    if(settings->curses)
	wattrset(pal_curwin, A_NORMAL);
    else
	g_print("%c[0m", 0x1B);
}





static const char *string_colors[] = { "black", "red", "green",
				       "yellow", "blue", "magenta",
				       "cyan", "white" };

/* free returned string when done. */
gchar* string_color_of(const int color)
{
    if(color >=0 && color < 8)
	return g_strdup(string_colors[color]);
    
    else /* when in doubt, use default color */
	return string_color_of(settings->event_color); 
}


/* returns -1 on failure to match */
int int_color_of(gchar* string)
{
    int i;
    for(i=0; i<8; i++)
    {
	if(g_ascii_strcasecmp(string, string_colors[i]) == 0)
	    return i;
    }
    return -1;
}
