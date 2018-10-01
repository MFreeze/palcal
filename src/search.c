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

#include <string.h>
#include <sys/types.h> /* FreeBSD, regex.h needs this */
#include <regex.h>     /* regular expressions */

#include "main.h"
#include "output.h"
#include "event.h"
#include "search.h"

/* returns a list of the events matching the 'search' string.  'date'
 * is the starting date.  'window' is the number of days from the
 * starting date to search.
 *
 * The returned list alternates between GDate and PalEvent pointers.
 * The GDate pointer allows the caller to determine the date the event
 * was found on (because the event could be recurring and happen
 * multiple times).  The GDate pointers in the list should be freed.
 * The PalEvent pointers should not be freed.
*/

static GList* pal_search_get_results(const gchar* search, const GDate* date, const gint window)
{
    regex_t preg;
    gint i,j;
    GList* hit_list = NULL;
    GDate *searchdate = g_date_new();
    
    memcpy( searchdate, date, sizeof( GDate ) );

    if(settings->reverse_order)
	g_date_add_days(searchdate, window-1);

    regcomp(&preg, search, REG_ICASE|REG_NOSUB);

    for(i=0; i<window; i++)
    {
	GList* events = get_events(searchdate);

	if(events != NULL)
	{
	    GList* item = g_list_first(events);
	    for(j=0; j<g_list_length(events); j++)
	    {

		if(regexec(&preg, ((PalEvent*) (item->data))->text, 0, NULL, 0)==0 ||
		   regexec(&preg, ((PalEvent*) (item->data))->type, 0, NULL, 0)==0)
		{
		    GDate* tmp = g_malloc(sizeof(GDate));
		    memcpy(tmp, searchdate, sizeof(GDate));
		    hit_list = g_list_append(hit_list, tmp);
		    hit_list = g_list_append(hit_list, item->data);
		}

		item = g_list_next(item);
	    }
	}

	if(settings->reverse_order)
	    g_date_subtract_days(searchdate, 1);
	else
	    g_date_add_days(searchdate, 1);
    }

    regfree(&preg);
    return hit_list;
}


/* returns the number of events found */
int pal_search_view(const gchar* search_string, GDate* date, const gint window, const gboolean number_events)
{
    GList* hit_list = pal_search_get_results(search_string, date, window);
    GList* item = NULL;
    int hit_count = g_list_length(hit_list) / 2;
    int event_count = 1;
    gchar start_date[128];
    gchar end_date[128];

    g_date_strftime(start_date, 128, settings->date_fmt, date);
    g_date_add_days(date, window-1);
    g_date_strftime(end_date, 128, settings->date_fmt, date);
    g_date_subtract_days(date, window-1);

    pal_output_attr(BRIGHT, _("[ Begin search results: %s ]\n[ From %s to %s inclusive ]\n\n"),
		    search_string, start_date, end_date);

    item = g_list_first(hit_list);

    while(g_list_length(item) != 0)
    {
	PalEvent* event_tmp = NULL;
	GDate* date_tmp = NULL;
	GDate* next_date = NULL;

	date_tmp  = (GDate*)    (item->data);
	item = g_list_next(item);
	event_tmp = (PalEvent*) (item->data);
	item = g_list_next(item);

	if(!settings->compact_list)
	    pal_output_date_line(date_tmp);

	if(number_events)
	    pal_output_event(event_tmp, date_tmp, event_count++);
	else
	    pal_output_event(event_tmp, date_tmp, -1);

	if(g_list_length(item) != 0)
	    next_date = (GDate*) item->data;

	while(g_list_length(item) != 0 && g_date_compare(next_date, date_tmp) == 0)
	{
	    g_date_free(date_tmp);

	    date_tmp  = (GDate*)    (item->data);
	    item = g_list_next(item);
	    event_tmp = (PalEvent*) (item->data);
	    item = g_list_next(item);

	    if(number_events)
		pal_output_event(event_tmp, date_tmp, event_count++);
	    else
		pal_output_event(event_tmp, date_tmp, -1);

	    if(g_list_length(item) != 0)
		next_date = (GDate*) item->data;
	}

	g_date_free(date_tmp);

	if(!settings->compact_list)
	    g_print("\n");
    }

    g_list_free(hit_list);

    /* no extra newlines when using compact list, so add one here */
    if(settings->compact_list)
	g_print("\n");

    pal_output_attr(BRIGHT, _("[ End search results: %s ]"), search_string);
    pal_output_attr(BRIGHT, ngettext("[ %d event found ]\n", "[ %d events found ]\n",
				     hit_count), hit_count);
    
    return hit_count;
}



/* Returns the event 'event_number' from the search.  Stores the date
 * the event occurs on in store_date */
PalEvent* pal_search_event_num(gint event_number, GDate** store_date, const gchar* search_string,
			       const GDate* date, const gint window)
{
    PalEvent* ret_val = NULL;
    GList* hit_list = pal_search_get_results(search_string, date, window);
    gint num_events = g_list_length(hit_list) / 2;
    GList* tmp = NULL;

    if(hit_list == NULL || event_number < 1 || event_number > num_events)
	return NULL;

    *store_date = (GDate*) g_list_nth_data(hit_list, (event_number-1)*2);
    ret_val = (PalEvent*) g_list_nth_data(hit_list, (event_number-1)*2+1);

    tmp = g_list_first(hit_list);
    while(tmp != NULL)
    {
	if(*store_date != (GDate*) (tmp->data))
	    g_date_free((GDate*) (tmp->data));
	tmp = g_list_next(tmp);
	tmp = g_list_next(tmp);
    }

    g_list_free(hit_list);

    return ret_val;

}



/* A simpler search, just searches for the first event which contains this
 * string. Used by the interactive search in the manage interface. Attempts
 * a semblance of case-insensetivity */
gboolean pal_search_isearch_event( GDate **date, gint *selected, gchar *string, gboolean forward)
{
    int i, j;
    gboolean found = FALSE;
    gchar *searchstring = g_utf8_casefold(string,-1);

    /* Search upto a year */
    for(i=0; i<366; i++)
    {
	GList* events = get_events(*date);

	if(events != NULL)
	{
	    GList* item = g_list_first(events);
	    for(j=0; j<g_list_length(events) && !found; j++)
	    {
	        gchar *string = g_strconcat( ((PalEvent*) (item->data))->type, ": ", ((PalEvent*) (item->data))->text, NULL );
	        gchar *string2 = g_utf8_casefold(string,-1);
	        
	        if( strstr( string2, searchstring ) )
                {
                    *selected = j;
                    found = TRUE;
		}

                g_free(string);
                g_free(string2);
		item = g_list_next(item);
	    }
	    g_list_free( events );
	}
	
	if( found )
	    break;

	if(forward)
	    g_date_add_days(*date, 1);
	else
	    g_date_subtract_days(*date, 1);
    }
    g_free(searchstring);
    return found;
}
