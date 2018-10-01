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
#include <time.h>

#include "main.h"
#include "event.h"


static gint get_nth_day(const GDate* date);
static gboolean last_weekday_of_month(const GDate* date);


/* Currently in add.c, should be moved at some stage */
void pal_add_suffix(gint number, gchar* suffix, gint buf_size);

PalEvent* pal_event_init()
{
    PalEvent* event = g_malloc(sizeof(PalEvent));
    event->text = NULL;
    event->type = NULL;
    event->start_date = NULL;
    event->end_date = NULL;
    event->date_string = NULL;
    event->start_time = NULL;
    event->end_time = NULL;
    event->file_name = NULL;
    event->key = NULL;
    event->eventtype = NULL;
    event->period_count = 1;
    return event;
}

PalEvent* pal_event_copy(PalEvent* orig)
{
    PalEvent* new = g_malloc(sizeof(PalEvent));
    new->text = g_strdup(orig->text);
    new->start = orig->start;
    new->end   = orig->end;
    new->hide  = orig->hide;
    new->type  = g_strdup(orig->type);
    new->file_num = orig->file_num;
    new->file_name = g_strdup(orig->file_name);
    new->color = orig->color;

    if(orig->start_date == NULL)
	new->start_date = NULL;
    else
    {
	new->start_date = g_malloc(sizeof(GDate));
	memcpy(new->start_date, orig->start_date, sizeof(GDate));
    }

    if(orig->end_date == NULL)
	new->end_date = NULL;
    else
    {
	new->end_date = g_malloc(sizeof(GDate));
	memcpy(new->end_date, orig->end_date, sizeof(GDate));
    }

    if(orig->start_time == NULL)
	new->start_time = NULL;
    else
    {
	new->start_time = g_malloc(sizeof(PalTime));
	memcpy(new->start_time, orig->start_time, sizeof(PalTime));
    }

    if(orig->end_time == NULL)
	new->end_time = NULL;
    else
    {
	new->end_time = g_malloc(sizeof(PalTime));
	memcpy(new->end_time, orig->end_time, sizeof(PalTime));
    }


    new->date_string = g_strdup(orig->date_string);
    new->key = g_strdup(orig->key);
    new->eventtype = orig->eventtype;
    new->period_count = orig->period_count;
    new->global = orig->global;
    return new;
}

void pal_event_free(PalEvent* event)
{
    if(event == NULL)
	return;

    if(event->text != NULL)
	g_free(event->text);

    if(event->type != NULL)
	g_free(event->type);

    if(event->start_date != NULL)
	g_date_free(event->start_date);

    if(event->end_date != NULL)
	g_date_free(event->end_date);

    if(event->date_string != NULL)
	g_free(event->date_string);

    if(event->start_time != NULL)
	g_free(event->start_time);

    if(event->end_time != NULL)
	g_free(event->end_time);

    if(event->file_name != NULL)
	g_free(event->file_name);

    if(event->key != NULL)
	g_free(event->key);

    g_free(event);

    event = NULL;
    return;
}


static gboolean is_valid_todo(const gchar* date_string)
{
    if( strcmp(date_string, "TODO") == 0 )
        return TRUE;
    return FALSE;
}

static gboolean get_key_todo(const GDate* date, gchar *buffer)
{
    GDate* today = g_date_new();
    g_date_set_time_t(today, time(NULL));
    if(g_date_days_between(today, date) != 0)
    {
	g_date_free(today);
        return FALSE;
    }

    g_date_free(today);
    strcpy( buffer, "TODO" );
    return TRUE;
}

static gchar *get_descr_todo(const GDate *date)
{
    (void)date;	/* Avoid unused warning */
    return g_strdup("TODO event");
}

static gboolean is_valid_daily(const gchar* date_string)
{
    if( strcmp(date_string, "DAILY") == 0 )
        return TRUE;
    return FALSE;
}

static gboolean get_key_daily(const GDate* date, gchar *buffer)
{
    (void)date;	/* Avoid unused warning */
    strcpy( buffer, "DAILY" );
    return TRUE;
}

static gchar *get_descr_daily(const GDate *date)
{
    (void)date;	/* Avoid unused warning */
    return g_strdup("Daily");
}

static gboolean is_valid_yyyymmdd(const gchar* date_string)
{
    gint d[8];

    /* if key is regular event in the form:
       single day, monthly, yearly, yearly/monthly */
    if(sscanf(date_string, "%1d%1d%1d%1d%1d%1d%1d%1d",
	      &d[0],&d[1],&d[2],&d[3],&d[4],&d[5],&d[6],&d[7]) == 8)
    {
	gint year, month, day;

	year  = d[0] * 1000 + d[1] * 100 + d[2] * 10 + d[3];
	month = d[4] * 10 + d[5];
	day   = d[6] * 10 + d[7];

	if(day < 1 || day > 31 || month < 1 || month > 12 || year < 1)
	    return FALSE;

	/* FIXME: Don't allow one-time events on leap day in years
	 * that leap day doesn't exist */
	if(month == 2 && day > 29)
	    return FALSE;

	/* 30 days in april, june, sept, nov */
	if((month == 4 || month == 6 || month == 9 || month == 11) &&
	   day > 30)
	    return FALSE;

	/* make sure there weren't more characters that sscanf didn't see */
	if(strlen(date_string) == 8)
	    return TRUE;
    }

    return FALSE;
}

static gboolean get_key_yyyymmdd(const GDate* date, gchar *buffer)
{
    snprintf(buffer, 9, "%04d%02d%02d", g_date_get_year(date),
	    g_date_get_month(date), g_date_get_day(date));
    return TRUE;
}

static gchar *get_descr_yyyymmdd(const GDate* date)
{
    char buf[128];
    char *ptr;
    ptr = strcpy( buf, "Only on " );
    g_date_strftime(ptr, 100, settings->date_fmt, date);
    return g_strdup(buf);
}

static const gchar *day_names[8] = {
  /*[0] =*/ "",
  /*[1] =*/ "MON",
  /*[2] =*/ "TUE",
  /*[3] =*/ "WED",
  /*[4] =*/ "THU",
  /*[5] =*/ "FRI",
  /*[6] =*/ "SAT",
  /*[7] =*/ "SUN"
};

static gboolean is_valid_weekly(const gchar* date_string)
{
    int i;
    for( i=1; i<=7; i++ )
        if( strcmp( date_string, day_names[i] ) == 0 )
            return TRUE;
    return FALSE;
}

static gboolean get_key_weekly(const GDate* date, gchar* buffer)
{
    strcpy( buffer, day_names[ g_date_get_weekday(date) ] );
    return TRUE;
}

static gchar *get_descr_weekly(const GDate* date)
{
    char buf[128];
    g_date_strftime(buf, 128, "Weekly: Every %A", date);
    return g_strdup(buf);
}

static gboolean is_valid_000000dd(const gchar* date_string)
{
    gint d[8];

    if(sscanf(date_string, "000000%1d%1d", &d[6],&d[7]) == 2)
    {
	int day   = d[6] * 10 + d[7];

	if(day < 1 || day > 31)
	    return FALSE;

	if(strlen(date_string) == 8)
	    return TRUE;
    }
    return FALSE;
}

static gboolean get_key_000000dd(const GDate* date, gchar* buffer)
{
    snprintf( buffer, MAX_KEYLEN, "000000%02d",  g_date_get_day(date) );
    return TRUE;
}

static gchar *get_descr_000000dd(const GDate* date)
{
    char buf[128];
    snprintf( buf, 128, "Monthly: Day %d of every month", g_date_get_day(date) );
    return g_strdup(buf);
}

static gboolean is_valid_0000mmdd(const gchar* date_string)
{
    gint d[8];

    if(sscanf(date_string, "0000%1d%1d%1d%1d", &d[4],&d[5],&d[6],&d[7]) == 4)
    {
	gint month, day;

	day   = d[6] * 10 + d[7];
	month = d[4] * 10 + d[5];

	if(day < 1 || day > 31 || month < 1 || month > 12)
	    return FALSE;

	/* FIXME: Don't allow one-time events on leap day in years
	 * that leap day doesn't exist */
	if(month == 2 && day > 29)
	    return FALSE;

	/* 30 days in april, june, sept, nov */
	if((month == 4 || month == 6 || month == 9 || month == 11) &&
	   day > 30)
	    return FALSE;

	if(strlen(date_string) == 8)
	    return TRUE;

    }
    return FALSE;
}


static gboolean get_key_0000mmdd(const GDate* date, gchar* buffer)
{
    snprintf( buffer, MAX_KEYLEN, "0000%02d%02d",  g_date_get_month(date), g_date_get_day(date) );
    return TRUE;
}

static gchar *get_descr_0000mmdd(const GDate* date)
{
    char buf1[128];
    char buf2[128];
    g_date_strftime(buf1, 128, "%B", date);
    snprintf( buf2, 128, "Annually: %d %s", g_date_get_day(date), buf1 );
    return g_strdup(buf2);
}



static gboolean is_valid_star_00nd(const gchar* date_string)
{
    gint d[2];
    if(sscanf(date_string, "*00%1d%1d", &d[0],&d[1]) == 2) /* nth weekday of month  */
    {
	gint n, weekday;

	n = d[0];
	weekday = d[1];

	if(weekday >  0 && weekday < 8  &&
	         n >  0 &&       n < 6)	   /* no more than 5 weeks in a month */
	{
	    if(strlen(date_string) == 5)
		return TRUE;
	}
    }

    return FALSE;
}

static gboolean get_key_star_00nd(const GDate* date, gchar* buffer)
{
    /* convert weekday to friendly weekday
       from: 1(mon) -> 7(sun)
       to:   1(sun) -> 7(sat) */
    int weekday = (g_date_get_weekday(date) % 7) + 1;
    snprintf( buffer, MAX_KEYLEN, "*00%d%d",  get_nth_day(date), weekday );
    return TRUE;
}

static gchar *get_descr_star_00nd(const GDate* date)
{
    char suffix[16];
    char buf1[128];
    char buf2[128];
    pal_add_suffix(get_nth_day(date), suffix, 16);
    g_date_strftime(buf1, 128, "%A", date);
    snprintf(buf2, 128, "Monthly: The %s %s of every month",
                        suffix, buf1);
    return g_strdup(buf2);
}

static gboolean is_valid_star_mmnd(const gchar* date_string)
{
    gint d[8];
    if(sscanf(date_string, "*%1d%1d%1d%1d", &d[0],&d[1],&d[2],&d[3]) == 4) /* nth weekday of month  */
    {
	gint month, n, weekday;

	month = d[0] * 10 + d[1];
	n = d[2];
	weekday = d[3];

	if(weekday >  0 && weekday < 8  &&
	     month >  0 &&   month < 13 &&
	         n >  0 &&       n < 6)	   /* no more than 5 weeks in a month */
	{
	    if(strlen(date_string) == 5)
		return TRUE;
	}
    }

    return FALSE;
}

static gboolean get_key_star_mmnd(const GDate* date, gchar* buffer)
{
    /* convert weekday to friendly weekday
       from: 1(mon) -> 7(sun)
       to:   1(sun) -> 7(sat) */
    int weekday = (g_date_get_weekday(date) % 7) + 1;
    snprintf( buffer, MAX_KEYLEN, "*%02d%d%d", g_date_get_month(date), get_nth_day(date), weekday );
    return TRUE;
}

static gchar *get_descr_star_mmnd(const GDate* date)
{
    char suffix[16];
    char buf1[128];
    char buf2[128];
    char buf3[128];
    pal_add_suffix(get_nth_day(date), suffix, 16);
    g_date_strftime(buf1, 128, "%A", date);
    g_date_strftime(buf2, 128, "%B", date);
    snprintf(buf3, 128, "Annually: The %s %s of every %s",
                        suffix, buf1, buf2);
    return g_strdup(buf3);
}

static gboolean is_valid_star_00Ld(const gchar* date_string)
{
    gint d[2];
    if(sscanf(date_string, "*00L%1d", &d[0]) == 1) /* last weekday of month */
    {
	gint weekday;

	weekday = d[0];
	if(weekday >  0 && weekday < 8)
	{
	    if(strlen(date_string) == 5)
		return TRUE;
	}

    }

    return FALSE;
}

static gboolean get_key_star_00Ld(const GDate* date, gchar* buffer)
{
    int weekday;

    if( !last_weekday_of_month(date) )
        return FALSE;

    /* convert weekday to friendly weekday
       from: 1(mon) -> 7(sun)
       to:   1(sun) -> 7(sat) */
    weekday = (g_date_get_weekday(date) % 7) + 1;

    snprintf( buffer, MAX_KEYLEN, "*00L%d",  weekday );
    return TRUE;
}

static gchar *get_descr_star_00Ld(const GDate* date)
{
    char buf1[128];
    char buf2[128];
    if( !last_weekday_of_month(date) )
        return NULL;

    g_date_strftime(buf1, 128, "%A", date);
    snprintf(buf2, 128, "Monthly: The last %s of every month",
                        buf1);
    return g_strdup(buf2);
}


static gboolean is_valid_star_mmLd(const gchar* date_string)
{
    gint d[8];
    if(sscanf(date_string, "*%1d%1dL%1d", &d[0],&d[1],&d[2]) == 3) /* last weekday of month */
    {
	gint month, weekday;

	month = d[0] * 10 + d[1];
	weekday = d[2];

	if(weekday >  0 && weekday < 8 &&
	   month   >  0 &&   month < 13 )
	{
	    if(strlen(date_string) == 5)
		return TRUE;
	}

    }

    return FALSE;
}

static gboolean get_key_star_mmLd(const GDate* date, gchar* buffer)
{
    int weekday;

    if( !last_weekday_of_month(date) )
        return FALSE;

    /* convert weekday to friendly weekday
       from: 1(mon) -> 7(sun)
       to:   1(sun) -> 7(sat) */
    weekday = (g_date_get_weekday(date) % 7) + 1;
    snprintf( buffer, MAX_KEYLEN, "*%02dL%d", g_date_get_month(date), weekday );
    return TRUE;
}

static gchar *get_descr_star_mmLd(const GDate* date)
{
    char buf1[128];
    char buf2[128];
    char buf3[128];
    if( !last_weekday_of_month(date) )
        return NULL;

    g_date_strftime(buf1, 128, "%A", date);
    g_date_strftime(buf2, 128, "%B", date);
    snprintf(buf3, 128, "Annually: The last %s of every %s",
                        buf1, buf2);
    return g_strdup(buf3);
}


static gboolean is_valid_EASTER(const gchar* date_string)
{

    if(strncmp(date_string, "EASTER", 6) == 0)
    {

	if(strlen(date_string) == 6)
	    return TRUE;

	if(date_string[6] == '-' || date_string[6] == '+')
	{
	    if(g_ascii_isdigit(date_string[7]) &&
	       g_ascii_isdigit(date_string[8]) &&
	       g_ascii_isdigit(date_string[9]))
	    {
		if(date_string[10] == '\0')
		    return TRUE;
	    }
	}

    }

    return FALSE;
}




/* checks if date_string is a valid date string.  Before calling this
 * function, g_strstrip needs to be called on date_string!  g_ascii_strup
 * should also be called on the date_string. */
gboolean parse_event( PalEvent *event, const gchar* date_string)
{
    gchar** s;
    gchar* ptr;

    int count = 1;
    int i;
    PalPeriodic period = PAL_ONCEONLY;

    s = g_strsplit(date_string, ":", 3);

    do {    /* Loop to break out of for failure so we can clean up */
        if( s[1] && !is_valid_yyyymmdd(s[1]) )  /* start date */
            break;

        /* Need to check both, if s[1] is NULL, s[2] isn't valid */
        if( s[1] && s[2] && !is_valid_yyyymmdd(s[2]) )  /* end date */
            break;

        if( (ptr = strrchr( s[0], '/' )) != NULL )  /* Repeat counter */
        {
            if( sscanf( ptr+1, "%d%*s", &count ) != 1 || count < 1 )  /* Invalid count */
                break;

            *ptr = '\0';
        }

        for( i=0; i < PAL_NUM_EVENTTYPES; i++ )
        {
            if( PalEventTypes[i].valid_string( s[0] ) )
            {
                period = PalEventTypes[i].period;
                break;
            }
        }

        if( i == PAL_NUM_EVENTTYPES )
            break;  /* Fail */

        /* We now know the string is valid */

        if(s[1])
        {
            event->start_date = get_date(s[1]);

            if( s[2] )
                event->end_date = get_date(s[2]);
            else
                event->end_date = g_date_new_dmy(1,1,3000);
        }
        event->period_count = count;
        event->eventtype = &PalEventTypes[i];
        event->key = g_strdup( s[0] );
        g_strfreev(s);
        return TRUE;
    } while(0);

    g_strfreev(s);
    return FALSE;

}



static GDate* find_easter(gint year)
{
    gint a,b,c,d,e,f,g,h,i,k,l,m,p,month,day;

    a = year%19;
    b = year/100;
    c = year%100;
    d = b/4;
    e = b%4;
    f = (b+8) / 25;
    g = (b-f+1) / 3;
    h = (19*a+b-d-g+15) % 30;
    i = c/4;
    k = c%4;
    l = (32+2*e+2*i-h-k)%7;
    m = (a+11*h+22*l) / 451;
    month = (h+l-7*m+114) / 31;
    p = (h+l-7*m+114) % 31;
    day = p+1;

    return g_date_new_dmy((GDateDay) day, (GDateMonth) month,
			  (GDateYear) year);
}

static gboolean get_key_EASTER(const GDate* date, gchar *buffer)
{
    GDate* easter = find_easter(g_date_get_year(date));
    gint diff = g_date_days_between(date,easter);
    g_date_free(easter);

    if(diff != 0)
	snprintf(buffer, 12, "EASTER%c%03d", (diff > 0) ? '-' : '+', (diff > 0) ? diff : -diff);
    else
	snprintf(buffer, 12, "EASTER");

    return TRUE;
}

static gchar *get_descr_EASTER(const GDate* date)
{
    char buf[128];
    GDate* easter = find_easter(g_date_get_year(date));
    gint diff = g_date_days_between(date,easter);
    g_date_free(easter);

    if(diff != 0)
	snprintf(buf, 128, "%d days %s Easter", (diff > 0) ? diff : -diff, (diff > 0) ? "before" : "after");
    else
	snprintf(buf, 128, "Easter");

    return g_strdup(buf);
}


/* gets the yyyymmdd key for the given date.  The returned string
 * should be freed. */
gchar* get_key(const GDate* date)
{
    gchar* key = g_malloc(sizeof(gchar)*9);

    snprintf(key, 9, "%04d%02d%02d", g_date_get_year(date),
	    g_date_get_month(date), g_date_get_day(date));
    return key;
}


/* this only works on dates in yyyymmdd format.  It ignores everything
 * after the 8th character.  The returned GDate object should be
 * freed.  Returns NULL on failure*/
GDate* get_date(const gchar* key)
{
    GDate* date = NULL;
    gint year, month, day;

    sscanf(key, "%04d%02d%02d", &year, &month, &day);

    if(g_date_valid_dmy((GDateDay) day, (GDateMonth) month,
			(GDateYear) year))
	date = g_date_new_dmy((GDateDay) day, (GDateMonth) month,
			      (GDateYear) year);

    return date;

}


static gboolean last_weekday_of_month(const GDate* date)
{
    GDate* local = g_memdup(date,sizeof(GDate));

    g_date_add_days(local,7);
    if(g_date_get_month(local) != g_date_get_month(date))
    {
	g_date_free(local);
	return TRUE;
    }
    g_date_free(local);

    return FALSE;
}


/* Returns n from date --- as in: "date" is the "n"th
 * sunday/monday/... of the month */
static gint get_nth_day(const GDate* date)
{
    int i = (g_date_get_day(date) / 7) + 1;
    if(g_date_get_day(date) % 7 == 0)
	i--;
    return i;
}


/* removes events from the list whose range that are does not include
 * "date".  Also remove recurring events that should be skipped for
 * the given date.
 *
 * Returns the beginning of the new list (it might have changed) */
static GList* inspect_range(GList* list, const GDate* date)
{
    GList* item = list;

    if(list == NULL)
	return list;

    while(g_list_length(item) > 0)
    {
	gboolean remove = FALSE;
	PalEvent *event = (PalEvent*) item->data;
	if(event->start_date != NULL && event->end_date != NULL)
	{

	    if(g_date_days_between(date, event->start_date) > 0 ||
	       g_date_days_between(date, event->end_date) < 0)
		remove = TRUE;
	    else if( event->period_count != 1 )
	    {
                int event_count = 0; /* Number of times event has happened since start */

                switch( event->eventtype->period )
                {
                  case PAL_ONCEONLY:
                      event_count = 1;
                      break;
                  case PAL_DAILY:
                      event_count = g_date_days_between(event->start_date, date);
                      break;
                  case PAL_WEEKLY:
                      event_count = g_date_days_between(event->start_date, date) / 7;
                      break;
                  case PAL_MONTHLY:
                  {
                      int month_start = g_date_get_month(event->start_date) + 12*g_date_get_year(event->start_date);
                      int month_cur   = g_date_get_month(date)              + 12*g_date_get_year(date);
                      event_count     = month_cur - month_start;
                      break;
                  }
                  case PAL_YEARLY:
                  {
                      event_count     = g_date_get_year(date) - g_date_get_year(event->start_date);
                      break;
                  }
               }
               if( (event_count % event->period_count) != 0 )
                   remove = TRUE;
	    }

	    if( remove )
	    {
		/* if not on last item */
		if(g_list_length(item) > 1)
		{
		    /* save reference to next data item */
		    gpointer n = g_list_next(item)->data;

		    /* remove this list element */
		    list = g_list_remove(list, item->data);

		    /* find what we need to look at next */
		    item = g_list_find(list, n);
		}
		else /* we're on last item */
		{
		    list = g_list_remove(list, item->data);
		    item = g_list_last(list);
		}
	    }
	    else
		item = g_list_next(item);
	}
	else
	    item = g_list_next(item);

    }

    return list;
}


static gint pal_event_sort_fn(gconstpointer x, gconstpointer y)
{
    PalEvent* a = (PalEvent*) x;
    PalEvent* b = (PalEvent*) y;

    /* Put events with start times before events without start times */
    if(a->start_time != NULL && b->start_time == NULL)
	return 1;
    if(a->start_time == NULL && b->start_time != NULL)
	return -1;

    /* if both events have start times, sort by start time */
    if(a->start_time != NULL && b->start_time != NULL)
    {
	if(a->start_time->hour < b->start_time->hour)
	    return -1;
	if(a->start_time->hour > b->start_time->hour)
	    return 1;

	/* if we get here, the hours are the same */
	if(a->start_time->min < b->start_time->min)
	    return -1;
	if(a->start_time->min > b->start_time->min)
	    return 1;
	return 0;
    }

    /* if neither event has start times, sort by order in pal.conf */
    if(a->file_num == b->file_num)
	return 0;
    if(a->file_num <  b->file_num)
	return -1;
    else
	return 1;
}

static GList* pal_event_sort_events(GList* events)
{
    if(events == NULL)
	return NULL;

    return g_list_sort(events, pal_event_sort_fn);

}


/* Returns a list of events on the given date.
   The returned list is sorted. */
GList* get_events(const GDate* date)
{
    GList* list = NULL;
    GList* days_events = NULL;
    gchar eventkey[MAX_KEYLEN];
    int i;

    for( i=0; i < PAL_NUM_EVENTTYPES; i++ )
    {
        if( PalEventTypes[i].get_key( date, eventkey ) == FALSE )
            continue;

        days_events = g_hash_table_lookup(ht,eventkey);
	
        if(days_events != NULL)
            list = g_list_concat(list, g_list_copy(days_events));
    }

    list = inspect_range(list, date);
    list = pal_event_sort_events(list);

    return list;
}

/* Some places only need to know the number of events on a day. They should
 * use this instead to avoid leaking memory when doing g_list_length(get_events) */
gint pal_get_event_count( GDate *date )
{
    GList *events = get_events( date );
    gint count = g_list_length(events);
    g_list_free(events);
    return count;
}

/* the returned string should be freed */
gchar* pal_event_escape(const PalEvent* event, const GDate* today)
{
    gchar* in = event->text;
    gchar* out_string = g_malloc(sizeof(gchar)*strlen(event->text)*2);
    gchar* out = out_string;

    while(*in != '\0')
    {
	if( *in == '!' && strlen(in) > 5 &&
	    g_ascii_isdigit(*(in+1)) &&
	    g_ascii_isdigit(*(in+2)) &&
	    g_ascii_isdigit(*(in+3)) &&
	    g_ascii_isdigit(*(in+4)) &&
	    *(in+5) == '!')
	{
	    int diff;
	    int now = g_date_get_year(today);
	    int event = g_ascii_digit_value(*(in+1));
	    event *= 10;
	    event += g_ascii_digit_value(*(in+2));
	    event *= 10;
	    event += g_ascii_digit_value(*(in+3));
	    event *= 10;
	    event += g_ascii_digit_value(*(in+4));
	    diff = now-event;
	    out += sprintf(out, "%i", diff);
	    in += 6;
	}
	else
	{
	    *out = *in;
	    out++;
	    in++;
	}
    }

    *out = '\0';
    return out_string;
}




PalEventType PalEventTypes[] = {
    /* todo */
    { PAL_ONCEONLY,  is_valid_todo,       get_key_todo,       get_descr_todo      },
    /* single day event */
    { PAL_ONCEONLY,  is_valid_yyyymmdd,   get_key_yyyymmdd,   get_descr_yyyymmdd  },
    /* daily event */
    { PAL_DAILY,     is_valid_daily,      get_key_daily,      get_descr_daily     },
    /* weekly event */
    { PAL_WEEKLY,    is_valid_weekly,     get_key_weekly,     get_descr_weekly    },
    /* monthly event */
    { PAL_MONTHLY,   is_valid_000000dd,   get_key_000000dd,   get_descr_000000dd  },
    /* monthly event Nth something-day */
    { PAL_MONTHLY,   is_valid_star_00nd,  get_key_star_00nd,  get_descr_star_00nd },
    /* yearly event */
    { PAL_YEARLY,    is_valid_0000mmdd,   get_key_0000mmdd,   get_descr_0000mmdd  },
    /* yearly event on Nth something-day of a certain month */
    { PAL_YEARLY,    is_valid_star_mmnd,  get_key_star_mmnd,  get_descr_star_mmnd },
    /* monthly event on the last something-day */
    { PAL_MONTHLY,   is_valid_star_00Ld,  get_key_star_00Ld,  get_descr_star_00Ld },
    /* yearly event on the last something-day */
    { PAL_YEARLY,    is_valid_star_mmLd,  get_key_star_mmLd,  get_descr_star_mmLd },
    /* easter */
    { PAL_YEARLY,    is_valid_EASTER,     get_key_EASTER,     get_descr_EASTER    }
};

const gint PAL_NUM_EVENTTYPES = sizeof(PalEventTypes) / sizeof(PalEventTypes[0]);


