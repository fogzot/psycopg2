/* typecast_mxdatetime.c - date and time typecasting functions to mx types
 *
 * Copyright (C) 2001-2003 Federico Di Gregorio <fog@debian.org>
 *
 * This file is part of the psycopg module.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "mxDateTime.h"

/* the pointer to the mxDateTime API is initialized by the module init code,
   we just need to grab it */
extern mxDateTimeModule_APIObject *mxDateTimeP;

/** DATE - cast a date into mx.DateTime python object **/

static PyObject *
typecast_MXDATE_cast(PyObject *s, PyObject *curs)
{
    int n, y=0, m=0, d=0;
    int hh=0, mm=0;
    double ss=0.0;
    char *str;

    if (s == Py_None) {Py_INCREF(s); return s;}

    str = PyString_AsString(s);
    
    /* check for infinity */
    if (!strcmp(str, "infinity") || !strcmp(str, "-infinity")) {
        if (str[0] == '-') {
            return mxDateTimeP->DateTime_FromDateAndTime(-999998,1,1, 0,0,0);
        }
        else {
            return mxDateTimeP->DateTime_FromDateAndTime(999999,12,31, 0,0,0);
        }
    }
    
    Dprintf("typecast_MXDATE_cast: s = %s", str);
    n = sscanf(str, "%d-%d-%d %d:%d:%lf", &y, &m, &d, &hh, &mm, &ss);
    Dprintf("typecast_MXDATE_cast: date parsed, %d components", n);
    
    if (n != 3 && n != 6) {
        PyErr_SetString(DataError, "unable to parse date");
        return NULL;
    }
    return mxDateTimeP->DateTime_FromDateAndTime(y, m, d, hh, mm, ss);
}

/** TIME - parse time into an mx.DateTime object **/

static PyObject *
typecast_MXTIME_cast(PyObject *s, PyObject *curs)
{
    int n, hh=0, mm=0;
    double ss=0.0;    
    char *str;

    if (s == Py_None) {Py_INCREF(s); return s;}

    str = PyString_AsString(s);
    
    Dprintf("typecast_MXTIME_cast: s = %s", str);
    
    n = sscanf(str, "%d:%d:%lf", &hh, &mm, &ss);
    Dprintf("typecast_MXTIME_cast: time parsed, %d components", n);
    Dprintf("typecast_MXTIME_cast: hh = %d, mm = %d, ss = %f", hh, mm, ss);
    
    if (n != 3) {
        PyErr_SetString(DataError, "unable to parse time");
        return NULL;
    }

    return mxDateTimeP->DateTimeDelta_FromTime(hh, mm ,ss);
}

/** INTERVAL - parse an interval into an mx.DateTimeDelta **/

static PyObject *
typecast_MXINTERVAL_cast(PyObject *s, PyObject *curs)
{
    long years = 0, months = 0, days = 0, denominator = 1;
    double hours = 0.0, minutes = 0.0, seconds = 0.0, hundredths = 0.0;
    double v = 0.0, sign = 1.0;
    int part = 0;
    char *str;

    if (s == Py_None) {Py_INCREF(s); return s;}
    
    str = PyString_AsString(s);
    Dprintf("typecast_MXINTERVAL_cast: s = %s", str);
    
    while (*str) {
        switch (*str) {

        case '-':
            sign = -1.0;
            break;

        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
            v = v*10 + (double)*str - (double)'0';
            Dprintf("typecast_MXINTERVAL_cast: v = %f", v);
            if (part == 6){
                denominator *= 10;
                Dprintf("typecast_MXINTERVAL_cast: denominator = %ld",
                        denominator);
            }
            break;

        case 'y':
            if (part == 0) {
                years = (long)(v*sign);
                str = skip_until_space(str);
                Dprintf("typecast_MXINTERVAL_cast: years = %ld, rest = %s",
                        years, str);
                v = 0.0; sign = 1.0; part = 1;
            }
            break;

        case 'm':
            if (part <= 1) {
                months = (long)(v*sign);
                str = skip_until_space(str);
                Dprintf("typecast_MXINTERVAL_cast: months = %ld, rest = %s",
                        months, str);
                v = 0.0; sign = 1.0; part = 2;
            }
            break;

        case 'd':
            if (part <= 2) {
                days = (long)(v*sign);
                str = skip_until_space(str);
                Dprintf("typecast_MXINTERVAL_cast: days = %ld, rest = %s",
                        days, str);
                v = 0.0; sign = 1.0; part = 3;
            }
            break;

        case ':':
            if (part <= 3) {
                hours = v;
                Dprintf("typecast_MXINTERVAL_cast: hours = %f", hours);
                v = 0.0; part = 4;
            }
            else if (part == 4) {
                minutes = v;
                Dprintf("typecast_MXINTERVAL_cast: minutes = %f", minutes);
                v = 0.0; part = 5;
            }
            break;

        case '.':
            if (part == 5) {
                seconds = v;
                Dprintf("typecast_MXINTERVAL_cast: seconds = %f", seconds);
                v = 0.0; part = 6;
            }
            break;   

        default:
            break;
        }
        
        str++;
    }

    /* manage last value, be it minutes or seconds or hundredths of a second */
    if (part == 4) {
        minutes = v;
        Dprintf("typecast_MXINTERVAL_cast: minutes = %f", minutes);
    }
    else if (part == 5) {
        seconds = v;
        Dprintf("typecast_MXINTERVAL_cast: seconds = %f", seconds);
    }
    else if (part == 6) {
        hundredths = v;
        Dprintf("typecast_MXINTERVAL_cast: hundredths = %f", hundredths);
        hundredths = hundredths/denominator;
        Dprintf("typecast_MXINTERVAL_cast: fractions = %.20f", hundredths);
    }
    
    /* calculates seconds */
    if (sign < 0.0) {
        seconds = - (hundredths + seconds + minutes*60 + hours*3600);
    }
    else {
        seconds += hundredths + minutes*60 + hours*3600;
    }

    /* calculates days */
    days += years*365 + months*30;
    
    Dprintf("typecast_MXINTERVAL_cast: days = %ld, seconds = %f",
            days, seconds);
    return mxDateTimeP->DateTimeDelta_FromDaysAndSeconds(days, seconds);
}

/* psycopg defaults to using mx types */

#ifdef PSYCOPG_DEFAULT_MXDATETIME 
#define typecast_DATE_cast typecast_MXDATE_cast
#define typecast_TIME_cast typecast_MXTIME_cast
#define typecast_INTERVAL_cast typecast_MXINTERVAL_cast
#define typecast_DATETIME_cast typecast_MXDATE_cast
#endif
