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
#include <ncurses.h>

#include "main.h"
#include "output.h"
#include "event.h"
#include "rl.h"
#include "input.h"
#include "edit.h"

void pal_del_write_file(PalEvent* dead_event)
{
    FILE *file = NULL;
    gchar* filename = g_strdup(dead_event->file_name);
    FILE *out_file = NULL;
    gchar *out_filename = NULL;
    PalEvent* event_head = NULL;

    g_strstrip(filename);
    out_filename = g_strconcat(filename, ".paltmp", NULL);


    file = fopen(filename, "r");
    if(file == NULL)
    {
	pal_output_error(_("ERROR: Can't read file: %s\n"), filename);
	pal_output_error(_("       The event was NOT deleted."));
	return;
    }

    out_file = fopen(out_filename, "w");
    if(out_file == NULL)
    {
	pal_output_error(_("ERROR: Can't write file: %s\n"), out_filename);
	pal_output_error(_("       The event was NOT deleted."));
	if(file != NULL)
	    fclose(file);
	return;
    }



    pal_input_skip_comments(file, out_file);
    event_head = pal_input_read_head(file, out_file, filename);

    while(1)
    {
	PalEvent* pal_event = NULL;

	pal_input_skip_comments(file, out_file);

	pal_event = pal_input_read_event(file, out_file, filename, event_head, dead_event);

	/* stop trying to delete dead_event if we just deleted it */
	if(dead_event != NULL && pal_event == dead_event)
	    dead_event = NULL;
	else if(pal_event == NULL && pal_input_eof(file))
	    break;
    }


    fclose(file);
    fclose(out_file);

    if(rename(out_filename, filename) != 0)
    {
	pal_output_error(_("ERROR: Can't rename %s to %s\n"), out_filename, filename);
	pal_output_error(_("       The event was NOT deleted."));
	return;
    }


    if(dead_event == NULL)
    {
	pal_output_fg(BRIGHT, GREEN, ">>> ");
	g_print(_("Event removed from %s.\n"), filename);
    }
    else
	pal_output_error(_("ERROR: Couldn't find event to be deleted in %s"), filename);

    g_free(filename);
}

static void pal_del_event( GDate *date, int eventnum )
{
    PalEvent* dead_event = NULL;
    GDate* event_date = NULL;

    clear();
    pal_output_fg(BRIGHT, GREEN, "* * * ");
    pal_output_attr(BRIGHT, _("Delete an event"));
    pal_output_fg(BRIGHT, GREEN, " * * *\n");

    pal_output_fg(BRIGHT, YELLOW, "> ");
    pal_output_wrap(_("If you want to delete old events that won't occur again, you can use pal's -x option instead of deleting the events manually."),2,2);

    dead_event = pal_rl_get_event(&event_date, FALSE);

    g_print("\n");
    pal_output_fg(BRIGHT, GREEN, "> ");
    g_print(_("You have selected to delete the following event:\n"));
    pal_output_event(dead_event, event_date, -1);

    if(pal_rl_get_y_n(_("Are you sure you want to delete this event? [y/n]: ")))
       pal_del_write_file(dead_event);

    pal_main_reload();

}
