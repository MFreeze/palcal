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


#include <time.h>

#include "main.h"
#include "event.h"
#include "html.h"
#include "colorize.h"

/* prints out the string but properly escapes things for HTML */
static void pal_html_escape_print(gchar* s)
{
    gunichar c;
    while( (c = g_utf8_get_char(s)) != '\0')
    {
	if(c == '<')        /* < to &lt; */
	    g_print("&lt;");
	else if(c == '>')   /* > to &gt; */
	    g_print("&gt;");
	else if(c == '&')   /* & to *amp; */
	    g_print("&amp;");
	else if(c >= 32 && c < 128)  /* ASCII */
	    g_print("%c", c);
        else
            g_print("&#%d;", c); /* unicode */

	s = g_utf8_next_char(s);
    }
}



/* finishes with date on the first day of the next month */
static void pal_html_month(GDate* date, gboolean force_month_label,
			  const GDate* today)
{
    gint orig_month = g_date_get_month(date);
    int i;
    gchar buf[1024] = "";
    gchar start[64] = "<td class='pal-dayname' align='center'>";
    gchar end[64] = "</td>";

    fputs("<table class='pal-cal' cellspacing='0' cellpadding='1'>\n", stdout);

    g_date_strftime(buf, 1024, "%B %Y", date);
    g_print("<tr><td class='pal-month' align='center' colspan='7'>%s</td></tr>\n", buf);

    fputs("<tr>\n", stdout);

    if(!settings->week_start_monday)
	g_print("%s%s%s\n", start, _("Sunday"), end);

    g_print("%s%s%s\n", start, _("Monday"), end);
    g_print("%s%s%s\n", start, _("Tuesday"), end);
    g_print("%s%s%s\n", start, _("Wednesday"), end);
    g_print("%s%s%s\n", start, _("Thursday"), end);
    g_print("%s%s%s\n", start, _("Friday"), end);
    g_print("%s%s%s\n", start, _("Saturday"), end);

    if(settings->week_start_monday)
	g_print("%s%s%s\n", start, _("Sunday"), end);

    fputs("</tr>\n", stdout);

    /* start the month on the right weekday */
    if(settings->week_start_monday)
    {
    	if(g_date_get_weekday(date) != 1)
    	{
	    fputs("<tr>\n", stdout);
	    
	    for(i=0; i<g_date_get_weekday(date)-1; i++)
	    {
	    	fputs("<td class='pal-blank'>&nbsp;</td>\n", stdout);
	    }
	}
    }
    else
    {
	if(g_date_get_weekday(date) != 7)
	{
	    fputs("<tr>\n", stdout);
	    
	    for(i=0; i<g_date_get_weekday(date); i++)
	    {
		fputs("<td class='pal-blank'>&nbsp;</td>\n", stdout);
	    }
	}
    }


    while(g_date_get_month(date) == orig_month)
    {

	GList* events = get_events(date);
	gint num_events = g_list_length(events);
	gint num_events_printed = 0;
	GList* item = NULL;

	if(( settings->week_start_monday && g_date_get_weekday(date) == 1) ||
	   (!settings->week_start_monday && g_date_get_weekday(date) == 7))
	    fputs("<tr>\n", stdout);

	/* make today bright */
	if(g_date_compare(date,today) == 0)
	    g_print("<td class='pal-today' valign='top'><b>%02d</b><br />\n", g_date_get_day(date));
	else
	{
	    switch(g_date_get_weekday(date))
	    {
		case 1:
		    g_print("<td class='pal-mon' valign='top'><b>%02d</b><br />\n", g_date_get_day(date));
		    break;
		case 2:
		    g_print("<td class='pal-tue' valign='top'><b>%02d</b><br />\n", g_date_get_day(date));
		    break;
		case 3:
		    g_print("<td class='pal-wed' valign='top'><b>%02d</b><br />\n", g_date_get_day(date));
		    break;
		case 4:
		    g_print("<td class='pal-thu' valign='top'><b>%02d</b><br />\n", g_date_get_day(date));
		    break;
		case 5:
		    g_print("<td class='pal-fri' valign='top'><b>%02d</b><br />\n", g_date_get_day(date));
		    break;
		case 6:
		    g_print("<td class='pal-sat' valign='top'><b>%02d</b><br />\n", g_date_get_day(date));
		    break;
		case 7:
		    g_print("<td class='pal-sun' valign='top'><b>%02d</b><br />\n", g_date_get_day(date));
		    break;
		default: /* shouldn't happen */
		    break;

	    }
	}

	item = g_list_first(events);

	/* while there are more events to be displayed */
	while(num_events > num_events_printed)
	{
	    gchar* event_text = pal_event_escape((PalEvent*) (item->data), date);
	    g_print("<span class='pal-event-%s'>\n", string_color_of(((PalEvent*) (item->data))->color));
	    fputs("<b>*</b> ", stdout);
	    pal_html_escape_print(event_text);
	    fputs("<br />\n", stdout);
	    fputs("</span>\n", stdout);
	    num_events_printed++;
	    item = g_list_next(item);
	    g_free(event_text);
	}

	g_print("</td>\n");

	if((settings->week_start_monday && g_date_get_weekday(date) == 7) ||
	   (!settings->week_start_monday && g_date_get_weekday(date) == 6))
	    fputs("</tr>\n", stdout);

	g_date_add_days(date,1);
	g_list_free(events);
    }

    /* we are on the first day of the next month, go back to the last
     * day */
    g_date_add_days(date, -1);

    /* skip to end of calendar */
    if(settings->week_start_monday)   /* set i to the number of blanks to print */
	i = 7 - g_date_get_weekday(date);
    else
    {
	if(g_date_get_weekday(date) == 7)
	    i = 6;
	else
	    i = 6 - g_date_get_weekday(date);
    }

    while(i > 0)
    {
	fputs("<td class='pal-blank'>&nbsp;</td>", stdout);
	i--;
    }


    fputs("</tr></table>\n", stdout);

    /* jump one day ahead to the first day of the next month */
    g_date_add_days(date, 1);
}




void pal_html_out()
{
    gint on_month = 0;
    GDate* today = g_date_new();
    GDate* date = g_date_new();


    if(settings->query_date == NULL)
    {
	g_date_set_time_t(today, time(NULL));
	g_date_set_time_t(date, time(NULL));
    }
    else
    {
	g_date_set_day(today, g_date_get_day(settings->query_date));
	g_date_set_month(today, g_date_get_month(settings->query_date));
	g_date_set_year(today, g_date_get_year(settings->query_date));
	g_date_set_day(date, g_date_get_day(settings->query_date));
	g_date_set_month(date, g_date_get_month(settings->query_date));
	g_date_set_year(date, g_date_get_year(settings->query_date));
    }

    /* back up to the first of the month */
    g_date_subtract_days(date, g_date_get_day(date) - 1);

    g_print("%s %s %s", "<!-- Generated with pal", PAL_VERSION, "-->\n");

    for(on_month=0; on_month < settings->cal_lines; on_month++)
	pal_html_month(date, TRUE, today);

    g_print("<div class='pal-tagline'><p><i>%s</i></p></div>\n",
	    _("Calendar created with <a href='http://palcal.sourceforge.net/'>pal</a>."));

}
