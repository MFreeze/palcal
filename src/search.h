#ifndef PAL_SEARCH_H
#define PAL_SEARCH_H

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

PalEvent* pal_search_event_num(gint event_number, GDate** store_date, const gchar* search_string, const GDate* date, const gint window);
int pal_search_view(const gchar* search_string, GDate* date, const gint window, const gboolean number_events);
gboolean pal_search_isearch_event( GDate **date, gint *selected, gchar *string, gboolean forward);

#endif
