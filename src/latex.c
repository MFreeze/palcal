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
#include <string.h>

#include "main.h"
#include "event.h"

/* prints out the string but properly escapes things for LaTeX */
static void pal_latex_escape_print(gchar* s)
{
    gunichar c;
    /* Note: g_print uses the current locale encoding, even though we print UTF-8 */
    while( (c = g_utf8_get_char(s)) != '\0')
    {
	if(c == '&')        /* & to \& */
	    g_print("\\&");
	else if(c == '$')   /* $ to \$ */
	    g_print("\\$");
	else if(c == '\\')  /* \ to \\ */
	    g_print("\\\\");
	else if(c == '}')   /* } to \} */
	    g_print("\\}");
	else if(c == '{')   /* { to \{ */
	    g_print("\\{");
	else if(c == '#')   /* # to \# */
	    g_print("\\#");
	else if(c == '%')   /* % to \% */
	    g_print("\\%%");
	else if(c == '^')   /* ^ to \^ */
	    g_print("\\^");
	else if(c == '_')   /* _ to \_ */
	    g_print("\\_");
	else if(c >= 32 && c < 128)
	    g_print("%c", c);
        else
	    g_print("\\unichar{%d}", c); /* print 1 char */

	s = g_utf8_next_char(s);
    }
}


/* finishes with date on the first day in the next month */
static void pal_latex_month(GDate* date, gboolean force_month_label)
{
    gint i;
    gchar buf[1024];
    gint orig_month = g_date_get_month(date);

    g_date_strftime(buf, 1024, "%B %Y", date);

    g_print("%s%s%s", "\\textbf{\\Large ", buf, "}\n");
    g_print("%s", "\\smallskip\n");

    g_print("%s", "\\begin{tabularx}{10.0in}{|p{1.26in}|p{1.26in}|p{1.26in}|p{1.26in}|p{1.26in}|p{1.26in}|p{1.24in}|}\n");
    g_print("%s", "\\hline\n");

    if(!settings->week_start_monday)
	g_print("%s%s%s&", "\\textbf{", _("Sunday"), "}");

    g_print("%s%s%s&", "\\textbf{", _("Monday"), "}");
    g_print("%s%s%s&", "\\textbf{", _("Tuesday"), "}");
    g_print("%s%s%s&", "\\textbf{", _("Wednesday"), "}");
    g_print("%s%s%s&", "\\textbf{", _("Thursday"), "}");
    g_print("%s%s%s&", "\\textbf{", _("Friday"), "}");
    g_print("%s%s%s", "\\textbf{", _("Saturday"), "}");

    if(settings->week_start_monday)
	g_print("&%s%s%s", "\\textbf{", _("Sunday"), "}");

    g_print("\\\\\n");

    g_print("\\hline \\hline\n");

    /* start the month on the right weekday */
    if(settings->week_start_monday)
	for(i=0; i<g_date_get_weekday(date)-1; i++)
	    g_print(" & ");
    else
    {
	if(g_date_get_weekday(date) != 7)
	    for(i=0; i<g_date_get_weekday(date); i++)
		g_print(" & ");
    }


    /* fill in stuff */
    while(g_date_get_month(date) == orig_month)
    {
	GList* events = get_events(date);
	gint num_events = g_list_length(events);
	gint num_events_printed = 0;
	GList* item;

	g_print("\\textbf{\\textit{\\Large %d}} {\\raggedright\n", g_date_get_day(date));

	item = g_list_first(events);

	/* while theres more events to be displayed */
	while(num_events > num_events_printed)
	{
	    gchar* event_text = pal_event_escape((PalEvent*) (item->data), date);
	    g_print("$\\cdot$");
	    pal_latex_escape_print(event_text);
	    g_print("\n\n");
	    num_events_printed++;
	    item = g_list_next(item);
	    g_free(event_text);
	}

	g_print("}");
	if(num_events == 0)
	    g_print("\\vspace{.9in}");
	else
	    g_print("\\vspace{.3in}");


	if((settings->week_start_monday && g_date_get_weekday(date) == 7) ||
	   (!settings->week_start_monday && g_date_get_weekday(date) == 6))
	{
	    if(!g_date_is_last_of_month(date))
		g_print("\\\\ \\hline\n");
	    else
		g_print("\n");
	}
	else
	    g_print (" &\n");

	g_date_add_days(date,1);

	g_list_free(events);
    }

    /* skip to end of calendar */
    if(settings->week_start_monday)
    {
	int tmp = g_date_get_weekday(date);
	if(tmp == 1) tmp = 7;
	while(tmp != 7)
	{
	    g_print(" & ");
	    tmp++;
	}
    }
    else
    {
	int tmp = g_date_get_weekday(date);
	if(tmp == 7) tmp = 6;
	while(tmp != 6)
	{
	    g_print(" & ");
	    tmp++;
	}
    }

    g_print("\\\\ \\hline \\end{tabularx}\n");


}




void pal_latex_out(void)
{
    gint on_month = 0;
    GDate* date = g_date_new();

    if( settings->query_date )
	memcpy( date, settings->query_date, sizeof(GDate) );
    else
	g_date_set_time_t(date, time(NULL));

    g_print("%s%s\n", "% Generated with pal ", PAL_VERSION);

    g_print("%s", "\\documentclass[12pt]{article}\n");
    g_print("%s", "\\usepackage{lscape}\n");
    g_print("%s", "\\usepackage{tabularx}\n\n");
    g_print("%s", "%% Uncomment this line if you want you have unicode output\n");
    g_print("%s", "%%\\usepackage{ucs}\n");
    g_print("%s", "\\setlength{\\hoffset}{-.5in}\n");
    g_print("%s", "\\setlength{\\oddsidemargin}{0in}\n");
    g_print("%s", "\\setlength{\\evensidemargin}{0in}\n");
    g_print("%s", "\\setlength{\\voffset}{-.5in}\n");
    g_print("%s", "\\setlength{\\topmargin}{0in}\n");
    g_print("%s", "\\setlength{\\headheight}{0in}\n");
    g_print("%s", "\\setlength{\\headsep}{0in}\n");
    g_print("%s", "\\setlength{\\textheight}{10in}\n");
    g_print("%s", "\\setlength{\\textwidth}{7.5in}\n");
    g_print("%s", "\\setlength{\\marginparwidth}{0in}\n");
    g_print("%s", "\\setlength{\\marginparsep}{0in}\n\n");

    g_print("%s", "\\pagestyle{empty}\n");
    g_print("%s", "\\parindent=0pt\n\n");

    g_print("%s", "\\begin{document}\n");
    g_print("%s", "\\begin{landscape}\n\n");

    g_print("%s", "\\begin{center}\n\n");

    /* back up to the first of the month */
    g_date_subtract_days(date, g_date_get_day(date) - 1);

    for(on_month=0; on_month < settings->cal_lines; on_month++)
    {
	if(on_month != 0)
	    g_print("%s", "\\newpage\n");

	pal_latex_month(date, TRUE);
    }

    g_print("%s", "\n\\end{center}\n");
    g_print("%s", "\\end{landscape}\n");
    g_print("%s", "\\end{document}\n");
}
