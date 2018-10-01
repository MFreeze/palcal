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


#include <sys/ioctl.h> /* get # columns for terminal */
#include <time.h>
#include <string.h>
#include <locale.h>
#include <sys/types.h> /* FreeBSD, regex.h needs this */
#include <regex.h>     /* regular expressions */
#include <glib.h>

#include <ncurses.h>

#include "output.h"
#include "main.h"
#include "input.h"
#include "event.h"

#include "rl.h"
#include "html.h"
#include "latex.h"
#include "search.h"

#include "manage.h"

Settings* settings;
GHashTable* ht;     /* ht holds the loaded events */

/* prints the events on the dates from the starting_date to
 * starting_date+window */
static void view_range(GDate* starting_date, gint window)
{
    gint i;

    if(settings->reverse_order)
	g_date_add_days(starting_date,window-1);

    for(i=0; i<window; i++)
    {
	pal_output_date(starting_date,FALSE,-1);

	if(settings->reverse_order)
	    g_date_subtract_days(starting_date,1);
	else
	    g_date_add_days(starting_date,1);
    }
}



/* Returns a GDate object for the given key (in_string) */
/* This function only checks the one-time date events (not todo, or
 * recurring events).  It returns NULL on a failure and can optionally
 * print an error message on failure. */
GDate* get_query_date(gchar* in_string, gboolean show_error)
{
    GDate* to_show = NULL;
    gchar* date_string = g_ascii_strdown(in_string, -1);

    if(date_string == NULL)
	return NULL;

    g_strstrip(date_string);

    to_show = g_date_new();
    g_date_set_time_t(to_show, time(NULL));

    /* these could be better... */
    if(strncmp(date_string, _("tomorrow"), strlen(_("tomorrow"))) == 0)
    {
	g_date_add_days(to_show, 1);
	g_free(date_string);
	return to_show;
    }
    if(strncmp(date_string, _("yesterday"), strlen(_("yesterday"))) == 0)
    {
	g_date_subtract_days(to_show, 1);
	g_free(date_string);
	return to_show;
    }
    if(strncmp(date_string, _("today"), strlen(_("today"))) == 0)
    {
	g_date_subtract_days(to_show, 0);
	g_free(date_string);
	return to_show;
    }
    if(strncmp(date_string, _("mo"), strlen(_("mo"))) == 0 ||
       strncmp(date_string, _("next mo"), strlen(_("next mo"))) == 0)
    {
	do g_date_add_days(to_show, 1); while(g_date_get_weekday(to_show) != 1);
	g_free(date_string);
	return to_show;
    }
    if(strncmp(date_string, _("tu"), strlen(_("tu"))) == 0 ||
       strncmp(date_string, _("next tu"), strlen(_("next tu"))) == 0)
    {
	do g_date_add_days(to_show, 1); while(g_date_get_weekday(to_show) != 2);
	g_free(date_string);
	return to_show;
    }
    if(strncmp(date_string, _("we"), strlen(_("we"))) == 0 ||
       strncmp(date_string, _("next we"), strlen(_("next we"))) == 0)
    {
	do g_date_add_days(to_show, 1); while(g_date_get_weekday(to_show) != 3);
	g_free(date_string);
	return to_show;
    }
    if(strncmp(date_string, _("th"), strlen(_("th"))) == 0 ||
       strncmp(date_string, _("next th"), strlen(_("next th"))) == 0)
    {
	do g_date_add_days(to_show, 1); while(g_date_get_weekday(to_show) != 4);
	g_free(date_string);
	return to_show;
    }
    if(strncmp(date_string, _("fr"), strlen(_("fr"))) == 0 ||
       strncmp(date_string, _("next fr"), strlen(_("next fr"))) == 0)
    {
	do g_date_add_days(to_show, 1); while(g_date_get_weekday(to_show) != 5);
	g_free(date_string);
	return to_show;
    }
    if(strncmp(date_string, _("sa"), strlen(_("sa"))) == 0 ||
       strncmp(date_string, _("next sa"), strlen(_("next sa"))) == 0)
    {
	do g_date_add_days(to_show, 1); while(g_date_get_weekday(to_show) != 6);
	return to_show;
    }
    if(strncmp(date_string, _("su"), strlen(_("su"))) == 0 ||
       strncmp(date_string, _("next su"), strlen(_("next su"))) == 0)
    {
	do g_date_add_days(to_show, 1); while(g_date_get_weekday(to_show) != 7);
	g_free(date_string);
	return to_show;
    }
    if(strncmp(date_string, _("last mo"), strlen(_("last mo"))) == 0)
    {
	do g_date_subtract_days(to_show, 1); while(g_date_get_weekday(to_show) != 1);
	g_free(date_string);
	return to_show;
    }
    if(strncmp(date_string, _("last tu"), strlen(_("last tu"))) == 0)
    {
	do g_date_subtract_days(to_show, 1); while(g_date_get_weekday(to_show) != 2);
	g_free(date_string);
	return to_show;
    }
    if(strncmp(date_string, _("last we"), strlen(_("last we"))) == 0)
    {
	do g_date_subtract_days(to_show, 1); while(g_date_get_weekday(to_show) != 3);
	g_free(date_string);
	return to_show;
    }
    if(strncmp(date_string, _("last th"), strlen(_("last th"))) == 0)
    {
	do g_date_subtract_days(to_show, 1); while(g_date_get_weekday(to_show) != 4);
	g_free(date_string);
	return to_show;
    }
    if(strncmp(date_string, _("last fr"), strlen(_("last fr"))) == 0)
    {
	do g_date_subtract_days(to_show, 1); while(g_date_get_weekday(to_show) != 5);
	g_free(date_string);
	return to_show;
    }
    if(strncmp(date_string, _("last sa"), strlen(_("last sa"))) == 0)
    {
	do g_date_subtract_days(to_show, 1); while(g_date_get_weekday(to_show) != 6);
	g_free(date_string);
	return to_show;
    }
    if(strncmp(date_string, _("last su"), strlen(_("last su"))) == 0)
    {
	do g_date_subtract_days(to_show, 1); while(g_date_get_weekday(to_show) != 7);
	g_free(date_string);
	return to_show;
    }

    /* use regexs here for easier localization */
    {
	regex_t preg;

	regcomp(&preg, _("^[0-9]+ days away$"), REG_ICASE|REG_NOSUB|REG_EXTENDED);

	if(regexec(&preg, date_string, 0, NULL, 0)==0)
	{
	    gchar* ptr = date_string;
	    gint date_offset = 0;
	    while(!g_ascii_isdigit(*ptr) && ptr != NULL)
		ptr = g_utf8_find_next_char(ptr, NULL);

	    sscanf(ptr, "%d", &date_offset);

	    g_date_add_days(to_show, date_offset);
	    g_free(date_string);
	    regfree(&preg);
	    return to_show;
	}

	regfree(&preg);
	regcomp(&preg, _("^[0-9]+ days ago$"), REG_ICASE|REG_NOSUB|REG_EXTENDED);

	if(regexec(&preg, date_string, 0, NULL, 0)==0)
	{
	    gchar* ptr = date_string;
	    gint date_offset = 0;
	    while(!g_ascii_isdigit(*ptr) && ptr != NULL)
		ptr = g_utf8_find_next_char(ptr, NULL);

	    sscanf(ptr, "%d", &date_offset);

	    g_date_subtract_days(to_show, date_offset);
	    g_free(date_string);
	    regfree(&preg);
	    return to_show;
	}

	regfree(&preg);
    }


    if(g_ascii_isdigit(*(date_string))) /* if it begins with a digit ... */
    {
	gchar* ptr = date_string;

	while(g_ascii_isdigit(*ptr))
	    ptr++;

	/* ... and if the string is entirely made up of digits */
	if(*ptr == '\0')
	{
	    gint query_date_int = -1;

	    sscanf(date_string, "%d", &query_date_int);


	    /* check for 0, negative number, or excessive preceeding 0's */
	    if(!(query_date_int <= 0 ||
		(*date_string == '0' && *(date_string+1) == '0' &&
		 *(date_string+2) == '0' && *(date_string+3) == '0') ||
		(*(date_string+4) == '0' && *(date_string+5) == '0') ||
		(*(date_string+6) == '0' && *(date_string+7) == '0')))
	    {

		if(query_date_int < 32)
		{
		    if(query_date_int < g_date_get_day(to_show))
			g_date_add_months(to_show,1);

		    if(g_date_valid_dmy(query_date_int,
					(GDateMonth) g_date_get_month(to_show),
					(GDateYear) g_date_get_year(to_show)))
		    {
			g_date_set_day(to_show, query_date_int);
			g_free(date_string);
			return to_show;
		    }
		}
		else if(query_date_int < 1232)
		{
		    gint day, month;

		    day = query_date_int % 100;
		    month = query_date_int / 100;

		    if(day > 0 && day < 32 && month > 0 && month < 13)
		    {
			if(month < g_date_get_month(to_show))
			    g_date_add_years(to_show,1);
			if(day   < g_date_get_day(to_show) && month == g_date_get_month(to_show))
			    g_date_add_years(to_show, 1);

			if(g_date_valid_dmy((GDateDay) day,
					    (GDateMonth) month,
					    (GDateYear) g_date_get_year(to_show)))
			{
			    g_date_set_dmy(to_show, (GDateDay) day,
					   (GDateMonth) month,
					   (GDateYear) g_date_get_year(to_show));
			    g_free(date_string);
			    return to_show;
			}
		    }
		}
		else if(query_date_int > 10000)
		{
		    gint day, month, year;

		    day = query_date_int % 100;
		    month = (query_date_int % 10000 - day) / 100;
		    year = query_date_int / 10000;

		    if(day > 0 && day < 32 && month > 0 && month < 13 && year > 0)
		    {
			if(g_date_valid_dmy((GDateDay) day,
					    (GDateMonth) month,
					    (GDateYear) year))
			{
			    g_date_set_dmy(to_show, (GDateDay) day,
					   (GDateMonth) month,
					   (GDateYear) year);
			    g_free(date_string);
			    return to_show;
			}
		    }
		}
	    }
	}
    }

    /* glib is last resort, but don't let a few things get to it... */
    if(!((*date_string == '0' && *(date_string+1) == '0' &&
	  *(date_string+2) == '0' && *(date_string+3) == '0') ||
	 (*(date_string+4) == '0' && *(date_string+5) == '0') ||
	 (*(date_string+6) == '0' && *(date_string+7) == '0')))
    {
	g_date_set_parse(to_show, date_string);

	if(g_date_valid(to_show))
	{
	    g_free(date_string);
	    return to_show;
	}
    }

    if(show_error)
    {
	/* if we got here, there was an error */
	pal_output_error(_("ERROR: The following date is not valid: %s\n"), date_string);
	pal_output_error(  "       %s\n", _("Valid date formats include:"));
	pal_output_error(  "       %s '%s', '%s', '%s',\n", _("dd, mmdd, yyyymmdd,"), _("yesterday"), _("today"), _("tomorrow"));
	pal_output_error(  "       %s\n", _("'n days away', 'n days ago',"));
	pal_output_error(  "       %s\n", _("first two letters of weekday,"));
	pal_output_error(  "       %s\n", _("'next ' followed by first two letters of weekday,"));
	pal_output_error(  "       %s\n", _("'last ' followed by first two letters of weekday,"));
	pal_output_error(  "       %s\n", _("'1 Jan 2000', 'Jan 1 2000', etc."));
    }
    g_date_free(to_show);
    g_free(date_string);
    return NULL;
}


/* determines what should be dispalyed if -r, -s, -d are used */
static void view_details(void)
{
    GDate* to_show = settings->query_date;


    if(settings->search_string != NULL &&
       settings->range_days == 0 &&
       settings->range_neg_days == 0)
    {
	g_print("\n");
	pal_output_fg(BRIGHT, RED, "> ");
	pal_output_wrap(_("NOTE: You can use -r to specify the range of days to search.  By default, pal searches days within one year of today."),2,2);
	settings->range_days = 365;
    }

    /* if -r and -s isn't used */
    if(settings->range_days == 0 &&
       settings->range_neg_days == 0 &&
       settings->search_string == NULL)
    {
	/* if -d is used, show that day.  Otherwise, show nothing */
	if(to_show != NULL)
	    pal_output_date(to_show, TRUE, -1);
    }

    /* if -r or -s is used, show range of dates relative to -d */
    else if(settings->range_days     != 0 ||
	    settings->range_neg_days != 0)
    {
	GDate* starting_date = NULL;

	if(to_show == NULL) /* if -d isn't used, start from current date */
	{
	    starting_date = g_date_new();
	    g_date_set_time_t(starting_date, time(NULL));
	}
	else /* otherwise, start from date specified */
	    starting_date = g_memdup(to_show, sizeof(GDate));

	g_date_subtract_days(starting_date, settings->range_neg_days);

	if(settings->search_string == NULL)
	    view_range(starting_date, settings->range_neg_days + settings->range_days);
	else
	    pal_search_view(settings->search_string, starting_date, settings->range_neg_days + settings->range_days, FALSE);

	g_date_free(starting_date);
    }

}



static gint parse_arg(gchar** args, gint on_arg, gint total_args)
{
    args = args + on_arg;
    on_arg++;

    
    if(strcmp(*args,"-h") == 0 ||
       strcmp(*args,"--help") == 0)
    {
	g_print("%s %s - %s\n", "pal", PAL_VERSION, _("Copyright (C) 2006, Scott Kuhl"));

	g_print("  ");
	pal_output_wrap(_("pal is licensed under the GNU General Public License and has NO WARRANTY."), 2, 2);
	g_print("\n");

	pal_output_wrap(_(" -d date      Show events on the given date.  Valid formats for date include: dd, mmdd, yyyymmdd, 'Jan 1 2000'.  Run 'man pal' for a list of all valid formats."), 0, 16);
	pal_output_wrap(_(" -r n         Display events within n days after today or a date used with -d. (default: n=0, show nothing)"), 0, 16);
	pal_output_wrap(_(" -r p-n       Display events within p days before and n days after today or a date used with -d."), 0, 16);
	pal_output_wrap(_(" -s regex     Search for events matching the regular expression. Use -r to select range of days to search."), 0, 16);
	pal_output_wrap(_(" -x n         Expunge events that are n or more days old."), 0, 16);

	pal_output_wrap(_(" -c n         Display calendar with n lines. (default: 5)"), 0, 16);
	pal_output_wrap(_(" -f file      Load 'file' instead of ~/.pal/pal.conf"), 0, 16);
	pal_output_wrap(_(" -u username  Load /home/username/.pal/pal.conf"), 0, 16);
	pal_output_wrap(_(" -p palfile   Load *.pal file only (overrides files loaded from pal.conf)"), 0, 16);
	pal_output_wrap(_(" -m           Add/Modify/Delete events interactively."), 0, 16);
	pal_output_wrap(_(" --color      Force colors, regardless of terminal type."), 0, 16);
	pal_output_wrap(_(" --nocolor    Force no colors, regardless of terminal type."), 0, 16);
	pal_output_wrap(_(" --mail       Generate output readable by sendmail."), 0, 16);
	pal_output_wrap(_(" --html       Generate HTML calendar.  Set size of calendar with -c."), 0, 16);
	pal_output_wrap(_(" --latex      Generate LaTeX calendar.  Set size of calendar with -c."), 0, 16);
	pal_output_wrap(_(" -v           Verbose output."), 0, 16);
	pal_output_wrap(_(" --version    Display version information."), 0, 16);
	pal_output_wrap(_(" -h, --help   Display this help message."), 0, 16);

	g_print("\n");
	pal_output_wrap(_("Type \"man pal\" for more information."), 0, 16);
	exit(0);
    }

    if(strcmp(*args,"-r") == 0)
    {
	args++; on_arg++;

	if(on_arg > total_args   || (sscanf(*args, "%d", &(settings->range_days)) != 1 &&
				     sscanf(*args, "%d-%d", &(settings->range_neg_days),
					    &(settings->range_days)) != 2))
	{
	    settings->range_days = 0;
	    settings->range_neg_days = 0;

	    pal_output_error("%s\n", _("ERROR: Number required after -r argument."));
	    pal_output_error("       %s\n", _("Use --help for more information."));
	    return on_arg-1;
	}

	settings->range_arg = TRUE;

	if(sscanf(*args, "%d-%d", &(settings->range_neg_days),
		  &(settings->range_days)) == 2)
	    return on_arg;

	if(sscanf(*args, "%d", &(settings->range_days)) == 1)
	{
	    settings->range_neg_days = 0;
	    return on_arg;
	}

	return on_arg;
    }

    if(strcmp(*args,"-c") == 0)
    {
	args++; on_arg++;
	if(on_arg > total_args || sscanf(*args, "%d", &(settings->cal_lines)) != 1)
	{
	    pal_output_error("%s\n", _("ERROR: Number required after -c argument."));
	    pal_output_error("       %s\n", _("Use --help for more information."));
	    on_arg--;
	}
	return on_arg;
    }

    if(strcmp(*args,"-d") == 0)
    {
	args++; on_arg++;
	if(on_arg > total_args)
	{
	    pal_output_error("%s\n", _("ERROR: Date required after -d argument."));
	    pal_output_error("       %s\n", _("Use --help for more information."));
	    on_arg--;
	}
	else
	{
	    gchar *utf8arg = g_locale_to_utf8(*args, -1, NULL, NULL, NULL);
	    settings->query_date = get_query_date(utf8arg, TRUE);
	    g_free(utf8arg);
	    if(settings->query_date == NULL)
		pal_output_error(_("NOTE: Use quotes around the date if it has spaces.\n"));
	}
	return on_arg;
    }

    if(strcmp(*args,"-v") == 0)
    {
	settings->verbose = TRUE;
	return on_arg;
    }

    if(strcmp(*args,"-a") == 0)
    {
	settings->manage_events = TRUE;
	pal_output_error(_("WARNING: -a is deprecated, use -m instead.\n"));
	return on_arg;
    }

    if(strcmp(*args,"-m") == 0)
    {
	settings->manage_events = TRUE;
	return on_arg;
    }


    if(strcmp(*args,"--color") == 0)
    {
	set_colorize(1);
	return on_arg;
    }

    if(strcmp(*args,"--nocolor") == 0)
    {
	set_colorize(0);
	return on_arg;
    }

    if(strcmp(*args,"--mail") == 0)
    {
	set_colorize(-2); /* overrides a later --color argument */
	settings->mail = TRUE;
	return on_arg;
    }

    if(strcmp(*args,"--html") == 0)
    {
	settings->html_out = TRUE;
	return on_arg;
    }

    if(strcmp(*args, "--latex") == 0)
    {
	settings->latex_out = TRUE;
	return on_arg;
    }

    if(strcmp(*args,"--version") == 0)
    {
	g_print("pal %s\n", PAL_VERSION);
	g_print(_("Compiled with prefix: %s\n"), PREFIX);
	exit(0);
    }

    if(strcmp(*args,"-f") == 0)
    {
	gchar tmp[16384];
	args++; on_arg++;
	if(on_arg > total_args || sscanf(*args, "%s", tmp) != 1)
	{
	    pal_output_error("%s\n", _("ERROR: Pal conf file required after -f argument."));
	    pal_output_error("       %s\n", _("Use --help for more information."));
	    return on_arg;
	}

	g_free(settings->conf_file);
	settings->conf_file = g_strdup(tmp);
	settings->specified_conf_file = TRUE;
	return on_arg;
    }

    if(strcmp(*args,"-p") == 0)
    {
	gchar tmp[16384];
	args++; on_arg++;
	if(on_arg > total_args || sscanf(*args, "%s", tmp) != 1)
	{
	    pal_output_error("%s\n", _("ERROR: *.pal file required after -p argument."));
	    pal_output_error("       %s\n", _("Use --help for more information."));
	    return on_arg;
	}

	settings->pal_file = g_strdup(tmp);
	return on_arg;
    }

    if(strcmp(*args,"-u") == 0)
    {
	gchar username[256];
	args++; on_arg++;
	if(on_arg > total_args || sscanf(*args, "%s", username) != 1)
	{
	    pal_output_error("%s\n", _("ERROR: Username required after -u argument."));
	    pal_output_error("       %s\n", _("Use --help for more information."));
	    return on_arg;
	}

	g_free(settings->conf_file);
	settings->conf_file = g_strconcat("/home/", username, "/.pal/pal.conf", NULL);
	settings->specified_conf_file = TRUE;
	return on_arg;
    }

    if(strcmp(*args,"-s") == 0)
    {
	args++; on_arg++;

	if(on_arg > total_args)
	{
	    pal_output_error("%s\n", _("ERROR: Regular expression required after -s argument."));
	    pal_output_error("       %s\n", _("Use --help for more information."));
	}
	else
	    settings->search_string = g_strdup(*args);

	return on_arg;
    }

    if(strcmp(*args,"-x") == 0)
    {
	args++; on_arg++;
	if(on_arg > total_args || sscanf(*args, "%d", &(settings->expunge)) != 1)
	{
	    pal_output_error("%s\n", _("ERROR: Number required after -x argument."));
	    pal_output_error("       %s\n", _("Use --help for more information."));
	    on_arg--;
	}

	return on_arg;
    }


    pal_output_error("%s %s\n", _("ERROR: Bad argument:"), *args);
    pal_output_error("       %s\n", _("Use --help for more information."));

    return on_arg+1;
}



/* used when the pal is finished running and the hashtable entries
 * need to be properly freed */
static void hash_table_free_item(gpointer key, gpointer value, gpointer user_data)
{
    GList* list = (GList*) value;
    GList* prev = list;

    g_free(key);

    /* free the list for this hashtable key */
    while(g_list_length(list) != 0)
    {
	pal_event_free((PalEvent*) list->data);
	list = g_list_next(list);
    }
    g_list_free(prev);
}


/* free the hashtable */
static void pal_main_ht_free(void)
{
    if(ht != NULL)
    {
	g_hash_table_foreach(ht, (GHFunc) hash_table_free_item, NULL);
	g_hash_table_destroy(ht);
	ht = NULL;
    }
}

/* free, and reload the hashtable and settings from pal.conf and
 * calendar files */
void pal_main_reload(void)
{
    if(settings->verbose)
	g_printerr("Reloading events and settings.\n");

    pal_main_ht_free();
    ht = load_files();
}

int main(gint argc, gchar** argv)
{
    const gchar *charset = NULL;
    gint on_arg = 1;
    GDate* today = g_date_new();

    g_date_set_time_t(today, time(NULL));

    settings = g_malloc(sizeof(Settings));
    settings->cal_lines           = 5;
    settings->range_days          = 0;
    settings->range_neg_days      = 0;
    settings->range_arg           = FALSE;
    settings->search_string       = NULL;
    settings->verbose             = FALSE;
    settings->mail                = FALSE;
    settings->query_date          = NULL;
    settings->expunge             = -1;
    settings->date_fmt            = g_strdup("%a %e %b %Y");
    settings->week_start_monday   = FALSE;
    settings->reverse_order       = FALSE;
    settings->cal_on_bottom       = FALSE;
    settings->specified_conf_file = FALSE;
    settings->no_columns          = FALSE;
    settings->hide_event_type     = FALSE;
    settings->manage_events       = FALSE;
    settings->curses              = FALSE;
    settings->event_color         = BLUE;
    settings->pal_file            = NULL;
    settings->html_out            = FALSE;
    settings->latex_out           = FALSE;
    settings->compact_list        = FALSE;
    settings->term_cols           = 80;
    settings->term_rows           = 24;
    settings->compact_date_fmt    = g_strdup("%m/%d/%Y");
    settings->conf_file = g_strconcat(g_get_home_dir(), "/.pal/pal.conf", NULL);
    settings->show_weeknum        = FALSE;

    g_set_print_handler( pal_output_handler );
    g_set_printerr_handler( pal_output_handler ); 
    
    textdomain("pal");
    bind_textdomain_codeset("pal", "utf-8");
    if(setlocale(LC_MESSAGES, "") == NULL ||
       setlocale(LC_TIME, "") == NULL ||
       setlocale(LC_ALL, "") == NULL ||
       setlocale(LC_CTYPE, "") == NULL)
	pal_output_error("WARNING: Localization failed.\n");

    
#ifndef __CYGWIN__
    /* figure out the terminal width if possible */
    {
	struct winsize wsz;
	if(ioctl(0, TIOCGWINSZ, &wsz) != -1)
	{
	    settings->term_cols = wsz.ws_col;
	    settings->term_rows = wsz.ws_row;
	}
    }
#endif

    /* parse all the arguments */
    while(on_arg < argc)
	on_arg = parse_arg(argv,on_arg,argc);

    g_get_charset(&charset);
    if(settings->verbose)
	g_printerr("Character set: %s\n", charset);


    ht = load_files();




    /* adjust settings if --mail is used */
    if(settings->mail)
    {
	gchar pretty_date[128];
	g_date_strftime(pretty_date, 128, settings->date_fmt, today);

	g_print("From: \"pal\" <pal>\n");
	g_print("Content-Type: text/plain; charset=%s\n", charset);
	g_print("Subject: [pal] %s\n\n", pretty_date);


	/* let the mail reader handle the line wrapping */
	settings->term_cols = -1;
    }

    if(settings->manage_events)
	pal_manage();


    if(!settings->html_out && !settings->latex_out)
    {

	if(!settings->cal_on_bottom)
	{
	    pal_output_cal(settings->cal_lines,today);
	    /* print a newline under calendar if we're printing other stuff */
	    if(settings->cal_lines > 0 && (settings->range_days>0 ||
					   settings->range_neg_days>0 ||
					   settings->query_date != NULL))

		g_print("\n");

	}

	view_details(); /* prints results of -d,-r,-s */

	if(settings->cal_on_bottom)
	{
	/* print a new line over calendar if we've printed other stuff */
	    if(settings->cal_lines > 0 && (settings->range_days>0 ||
					   settings->range_neg_days>0 ||
					   settings->query_date != NULL))
		g_print("\n");

	    pal_output_cal(settings->cal_lines,today);
	}



    } /* end if not html_out and not latex_out */
    else if(settings->html_out && settings->latex_out)
    {
	pal_output_error("ERROR: Can't use both --html and --latex.\n");
	return 1;
    }
    else if(settings->html_out)
	pal_html_out();
    else if(settings->latex_out)
	pal_latex_out();

    g_date_free(today);

    pal_main_ht_free();

    /* free settings */
    g_free(settings->date_fmt);
    g_free(settings->conf_file);
    g_free(settings->search_string);
    if(settings->query_date != NULL)
	g_date_free(settings->query_date);
    g_free(settings->compact_date_fmt);
    g_free(settings->pal_file);

    g_free(settings);

    return 0;
}
