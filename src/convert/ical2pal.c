/* ical2pal
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
#include <errno.h>
#include <string.h>
#include "ical.h"

#include <stdlib.h>

char* read_stream(char *s, size_t size, void *d)
{
    char *c = fgets(s,size, (FILE*)d);
    return c;
}


void output_pal_from_ical(icalcomponent* c)
{
    int good_event = 0;
    int bad_event = 0;
    int error_flag = 0;

    icalcomponent* vevent =  icalcomponent_get_first_component(c, ICAL_VEVENT_COMPONENT);

    while(vevent != 0)
    {
	icalproperty* rrule;
	struct icalrecurrencetype recur;

	icalproperty* summary = icalcomponent_get_first_property(vevent, ICAL_SUMMARY_PROPERTY);

	struct icaltimetype start_value = icalcomponent_get_dtstart(vevent);
	struct icaltimetype end_value = icalcomponent_get_dtend(vevent);

	struct icaldurationtype duration = icaltime_subtract(end_value, start_value);

	char startstop[19];
	char time[15];

	const char* summary_text = NULL;


	startstop[0] = '\0';
	time[0] = '\0';


	if(duration.days > 1 || duration.weeks > 1)
	    snprintf(startstop, 19, ":%04i%02i%02i:%04i%02i%02i",
		     start_value.year, start_value.month, start_value.day,
		       end_value.year,   end_value.month,   end_value.day);



	if(duration.minutes > 0 || duration.hours > 0)
	    snprintf(time, 15, "(%02i:%02i-%02i:%02i)",
		     start_value.hour, start_value.minute,
		     end_value.hour, end_value.minute);


	rrule = icalcomponent_get_first_property(vevent,ICAL_RRULE_PROPERTY);

	if(rrule != 0)
	    recur = icalproperty_get_rrule(rrule);
	else
	    recur.freq = ICAL_NO_RECURRENCE;


	if(summary != NULL)
	    summary_text = icalproperty_get_summary(summary);
	else
	    summary_text = "NO SUMMARY FIELD FOR EVENT";


	if(error_flag == 0 && recur.freq == ICAL_NO_RECURRENCE)
	{
	    printf("%04i%02i%02i%s ", start_value.year, start_value.month, start_value.day, startstop);
	    printf("%s %s\n", summary_text, time);
	    good_event++;
	}
	else if(error_flag == 0 && recur.freq == ICAL_DAILY_RECURRENCE)
	{
	    if(recur.interval == 1)
		printf("DAILY%s %s %s", startstop, summary_text, time);
	    else
		error_flag = 1;
	}
	else if(error_flag == 0 && recur.freq == ICAL_YEARLY_RECURRENCE)
	{
	    printf("0000%02i%02i ", start_value.month, start_value.day);
	    printf("%s %s\n", summary_text, time);
	    good_event++;
	}

	else if(error_flag == 0 && recur.freq == ICAL_WEEKLY_RECURRENCE)
	{
	    if(recur.interval == 1)
	    {
		switch(icalrecurrencetype_day_day_of_week(recur.by_day[0]))
		{
		    case ICAL_SUNDAY_WEEKDAY:
			printf("SUN%s %s %s\n", startstop, summary_text, time);
			good_event++;
			break;
		    case ICAL_MONDAY_WEEKDAY:
			printf("MON%s %s %s\n", startstop, summary_text, time);
			good_event++;
			break;
		    case ICAL_TUESDAY_WEEKDAY:
			printf("TUE%s %s %s\n", startstop, summary_text, time);
			good_event++;
			break;
		    case ICAL_WEDNESDAY_WEEKDAY:
			printf("WED%s %s %s\n", startstop, summary_text, time);
			good_event++;
			break;
		    case ICAL_THURSDAY_WEEKDAY:
			printf("THU%s %s %s\n", startstop, summary_text, time);
			good_event++;
			break;
		    case ICAL_FRIDAY_WEEKDAY:
			printf("FRI%s %s %s\n", startstop, summary_text, time);
			good_event++;
			break;
		    case ICAL_SATURDAY_WEEKDAY:
			printf("SAT%s %s %s\n", startstop, summary_text, time);
			good_event++;
			break;
		    default:
			error_flag = 1; break;

		}
	    }


	    else if(recur.interval == 52)
	    {
		short n = recur.week_start;
		int weekday = -1;
		switch(icalrecurrencetype_day_day_of_week(recur.by_day[0]))
		{
		    case ICAL_SUNDAY_WEEKDAY:     weekday = 1; break;
		    case ICAL_MONDAY_WEEKDAY:     weekday = 2; break;
		    case ICAL_TUESDAY_WEEKDAY:    weekday = 3; break;
		    case ICAL_WEDNESDAY_WEEKDAY:  weekday = 4; break;
		    case ICAL_THURSDAY_WEEKDAY:   weekday = 5; break;
		    case ICAL_FRIDAY_WEEKDAY:     weekday = 6; break;
		    case ICAL_SATURDAY_WEEKDAY:   weekday = 7; break;
		    default:                   error_flag = 1; break;
		}

		if(n>0)
		{
		    printf("*%02i%1i%i%s ", start_value.month, n, weekday, startstop);
		    printf("%s %s\n", summary_text, time);
		    good_event++;
		}
		else if(n==-1)
		{
		    printf("*%02iL%i%s ", start_value.month, weekday, startstop);
		    printf("%s %s\n", summary_text, time);
		    good_event++;
		}
		else
		    error_flag = 1;
	    }
	    else
		error_flag = 1;
	}

	else if(error_flag == 0 && recur.freq == ICAL_MONTHLY_RECURRENCE)
	{
	    if(recur.interval == 1)
	    {
		if(recur.by_day[0] == ICAL_RECURRENCE_ARRAY_MAX)
		{
		    printf("000000%02i%s %s %s\n", start_value.day, startstop, summary_text, time);
		    good_event++;
		}

		else
		{
		    short n = icalrecurrencetype_day_position(recur.by_day[0]);
		    int weekday = -1;



		    switch(icalrecurrencetype_day_day_of_week(recur.by_day[0]))
		    {
			case ICAL_SUNDAY_WEEKDAY:     weekday = 1; break;
			case ICAL_MONDAY_WEEKDAY:     weekday = 2; break;
			case ICAL_TUESDAY_WEEKDAY:    weekday = 3; break;
			case ICAL_WEDNESDAY_WEEKDAY:  weekday = 4; break;
			case ICAL_THURSDAY_WEEKDAY:   weekday = 5; break;
			case ICAL_FRIDAY_WEEKDAY:     weekday = 6; break;
			case ICAL_SATURDAY_WEEKDAY:   weekday = 7; break;
			default:                   error_flag = 1; break;
		    }

		    if(n>0)
		    {
			printf("*00%1i%i%s ", n, weekday, startstop);
			printf("%s %s\n", summary_text, time);
			good_event++;
		    }
		    else if(n==-1)
		    {
			printf("*00L%i%s ", weekday, startstop);
			printf("%s %s\n", summary_text, time);
			good_event++;
		    }
		    else
			error_flag = 1;
		}

	    }
	    else if(recur.interval == 12)
	    {
		short n = icalrecurrencetype_day_position(recur.by_day[0]);
		int weekday = -1;
		switch(icalrecurrencetype_day_day_of_week(recur.by_day[0]))
		{
		    case ICAL_SUNDAY_WEEKDAY:     weekday = 1; break;
		    case ICAL_MONDAY_WEEKDAY:     weekday = 2; break;
		    case ICAL_TUESDAY_WEEKDAY:    weekday = 3; break;
		    case ICAL_WEDNESDAY_WEEKDAY:  weekday = 4; break;
		    case ICAL_THURSDAY_WEEKDAY:   weekday = 5; break;
		    case ICAL_FRIDAY_WEEKDAY:     weekday = 6; break;
		    case ICAL_SATURDAY_WEEKDAY:   weekday = 7; break;
		    default:                   error_flag = 1; break;
		}

		if(n>0)
		{
		    printf("*%02i%1i%i%s ", start_value.month, n, weekday, startstop);
		    printf("%s %s\n", summary_text, time);
		    good_event++;
		}
		else if(n==-1)
		{
		    printf("*%02iL%i%s ", start_value.month, weekday, startstop);
		    printf("%s %s\n", summary_text, time);
		    good_event++;
		}
		else
		    error_flag = 1;

	    }
	    else
		error_flag = 1;

	}

	else
	    error_flag = 1;




	if(error_flag == 1)
	{
	    fprintf(stderr, "Can't handle this yet:\n");
	    fprintf(stderr, "%s",icalcomponent_as_ical_string(vevent));
	    bad_event++;
	    error_flag = 0;
	}

	vevent =  icalcomponent_get_next_component(c, ICAL_VEVENT_COMPONENT);
    }


    fprintf(stderr, "Events converted: %i\n", good_event);
    fprintf(stderr, "Events that could not be converted: %i\n", bad_event);
}



int main(int argc, char* argv[])
{

    char* line;
    FILE* stream;
    icalcomponent *c;

    /* Create a new parser object */
    icalparser *parser = icalparser_new();

    if(argc != 4)
    {
	fprintf(stderr, "ical2pal %s - Copyright (C) 2004, Scott Kuhl\n", PAL_VERSION);
	fprintf(stderr, "  %s", "pal is licensed under the GNU General Public License and has NO WARRANTY.\n\n");

	fprintf(stderr, "Usage:  ical2pal ical-file.ics CC EventType > pal-file.pal\n");
	fprintf(stderr, "        CC - 2 characters used to mark event in pal calendar.\n");
	fprintf(stderr, "        EventType - Description of the type of events in file.\n");

	fprintf(stderr, "\n");


	exit(1);
    }

    if(strlen(argv[2]) == 0)
	printf("__ ");
    else if(strlen(argv[2]) == 1)
	printf("%c_ ", *argv[2]);
    else
	printf("%c%c ", *argv[2], *(argv[2]+1));

    printf("%s\n", argv[3]);

    stream = fopen(argv[1],"r");

    assert(stream != 0);

    /* Tell the parser what input routine it should use. */
    icalparser_set_gen_data(parser,stream);

    do{

	/* Get a single content line by making one or more calls to
           read_stream()*/
	line = icalparser_get_line(parser,read_stream);

	/* Now, add that line into the parser object. If that line
           completes a component, c will be non-zero */
	c = icalparser_add_line(parser,line);


	if (c != 0)
	{
	    output_pal_from_ical(c);
	    icalcomponent_free(c);
	}

    } while (line != 0);


    icalparser_free(parser);

    fprintf(stderr, "DONE!\n\n");
    fprintf(stderr, "NOTE: This program ignores the timezone settings in the ical calendar.\n");
    fprintf(stderr, "Some events might not transfer because there is no equivalent event time\nin the pal calendar file format.\n");
    return 0;
}
