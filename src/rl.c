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

#include "main.h"

#include <readline/readline.h>
#include <ncurses.h>
#include <time.h>

#include <term.h>


#include "output.h"
#include "rl.h"
#include "event.h"
#include "search.h"

static gint readline_x, readline_y;
char* pal_rl_default_text = NULL;

char* pal_rl_no_match()
{ return NULL; }


void pal_rl_default_text_fn(void)
{
    gchar* locale_default_text = g_locale_from_utf8(pal_rl_default_text, -1,
						    NULL, NULL, NULL);
    if(locale_default_text == NULL)
	rl_insert_text(pal_rl_default_text); /* this shouldn't happen */
    else
    {
	rl_insert_text(locale_default_text);
	g_free(locale_default_text);
    }

    rl_redisplay_function();
}



/* prompt for required input, including blank lines */
/* caller is responsible for freeing the prompt and the read line */
gchar* pal_rl_get_raw_line(const char* prompt, const int row, const int col)
{
    char *line = NULL;
    char *locale_prompt = NULL;

    locale_prompt = g_locale_from_utf8(prompt, -1, NULL, NULL, NULL);
    readline_x = col + strlen( locale_prompt );
    readline_y = row;
    if(locale_prompt == NULL)
        line = readline(prompt); /* this shouldn't happen */
    else
    {
        move(row, col);
        clrtoeol();
        move(row,col);
	pal_output_fg(BRIGHT, GREEN, "%s", prompt);
	
        refresh();

        line = readline("");

        refresh(); /* need refresh to prevent screen from getting
                    * messed up when user presses enter */
        g_free(locale_prompt);
    }


    if(line == NULL) /* line is null with ^D */
        exit(0);

    g_strstrip(line);

    /* try to convert to utf8 if it isn't ascii or utf8 already. */
    if(!g_utf8_validate(line, -1, NULL))
    {
	gchar* utf8_string = g_locale_to_utf8(line, -1, NULL, NULL, NULL);
	if(utf8_string == NULL)
	{
	    pal_output_error(_("WARNING: Failed to convert your input into UTF-8.\n"));
	    return line;
	}
	else
	{
	    if(settings->verbose)
		g_printerr("%s\n", _("Converted string to UTF-8."));
	    g_free(line);
	    return utf8_string;
	}
    }

    return line;
}

/* Calls pal_rl_get_raw_line, but ignores empty input */
gchar* pal_rl_get_line(const char* prompt, const int row, const int col)
{
    gchar *line = NULL;
    do {
        if( line )
            g_free(line);
            
        line = pal_rl_get_raw_line(prompt, row, col);
    } while( *line == '\0' );
    
    return line;
}



gchar* pal_rl_get_line_default(const char* prompt, const int row, const int col, const char* default_text)
{
    gchar* desc = NULL;
    
    if( default_text == NULL )
        default_text = "";
    pal_rl_default_text = strdup(default_text);
    rl_pre_input_hook = (rl_hook_func_t*) pal_rl_default_text_fn;

    desc = pal_rl_get_line(prompt, row, col);

    g_free(pal_rl_default_text);
    rl_pre_input_hook = NULL;

    return desc;
}





/* Displays completions */
void pal_rl_completions_output(char **matches, int num_matches, int max_length )
{
    int matches_per_line;
    int matchlen = strlen( matches[0] );
    int i;

    int y,x;
    getyx( stdscr, y, x );
    
    max_length += 2;   /* Two spaces between lists */
    matches_per_line = settings->term_cols / max_length;

    move( y+1, 0 );
    clrtobot();
    for( i=0; i<num_matches-1; i++ )
    {
        move( (y+1) + (i/matches_per_line), (i%matches_per_line) * max_length );
        addstr( matches[i+1]+matchlen );
    }
}


void pal_rl_ncurses_hack(void)
{
    int half = (settings->term_cols - readline_x) / 2 - 3;
    int start, end;
    
    move(readline_y, readline_x);
    clrtoeol();
    move(readline_y, readline_x);

      
    /* Move back to the nearest "half" boundary and another block. If we go
     * past the start of the string, reset to the start */
    start = rl_point - (rl_point % half) - half;
    if( start < 0 )
        start = 0;

    /* Determine where the screen can print upto. If the string ends first,
     * don't worry about it. */
    end = start + settings->term_cols - readline_x - 2;
    if( end > strlen( rl_line_buffer ) )
        end = -1;

    /* If we're not starting at the beginning, display marker */    
    if( start > 0 )
        addch( '<' );
         
    /* Display string, including marker if went off end */
    if( end > 0 )
        printw( "%.*s>", end - start, rl_line_buffer + start);
    else
        printw( "%s", rl_line_buffer + start );

    /* Place cursor, taking into account marker */
    move(readline_y, readline_x + rl_point - start + (start > 0) );
    refresh();
}

gboolean pal_rl_get_y_n(const char* prompt)
{
    gchar *s = NULL;

    int y, x;
    getyx( stdscr, y, x );

    for(;;)
    {
 	rl_num_chars_to_read = 1;
  	s = pal_rl_get_line(prompt, y, x);
 	rl_num_chars_to_read = 0;

	if(g_ascii_strcasecmp(s, _("y")) == 0)
	{
	    g_free(s);
	    return TRUE;
	}
	else if(g_ascii_strcasecmp(s, _("n")) == 0)
	{
	    g_free(s);
	    return FALSE;
	}

	g_free(s);
    }
}



/* d gets filled in with GDate entered by the user to find the PalEvent. */
PalEvent* pal_rl_get_event(GDate** d, gboolean allow_global)
{
    gchar* s = NULL;
    PalEvent* event = NULL;
    *d = NULL;


    while(1)
    {
	pal_output_fg(BRIGHT, YELLOW, "> ");
	pal_output_wrap(_("Use \"today\" to access TODO events."),2,2);

	pal_output_fg(BRIGHT, GREEN, "> ");
	pal_output_wrap(_("Valid date formats include: yyyymmdd, Jan 1 2000, 1 Jan 2000, 4 days away"),2,2);

	s = pal_rl_get_line(_("Date for event or search string: "), settings->term_rows-2, 0);
	*d = get_query_date(s, FALSE);

	if(*d != NULL)
	{
	    gint event_num = -1;

	    g_print("\n");
	    pal_output_date(*d, TRUE, -1);
	    g_print("\n");

	    {   /* Don't allow user select a day without events on it */
		GList* events = get_events(*d);
		gint num_events = g_list_length(events);
		if(num_events==0)
		    continue;
	    }

	    while(1)
	    {
		pal_output_fg(BRIGHT, YELLOW, "> ");
		pal_output_wrap(_("Use \"0\" to use a different date or search string."),2,2);

		s = pal_rl_get_line(_("Select event number: "),settings->term_rows-2,0);
		if(strcmp(s, "0") == 0)
		    return pal_rl_get_event(d, allow_global);

		if(sscanf(s, "%i", &event_num) != 1)
		    continue;

		event = pal_output_event_num(*d, event_num);
		if(event != NULL)
		{
		    if(!event->global || allow_global)
			return event;

		    pal_output_fg(BRIGHT, RED, "> ");
		    pal_output_wrap(_("This event is in a global calendar file.  You can change this event only by editing the global calendar file manually (root access might be required)."),2,2);
		}
	    }
	}

	else /* d == NULL */
	{
	    gchar* search_string = g_strdup(s);
	    gint event_num = -1;
	    GDate* date = g_date_new();
	    g_date_set_time_t(date,  time(NULL));

	    if(pal_search_view(search_string, date, 365, TRUE) == 0)
		continue;

	    while(1)
	    {
		pal_output_fg(BRIGHT, YELLOW, "> ");
		pal_output_wrap(_("Use \"0\" to use a different date or search string."),2,2);

		s = pal_rl_get_line(_("Select event number: "),settings->term_rows-2,0);
		if(strcmp(s, "0") == 0)
		    return pal_rl_get_event(d, allow_global);

		if(sscanf(s, "%i", &event_num) != 1)
		    continue;

		event = pal_search_event_num(event_num, d, search_string, date, 365);
		if(event != NULL)
		{
		    if(!event->global || allow_global)
			return event;

		    pal_output_fg(BRIGHT, RED, "> ");
		    pal_output_wrap(_("This event is in a global calendar file.  You can change this event only by editing the global calendar file manually (root access might be required)."),2,2);

		}
	    }

	    g_free(search_string);
	}



	if(*d != NULL) g_date_free(*d);
	if(s != NULL) g_free(s);

    } /* end while(1); */

    /* impossible */
    return NULL;



}

#if 0
/* Returns the hashtable key for the day that the user inputs.  The
 * user can select a TODO event by entering TODO for the date.  */
static gchar* pal_rl_get_date(int row, int col)
{
    gchar* s = NULL;
    GDate* d = NULL;

    do
    {
        move( row, col );
	pal_output_fg(BRIGHT, GREEN, "> ");
	pal_output_wrap(_("Valid date formats include: yyyymmdd, Jan 1 2000, 1 Jan 2000, 4 days away"),2,2);

	s = pal_rl_get_line(_("Date for event: "),row+2,0);
	d = get_query_date(s, FALSE);

	if(d != NULL)
	{
	    gchar buf[1024];

	    g_print("\n");

	    pal_output_fg(BRIGHT, GREEN, "> ");
	    g_print(_("Events on the date you selected:\n"));

	    g_print("\n");
	    pal_output_date(d, TRUE, -1);
	    g_print("\n");

	    pal_output_fg(BRIGHT, GREEN, "> ");
	    g_print(_("Is this the correct date?"));
	    g_print("\n");

	    g_date_strftime(buf, 1024, _("%a %e %b %Y - Accept? [y/n]: "), d);

	    if(pal_rl_get_y_n(buf))
	    {
		s = get_key(d);
		g_date_free(d);
		return s;
	    }
	}

	else
	{
	    if(g_ascii_strcasecmp(s, "todo") == 0)
		return g_strdup("TODO");
	}

	if(d != NULL) g_date_free(d);
	if(s != NULL) g_free(s);

    } while(1);



    /* impossible */
    return NULL;
}
#endif


