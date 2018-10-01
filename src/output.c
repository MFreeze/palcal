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

#include <stdarg.h>
#include <time.h>
#include <string.h>
#include <curses.h>

/* for vsnprintf */
#include <stdarg.h>

#include "main.h"
#include "colorize.h"
#include "event.h"


/* Interface between g_print and ncurses.  This is also the print
 * handler that gets used if we aren't using curses.  The default
 * handler apparently calls fflush() all of the time.  This slows down
 * some of our code that calls fflush() often (such as the code we use
 * to print out the calendar).  */
void pal_output_handler( const gchar *instr )
{
    gsize strsize;
    char *outstr = g_locale_from_utf8(instr, -1, NULL, &strsize, NULL);
    /* if the previous fails, try the filename conversion, it gives reasonable results in most cases */
    if(!outstr)
        outstr = g_filename_from_utf8(instr, -1, NULL, &strsize, NULL);
    /* If that doesn't work, just use the input string */
    if(!outstr)
        outstr = g_strdup(instr);
    
    if(settings->curses)
	waddnstr(pal_curwin, outstr, strsize);
    else
	fputs(outstr, stdout);
    
    g_free(outstr);
}


/* set attribute w/o changing color */
void pal_output_attr(gint attr, gchar *formatString, ...)
{
    gchar buf[2048] = "";
    va_list argptr;

    va_start(argptr, formatString);
    if( attr == BRIGHT )
      colorize_bright();

    /* glib 2.2 provides g_vfprintf */
    vsnprintf(buf, 2048, formatString, argptr);
    g_print("%s", buf);

    colorize_reset();
    va_end(argptr);
}

/* set foreground color and attribute */
void pal_output_fg( gint attr, gint color, gchar *formatString, ...)
{
    gchar buf[2048] = "";
    va_list argptr;

    va_start(argptr, formatString);
    colorize_fg(attr, color);

    /* glib 2.2 provides g_vfprintf */
    vsnprintf(buf, 2048, formatString, argptr);
    g_print("%s", buf);

    colorize_reset();
    va_end(argptr);
}


void pal_output_error(char *formatString, ... )
{
    gchar buf[2048] = "";
    va_list argptr;

    va_start( argptr, formatString );
    colorize_error();

    /* glib 2.2 provides g_vfprintf */
    vsnprintf(buf, 2048, formatString, argptr);
    g_printerr("%s", buf); /* use g_print to convert from UTF-8 */

    colorize_reset();
    va_end(argptr);
}


/* finishes with date on the sunday of the next week */
static void pal_output_text_week(GDate* date, gboolean force_month_label,
				 const GDate* today)
{
    gint i=0;

    if(settings->week_start_monday)
	/* go to last day in week (sun) */
	while(g_date_get_weekday(date) != 7)
	    g_date_add_days(date,1);
    else
	/* go to last day in week (sat) */
	while(g_date_get_weekday(date) != 6)
	    g_date_add_days(date,1);


    for(i=0; i<7; i++)
    {
	if(g_date_get_day(date) == 1)
	    force_month_label = TRUE;

	g_date_subtract_days(date,1);
    }

    g_date_add_days(date,1);
    /* date is now at beginning of week */

    if(force_month_label)
    {
	gchar buf[1024];

	g_date_add_days(date,6);
	g_date_strftime(buf, 128, "%b", date);
	g_date_subtract_days(date,6);

	/* make sure we're only showing 3 characters */
	if(g_utf8_strlen(buf, -1) != 3)
	{
	    /* append whitespace in case "buf" is too short */
	    gchar* s = g_strconcat(buf, "        ", NULL);

	    /* just show first 3 characters of month */
	    g_utf8_strncpy(buf, s, 3);
	    g_free(s);
	}

	pal_output_fg(BRIGHT, GREEN, "%s ", buf);

    }
    else if( settings->show_weeknum )
    {
	gint weeknum = settings->week_start_monday ? g_date_get_monday_week_of_year(date)
	    : g_date_get_sunday_week_of_year(date);
	pal_output_fg(BRIGHT, GREEN, " %2d ", weeknum);
    }

    else
	g_print("    ");


    for(i=0; i<7; i++)
    {
	GList* events = NULL;
	gunichar start=' ', end=' ';
	gchar utf8_buf[8];
	gint color = settings->event_color;
	events = get_events(date);

	if(g_date_compare(date,today) == 0)
	    start = end = '@';

	else if(events != NULL)
	{
	    GList* item  = g_list_first(events);
	    PalEvent *event = (PalEvent*) item->data;

	    gboolean same_char = TRUE;
	    gboolean same_color = TRUE;

	    /* skip to a event that isn't hidden or to the end of the list */
	    while(g_list_length(item) > 1 && event->hide)
	    {
		item = g_list_next(item);
		event = (PalEvent*) item->data;
	    }

	    /* save the markers for the event */
	    if(event->hide)
		start = ' ', end = ' ';
	    else
	    {
		start = event->start;
		end   = event->end;
		color = event->color;
	    }

	    /* if multiple events left */
	    while(g_list_length(item) > 1)
	    {
		item = g_list_next(item);
		event = (PalEvent*) item->data;

		/* find next non-hidden event */
		while(g_list_length(item) > 1 && event->hide)
		{
		    item = g_list_next(item);
		    event = (PalEvent*) item->data;
		}

		/* if this event is hidden, there aren't any more non-hidden events left */
		/* if this event isn't hidden, then determine if it has different markers */
		if( ! event->hide )
		{
		    gunichar new_start = event->start;
		    gunichar new_end   = event->end;
		    gint     new_color = event->color;

		    if(new_start != start || new_end != end)
			same_char = FALSE;
		    if(new_color != color)
			same_color = FALSE;
		}
		if(same_char == FALSE)
		    start = '*', end = '*';
		if(same_color == FALSE)
		    color = -1;
	    }
	}

	utf8_buf[g_unichar_to_utf8(start, utf8_buf)] = '\0';

	/* print color for marker if needed */
	if(start != ' ' && end != ' ')
	{
	    if(color == -1)
		pal_output_fg(BRIGHT, settings->event_color, utf8_buf);
	    else
		pal_output_fg(BRIGHT, color, utf8_buf);
	}
	else
	    g_print(utf8_buf);


	if(g_date_compare(date,today) == 0)	/* make today bright */
	    pal_output_attr(BRIGHT, "%02d", g_date_get_day(date));
	else
	    g_print("%02d", g_date_get_day(date));


	utf8_buf[g_unichar_to_utf8(end, utf8_buf)] = '\0';

	/* print color for marker if needed */
	if(start != ' ' && end != ' ')
	{
	    if(color == -1)
		pal_output_fg(BRIGHT, settings->event_color, utf8_buf);
	    else
		pal_output_fg(BRIGHT, color, utf8_buf);

	}
	else
	    g_print(utf8_buf);


	/* print extra space between days */
	if(i != 6)
	    g_print(" ");

	g_date_add_days(date,1);
	g_list_free(events);
    }

}



static void pal_output_week(GDate* date, gboolean force_month_label, const GDate* today)
{
    
    pal_output_text_week(date, force_month_label, today);

    if(!settings->no_columns && settings->term_cols >= 77)
    {
	pal_output_fg(DIM,YELLOW,"%s","|");

       /* skip ahead to next column */
	g_date_subtract_days(date, 6);
	g_date_add_days(date, settings->cal_lines*7);
	pal_output_text_week(date, force_month_label, today);

	/* skip back to where we were */
	g_date_subtract_days(date, settings->cal_lines*7);

    }

    if(settings->term_cols != 77)
	g_print("\n");

}



void pal_output_cal(gint num_lines, const GDate* today)
{
    gint on_week = 0;
    gchar* week_hdr = NULL;
    GDate* date = NULL;

    if(num_lines <= 0)
	return;
    
    date = g_date_new();
    memcpy(date, today, sizeof(GDate));

    if(settings->week_start_monday)
	week_hdr = g_strdup(_("Mo   Tu   We   Th   Fr   Sa   Su"));
    else
	week_hdr = g_strdup(_("Su   Mo   Tu   We   Th   Fr   Sa"));


    /* if showing enough lines, show previous week. */
    if(num_lines > 3)
	g_date_subtract_days(date, 7);

    if(settings->no_columns || settings->term_cols < 77)
	pal_output_fg(BRIGHT,GREEN, "     %s\n", week_hdr);

    else
    {
	pal_output_fg(BRIGHT,GREEN,"     %s ", week_hdr);
	pal_output_fg(DIM,YELLOW,"%s","|");
	pal_output_fg(BRIGHT,GREEN,"     %s\n", week_hdr);
    }

    g_free(week_hdr);

    while(on_week < num_lines)
    {
	if(on_week == 0)
	    pal_output_week(date, TRUE, today);
	else
	    pal_output_week(date, FALSE, today);

	on_week++;

    }
    g_date_free( date );
}

/* replaces tabs with spaces */
static void pal_output_strip_tabs(gchar* string)
{
    gchar *ptr = string;
    while(*ptr != '\0')
    {
	if(*ptr == '\t')
	    *ptr = ' ';
	ptr++;
    }
}


/* returns the length of the next word */
static gint pal_output_wordlen(gchar* string)
{
    gchar* p = string;
    int i=0;
    while(*p != ' ' && *p != '\0')
    {
	p = g_utf8_next_char(p);
	i++;
    }

    return i;
}


/* This function does not yet handle tabs and color codes.  Tabs
 * should be stripped from 'string' before this is called.
 * "chars_used" indicates the number of characters already used on the
 * line that "string" will be printed out on.
 * Returns the number of lines printed. 
 */
int pal_output_wrap(gchar* string, gint chars_used, gint indent)
{
    gint numlines = 0;
    gchar* s = string;
    gint width = settings->term_cols - 1;  /* -1 to avoid unexpected wrap */
    if( width <= 0 )
        width = 10000;

    while(*s != '\0')
    {
	/* print out any leading whitespace on this line */
	while(*s == ' ' && chars_used < width)
	{
	    g_print(" ");
	    chars_used++;
	    s = g_utf8_next_char(s);
	}

	/* if word doesn't fit on line, split it */
	if(pal_output_wordlen(s)+chars_used > width)
	{
	    gchar line[2048];
	    g_utf8_strncpy(line, s, width - chars_used);
	    g_print("%s\n", line); /* print as much as we can */
	    numlines++;
	    s = g_utf8_offset_to_pointer(s, width - chars_used);
	    chars_used = 0;
	}
	else /* if next word fits on line */
	{

	    while(*s != '\0' &&
		  pal_output_wordlen(s)+chars_used <= width)
	    {

		/* if the next word is not a blank, copy the word */
		if(*s != ' ')
		{
		    gint word_len = pal_output_wordlen(s);
		    gchar word[2048];
		    g_utf8_strncpy(word, s, word_len);
		    g_print("%s", word);
		    s = g_utf8_offset_to_pointer(s, word_len);
		    chars_used += word_len;
		}

		/* print out any spaces that follow the word */
		while(*s == ' ' && chars_used < width)
		{
		    g_print(" ");
		    chars_used++;
		    s = g_utf8_next_char(s);
		}


		/* if we filled line up perfectly, and there is a
		 * space next in the string, ignore it---the newline
		 * will act as the space */
		if(chars_used == width && *s == ' ')
		{
		    s = g_utf8_next_char(s);

		    /* if the next line is a space too, break out of
		     * this loop.  If we don't break, whitespace might
		     * not be preserved properly. */
		    if(*s == ' ')
			break;
		}
	    }

	    numlines++;
	    g_print("\n");
			
	    chars_used = width;
	}

	/* if not done, print indents for next line */
	if(*s != '\0')
	{
	    gint i;
	    
	    /* now, chars_used == width, onto the next line! */
	    chars_used = indent;

	    for(i=0; i<indent; i++)
		g_print(" ");
	}

    }

    return numlines;
    
}




/* If event_number is -1, don't number the events.
   Returns the number of lines printed.
*/
int pal_output_event(const PalEvent* event, const GDate* date, const gboolean selected)
{
    gint numlines = 0;
    gchar date_text[128];
    const gint indent = 2;
    gchar* event_text = NULL;
    date_text[0] = '\0';

    if(selected)
	pal_output_fg(BRIGHT, GREEN, "%s ", ">");
    else if(event->color == -1)
	pal_output_fg(BRIGHT, settings->event_color, "%s ", "*");
    else
	pal_output_fg(BRIGHT, event->color, "%s ", "*");

    pal_output_strip_tabs(event->text);
    pal_output_strip_tabs(event->type);

    event_text = pal_event_escape(event, date);

    if(settings->compact_list)
    {
	gchar* s = NULL;
	g_date_strftime(date_text, 128,
			settings->compact_date_fmt, date);
	pal_output_attr(BRIGHT, "%s ", date_text);
	
	if(settings->hide_event_type)
	    s = g_strconcat(event_text, NULL);
	else
	    s = g_strconcat(event->type, ": ", event_text, NULL);
	
	numlines += pal_output_wrap(s, indent+g_utf8_strlen(date_text,-1)+1, indent);
	g_free(s);
    }
    else
    {
	if(settings->hide_event_type)
	    numlines += pal_output_wrap(event_text, indent, indent);
	else
	{
	    gchar* s = g_strconcat(event->type, ": ", event_text, NULL);
	    numlines += pal_output_wrap(s, indent, indent);
	    g_free(s);
	}
    }
    g_free(event_text);

    return numlines;
}


void pal_output_date_line(const GDate* date)
{
    gchar pretty_date[128];
    gint diff = 0;

    GDate* today = g_date_new();
    g_date_set_time_t(today, time(NULL));

    g_date_strftime(pretty_date, 128, settings->date_fmt, date);

    pal_output_attr(BRIGHT, "%s", pretty_date);
    g_print(" - ");

    diff = g_date_days_between(today, date);
    if(diff == 0)
	pal_output_fg(BRIGHT, RED, "%s", _("Today"));
    else if(diff == 1)
	pal_output_fg(BRIGHT, YELLOW, "%s", _("Tomorrow"));
    else if(diff == -1)
	g_print("%s", _("Yesterday"));
    else if(diff > 1)
	g_print(_("%d days away"), diff);
    else if(diff < -1)
	g_print(_("%d days ago"), -1*diff);

    g_print("\n");

    g_date_free(today);
}


/* outputs the events in the order of PalEvent->file_num.
   Returns the number of lines printed. */
int pal_output_date(GDate* date, gboolean show_empty_days, int selected_event)
{
    gint numlines = 0;
    GList* events = get_events(date);
    gint num_events = g_list_length(events);

    if(events != NULL || show_empty_days)
    {
	GList* item = NULL;
	int i;

	if(!settings->compact_list)
	{
	    pal_output_date_line(date);
	    numlines++;
	}

	item = g_list_first(events);

	for(i=0; i<num_events; i++)
	{
	    numlines += pal_output_event((PalEvent*) (item->data),
					 date, i==selected_event);

	    item = g_list_next(item);
	}


	if(num_events == 0)
	{
	    if(settings->compact_list)
	    {
		gchar pretty_date[128];

		g_date_strftime(pretty_date, 128,
				settings->compact_date_fmt, date);
		pal_output_attr(BRIGHT, "  %s ", pretty_date);
		g_print("%s\n", _("No events."));
		
		numlines++;
	    } 
	    else
	    {
		g_print("%s\n", _("No events."));
		numlines++;
	    }
	}

	if(!settings->compact_list)
	{
	    g_print("\n");
	    numlines++;
	}
    }

    return numlines;
}



/* returns the PalEvent for the given event_number */
PalEvent* pal_output_event_num(const GDate* date, gint event_number)
{
    GList* events = get_events(date);
    gint num_events = g_list_length(events);

    if(events == NULL || event_number < 1 || event_number > num_events)
	return NULL;

    return (PalEvent*) g_list_nth_data(events, event_number-1);
}


