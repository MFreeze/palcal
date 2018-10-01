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
#include <stdlib.h>

#include "main.h"
#include "output.h"
#include "event.h"
#include "rl.h"
#include "input.h"
#include "edit.h"

/* escape ' */
static void pal_remind_escape(gchar *string, FILE* tmp_stream)
{
    while(*string != '\0')
    {
	if(*string == '$'  ||
	   *string == '`'  ||
	   *string == '"'  ||
	   *string == '\\')
	{
	    fputc('\\', tmp_stream);
	    fputc(*string, tmp_stream);
	}
	else
	    fputc(*string, tmp_stream);
	string++;
    }
}

static void pal_remind_event(void)
{
    PalEvent* remind_event = NULL;
    GDate* event_date = NULL;
    gchar* at_string;
    gchar tmp_name[] = "/tmp/pal-XXXXXX";
    FILE* tmp_stream;
    int return_val;
    gchar* email_add;
    G_CONST_RETURN gchar *charset;
    at_string = g_malloc(1024*sizeof(gchar));

    pal_output_fg(BRIGHT, GREEN, "* * * ");
    pal_output_attr(BRIGHT, _("Event reminder"));
    pal_output_fg(BRIGHT, GREEN, " * * *\n");

    pal_output_fg(BRIGHT, GREEN, "> ");
    pal_output_wrap(_("This feature allows you to select one event and have an email sent to you about the event at a date/time that you provide.  If the event is recurring, you will only receive one reminder.  You MUST have atd, crond and sendmail installed and working for this feature to work."),2,2);

    g_print("\n");

    remind_event = pal_rl_get_event(&event_date, TRUE);
    g_print("\n");

    if(remind_event->start_time != NULL)
    {
	snprintf(at_string, 1024, "%02d:%02d %04d-%02d-%02dW",
		 remind_event->start_time->hour,
		 remind_event->start_time->min,
		 g_date_get_year(event_date),
		 g_date_get_month(event_date),
		 g_date_get_day(event_date));

    }
    else
    {
	snprintf(at_string, 1024, "%02d:%02d %04d-%02d-%02d", 0,0,
		 g_date_get_year(event_date),
		 g_date_get_month(event_date),
		 g_date_get_day(event_date));

    }


#if 0
    pal_rl_default_text = at_string;
    rl_pre_input_hook = (rl_hook_func_t*) pal_rl_default_text_fn;
    at_string = pal_rl_get_line(_("Remind me on (HH:MM YYYY-MM-DD): "), settings->term_rows-2, 0);
    rl_pre_input_hook = NULL;
#endif

    at_string = pal_rl_get_line_default(_("Remind me on (HH:MM YYYY-MM-DD): "), settings->term_rows-2, 0, at_string);

#if 0
    pal_rl_default_text = g_strdup(g_get_user_name());
    rl_pre_input_hook = (rl_hook_func_t*) pal_rl_default_text_fn;
    email_add = pal_rl_get_line(_("Username on local machine or email address: "), settings->term_rows-2, 0);
    rl_pre_input_hook = NULL;
#endif
    
    email_add = pal_rl_get_line_default(_("Username on local machine or email address: "), settings->term_rows-2, 0, g_strdup(g_get_user_name()));

    mkstemp(tmp_name);
    tmp_stream = fopen(tmp_name, "w");

    fputs("echo \"", tmp_stream);
    fputs("From: \"pal\" <pal>\n", tmp_stream);
    fputs("To: ", tmp_stream);
    fputs(email_add, tmp_stream);
    fputs("\n", tmp_stream);

    g_get_charset(&charset);
    fputs("Content-Type: text/plain; charset=", tmp_stream);
    fputs(charset, tmp_stream);
    fputs("\n", tmp_stream);

    fputs("Subject: [pal] ", tmp_stream);
    pal_remind_escape(g_strndup(remind_event->text, 128), tmp_stream);
    fputs("\n\n", tmp_stream);

    fputs(_("Event: "), tmp_stream);
    pal_remind_escape(remind_event->text, tmp_stream);
    fputs("\n", tmp_stream);

    fputs(_("Event date: "), tmp_stream);

    {
	gchar pretty_date[128];
	g_date_strftime(pretty_date, 128, settings->date_fmt, event_date);
	fputs(pretty_date, tmp_stream);
    }
    fputs("\n", tmp_stream);

    fputs(_("Event type: "), tmp_stream);
    pal_remind_escape(remind_event->type, tmp_stream);
    fputs("\n", tmp_stream);
    fputs("\"| /usr/sbin/sendmail ", tmp_stream);
    fputs(email_add, tmp_stream);

    fclose(tmp_stream);

    pal_output_fg(BRIGHT, GREEN, "> ");
    g_print(_("Attempting to run 'at'...\n"));
    g_print("at -f %s %s\n", tmp_name, at_string);
    return_val = system(g_strconcat("at -f ", tmp_name, " ", at_string, NULL));

    if(return_val != 0)
	pal_output_error(_("ERROR: Date string was invalid or could not run 'at'.  Is 'atd' running?"));
    else
    {
	pal_output_fg(BRIGHT, GREEN, ">>> ");
	g_print(_("Successfully added event to the 'at' queue.\n"));
    }

    remove(tmp_name);
}
