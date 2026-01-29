


#define HOST_MINGW
#define HOST_WIN32
/* We prefer using non-oldnames functions, see also win32/fb_win32.h */
#define NO_OLDNAMES
#define _NO_OLDNAMES
#define fb_hSign( x ) (((x) < 0) ? -1 : 1)
#define FB_CHAR_TO_INT(ch)  ((int) ((unsigned) (unsigned char) (ch)))







double fixDouble(double x)
{
	if( x == 0.0 ) {
		return 0.0;
	} else if( x > 0.0 ) {
		return floor(fabs(x));
	} else {
		return floor(fabs(x)) * -1;
	}
}



static double xCore_Frequency;

int xCore_TimeLeap(int y)
{
	if( ( y % 400 ) == 0 )
		return 1;
	if( ( y % 100 ) == 0 )
		return 0;
	return ( ( ( y & 3 ) == 0 ) ? 1 : 0 );
}

int xCore_DaysInMonth(int y, int m)
{
	static const int days[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
	if ( (m >= 1) && (m <= 12) ) {
		return ((m == 2) ? days[m-1] + xCore_TimeLeap(y) : days[m-1]);
	} else {
		return 0;
	}
}

char* xCore_TimeS()
{
	time_t rawtime = time(NULL);
	struct tm* pstm = localtime(&rawtime);
	return xCore_FormatA("%02d:%02d:%02d", pstm->tm_hour, pstm->tm_min, pstm->tm_sec);
}

char* xCore_DateS()
{
	time_t rawtime = time(NULL);
	struct tm* pstm = localtime(&rawtime);
	return xCore_FormatA("%04d-%02d-%02d", 1900 + pstm->tm_year, pstm->tm_mon, pstm->tm_mday);
}

void xCore_Timer_Init()
{
	LARGE_INTEGER count;
	if( QueryPerformanceFrequency( &count ) ) {
		xCore_Frequency = 1.0 / count.QuadPart;
	} else {
		xCore_Frequency = 0.0;
	}
}

double xCore_Timer()
{
	if ( xCore_Frequency == 0.0 ) {
		return (double)GetTickCount() * 0.001;
	} else {
		LARGE_INTEGER count;
		QueryPerformanceCounter(&count);
		return (double)count.QuadPart * xCore_Frequency;
	}
}

int xCore_SetDateTime(int y, int M, int d, int h, int m, int s)
{
	SYSTEMTIME st;
	GetLocalTime( &st );
	st.wYear = y;
	st.wMonth = M;
	st.wDay = d;
	st.wHour = h;
	st.wMinute = m;
	st.wSecond = s;
	return SetLocalTime( &st );
}

int xCore_SetDate(int y, int m, int d)
{
	SYSTEMTIME st;
	GetLocalTime( &st );
	st.wYear = y;
	st.wMonth = m;
	st.wDay = d;
	return SetLocalTime( &st );
}

int xCore_SetTime(int h, int m, int s)
{
	SYSTEMTIME st;
	GetLocalTime( &st );
	st.wHour = h;
	st.wMinute = m;
	st.wSecond = s;
	return SetLocalTime( &st );
}

// 待重构优化			支持格式 : [mm/dd/yy]、[mm/dd/yyyy]、[mm-dd-yy]、[mm-dd-yyyy]
int xCore_SetDateSA(char* sDate)
{
	char *t, c, sep;
	int m, d, y;

	/* get month */
	m = 0;
	for( t = sDate; (c = *t) && isdigit(c); t++ ) {
		m = m * 10 + c - '0';
	}

	if( ((c != '/') && (c != '-')) || (m < 1) || (m > 12) ) {
		return 0;
	}
	sep = c;

	/* get day */
	d = 0;
	for( t++; (c = *t) && isdigit(c); t++ ) {
		d = d * 10 + c - '0';
	}

	if( (c != sep) || (d < 1) || (d > 31) ) {
		return 0;
	}

	/* get year */
	y = 0;
	for( t++; (c = *t) && isdigit(c); t++ ) {
		y = y * 10 + c - '0';
	}

	if (y < 100) y += 1900;
	
	return xCore_SetDate( y, m, d );
}

// 待重构优化			支持格式 : [hh]、[hh:mm]、[hh:mm:ss]
int xCore_SetTimeSA(char* sTime)
{
	char *t, c;
	int i, h, m = 0, s = 0;

	/* get hours */
	h = 0;
	for( i = 0, t = sTime; (c = *t) && isdigit(c); t++, i += 10 ) {
		h = h * i + c - '0';
	}

	if( (h > 23) || (c != '\0' && c != ':') ) {
		return 0;
	}

	if( c != '\0' ) {
		/* get minutes */
		m = 0;
		for( i = 0, t++; (c = *t) && isdigit(c); t++, i += 10 ) {
			 m = m * i + c - '0';
		}

		if( (m > 59) || (c != '\0' && c != ':') ) {
			return 0;
		}

		if( c != '\0' ) {
			/* get seconds */
			s = 0;
			for (i = 0, t++; (c = *t) && isdigit(c); t++, i += 10)
				s = s * i + c - '0';
		}
	}

	if( (s > 59) || (c != '\0') ) {
		return 0;
	}
	
	return xCore_SetTime( h, m, s );
}

double xCore_TimeSerial(int h, int m, int s)
{
	double dblHour = h / 24.0;
	double dblMinute = m / (24.0 * 60.0);
	double dblSecond = s / (24.0 * 60.0 * 60.0);
	return dblHour + dblMinute + dblSecond;
}

void xCore_hNormalizeDate( int *pDay, int *pMonth, int *pYear )
{
    {
        int day = *pDay;
        int month = *pMonth;
        int year = *pYear;
        if( month < 1 ) {
            int sub_months = -month + 1;
            int sub_years = (sub_months + 11) / 12;
            year -= sub_years;
            month = sub_years * 12 - sub_months + 1;
        } else {
            --month;
            year += month / 12;
            month %= 12;
            ++month;
        }

        if( day < 1 ) {
            int sub_days = -day + 1;
            while (sub_days > 0) {
                int dom;
                if( --month==0 ) {
                    month=12;
                    --year;
                }
                dom = xCore_DaysInMonth( year, month );
                if( sub_days > dom ) {
                    sub_days -= dom;
                } else {
                    day = dom - sub_days + 1;
                    sub_days = 0;
                }
            }
        } else {
            int dom = xCore_DaysInMonth( year, month );
            while( day > dom ) {
                day -= dom;
                if( ++month==13 ) {
                    month = 1;
                    ++year;
                }
                dom = xCore_DaysInMonth( year, month );
            }
        }
        *pDay = day;
        *pMonth = month;
        *pYear = year;
    }
}

int xCore_DateSerial(int y, int m, int d)
{
	int result = 2;
	int cur_year = 1900;
	int cur_month = 1;
	int cur_day = 1;

	xCore_hNormalizeDate( &d, &m, &y );

	if( cur_year < y ) {
		while( cur_year != y ) {
			result += xCore_DaysInYear( cur_year++ );
		}
	} else {
		while( cur_year != y ) {
			result -= xCore_DaysInYear( --cur_year );
		}
	}

	while( cur_month != m ) {
		result += xCore_DaysInMonth( y, cur_month++ );
	}

	result += d - cur_day;

	return result;
}

double xCore_Now()
{
	time_t rawtime = time(NULL);
	struct tm* ptm = localtime(&rawtime);
	double dblDate = xCore_DateSerial( 1900 + ptm->tm_year, 1 + ptm->tm_mon, ptm->tm_mday );
	double dblTime = xCore_TimeSerial( ptm->tm_hour, ptm->tm_min, ptm->tm_sec );
	return dblDate + dblTime;
}

int xCore_Date()
{
	time_t rawtime = time(NULL);
	struct tm* ptm = localtime(&rawtime);
	return xCore_DateSerial( 1900 + ptm->tm_year, 1 + ptm->tm_mon, ptm->tm_mday );
}

double xCore_Time()
{
	time_t rawtime = time(NULL);
	struct tm* ptm = localtime(&rawtime);
	return xCore_TimeSerial( ptm->tm_hour, ptm->tm_min, ptm->tm_sec );
}

int xCore_Year(double serial)
{
    int year;
    xCore_hDateDecodeSerial( serial, &year, NULL, NULL );
    return year;
}

int xCore_Month(double serial)
{
    int month;
    xCore_hDateDecodeSerial( serial, NULL, &month, NULL );
    return month;
}

int xCore_Day(double serial)
{
    int day;
    xCore_hDateDecodeSerial( serial, NULL, NULL, &day );
    return day;
}

int xCore_Weekday(double serial, int first_day_of_week)
{
    int dow = ((int) (floor(serial) - 1) % 7) + 1;

    if( first_day_of_week == XCORE_WEEK_DAY_SYSTEM ) {
        /* FIXME: query system default */
        first_day_of_week = XCORE_WEEK_DAY_DEFAULT;
    }

    dow -= first_day_of_week - 1;
    if( dow < 1 ) {
        dow += 7;
    } else if( dow > 7 ) {
        dow -= 7;
    }
    return dow;
}

int xCore_Hour(double serial)
{
    int hour;
    xCore_hTimeDecodeSerial( serial, &hour, NULL, NULL, TRUE );
    return hour;
}

int xCore_Minute(double serial)
{
    int minute;
    xCore_hTimeDecodeSerial( serial, NULL, &minute, NULL, TRUE );
    return minute;
}

int xCore_Second(double serial)
{
    int second;
    xCore_hTimeDecodeSerial( serial, NULL, NULL, &second, TRUE );
    return second;
}

int xCore_hTimeGetIntervalType( char* interval )
{
    int result = XCORE_TIME_INTERVAL_INVALID;
    if( interval!=NULL ) {
        if( strcmp( interval, "yyyy" )==0 ) {
            result = XCORE_TIME_INTERVAL_YEAR;
        } else if( strcmp( interval, "q" )==0 ) {
            result = XCORE_TIME_INTERVAL_QUARTER;
        } else if( strcmp( interval, "m" )==0 ) {
            result = XCORE_TIME_INTERVAL_MONTH;
        } else if( strcmp( interval, "y" )==0 ) {
            result = XCORE_TIME_INTERVAL_DAY_OF_YEAR;
        } else if( strcmp( interval, "d" )==0 ) {
            result = XCORE_TIME_INTERVAL_DAY;
        } else if( strcmp( interval, "w" )==0 ) {
            result = XCORE_TIME_INTERVAL_WEEKDAY;
        } else if( strcmp( interval, "ww" )==0 ) {
            result = XCORE_TIME_INTERVAL_WEEK_OF_YEAR;
        } else if( strcmp( interval, "h" )==0 ) {
            result = XCORE_TIME_INTERVAL_HOUR;
        } else if( strcmp( interval, "n" )==0 ) {
            result = XCORE_TIME_INTERVAL_MINUTE;
        } else if( strcmp( interval, "s" )==0 ) {
            result = XCORE_TIME_INTERVAL_SECOND;
        }
    }
    return result;
}

void xCore_hDateDecodeSerial( double serial, int *pYear, int *pMonth, int *pDay )
{
    int tmp_days;
    int cur_year = 1900;
    int cur_month = 1;
    int cur_day = 1;

    serial = floor( serial );

    serial -= 2;
    while( serial < 0 ) {
        serial += xCore_DaysInYear( --cur_year );
    }

    while( serial >= (tmp_days = xCore_DaysInYear( cur_year ) ) ) {
        serial -= tmp_days;
        ++cur_year;
    }

    if( pMonth || pDay ) {
        while( serial >= (tmp_days = xCore_DaysInMonth( cur_year, cur_month ) ) ) {
            serial -= tmp_days;
            ++cur_month;
        }
    }

    cur_day += serial;

    if( pYear )
        *pYear = cur_year;
    if( pMonth )
        *pMonth = cur_month;
    if( pDay )
        *pDay = cur_day;
}

void xCore_hTimeDecodeSerial( double serial, int *pHour, int *pMinute, int *pSecond, int use_qb_hack )
{
    int hour, minute, second;
    double dblFixValue = fixDouble( serial );

    serial -= dblFixValue;
    if( fb_hSign( serial ) == -1 ) {
        if( use_qb_hack ) {
            /* Test for both 0.0 and -0.0 because FPUs may handle this as
             * different values ... */
            if( dblFixValue==0.0 || dblFixValue==-0.0 ) {
                /* QB quirk ! */
                serial = -serial;
            } else {
                serial += 1.0l;
            }
        } else {
            serial += 1.0l;
        }
    }

    /* The inaccuracies of the IEEE floating point data types ... */
    serial += 0.000000001l;

    serial *= 24.0l;
    hour = (int) serial;
    serial -= hour;
    serial *= 60.0l;
    minute = (int) serial;
    serial -= minute;
    serial *= 60.0l;
    second = (int) serial;

    if( pHour )
        *pHour = hour;
    if( pMinute )
        *pMinute = minute;
    if( pSecond )
        *pSecond = second;
}

double xCore_DateAdd( char* interval, double interval_value_arg, double serial )
{
    int year, month, day, hour, minute, second;
    int carry_value, test_value;
    int interval_value = (int)fixDouble( interval_value_arg );
    int interval_type = xCore_hTimeGetIntervalType( interval );

    xCore_hTimeDecodeSerial ( serial, &hour, &minute, &second, FALSE );
    xCore_hDateDecodeSerial ( serial, &year, &month, &day );

    switch ( interval_type ) {
    case XCORE_TIME_INTERVAL_YEAR:
        year += interval_value;
        break;
    case XCORE_TIME_INTERVAL_QUARTER:
        month += interval_value * 3;
        break;
    case XCORE_TIME_INTERVAL_MONTH:
        month += interval_value;
        break;
    case XCORE_TIME_INTERVAL_DAY_OF_YEAR:
    case XCORE_TIME_INTERVAL_DAY:
    case XCORE_TIME_INTERVAL_WEEKDAY:
        day += interval_value;
        break;
    case XCORE_TIME_INTERVAL_WEEK_OF_YEAR:
        day += interval_value * 7;
        break;
    case XCORE_TIME_INTERVAL_HOUR:
        hour += interval_value;
        break;
    case XCORE_TIME_INTERVAL_MINUTE:
        minute += interval_value;
        break;
    case XCORE_TIME_INTERVAL_SECOND:
        second += interval_value;
        break;
    case XCORE_TIME_INTERVAL_INVALID:
    default:
        break;
    }

    /* Normalize date/time */
    switch ( interval_type ) {
    case XCORE_TIME_INTERVAL_HOUR:
    case XCORE_TIME_INTERVAL_MINUTE:
    case XCORE_TIME_INTERVAL_SECOND:
    case XCORE_TIME_INTERVAL_DAY_OF_YEAR:
    case XCORE_TIME_INTERVAL_DAY:
    case XCORE_TIME_INTERVAL_WEEKDAY:
    case XCORE_TIME_INTERVAL_WEEK_OF_YEAR:
        break;
    case XCORE_TIME_INTERVAL_YEAR:
    case XCORE_TIME_INTERVAL_QUARTER:
    case XCORE_TIME_INTERVAL_MONTH:
        /* Handle wrap-around for month */
        if( month < 1 ) {
            carry_value = (month - 12) / 12;
        } else {
            carry_value = (month - 1) / 12;
        }
        year += carry_value;
        month -= carry_value * 12;
        /* No wrap-around ... instead we must saturate the day */
        test_value = xCore_DaysInMonth( year, month );
        if( day > test_value )
            day = test_value;
        break;
    }
    return xCore_DateSerial( year, month, day ) + xCore_TimeSerial( hour, minute, second );
}

long long xCore_DateDiff( char* interval, double serial1, double serial2,
                              int first_day_of_week, int first_day_of_year )
{
    int year1, month1, hour, minute, second, week;
    int year2, month2;
    long long result = 0;
    double serial;
    int interval_type = xCore_hTimeGetIntervalType( interval );

    //fb_ErrorSetNum( FB_RTERROR_OK );

    switch ( interval_type ) {
    case XCORE_TIME_INTERVAL_YEAR:
        xCore_hDateDecodeSerial ( serial1, &year1, NULL, NULL );
        xCore_hDateDecodeSerial ( serial2, &year2, NULL, NULL );
        result = year2 - year1;
        break;
    case XCORE_TIME_INTERVAL_QUARTER:
    case XCORE_TIME_INTERVAL_MONTH:
        xCore_hDateDecodeSerial ( serial1, &year1, &month1, NULL );
        xCore_hDateDecodeSerial ( serial2, &year2, &month2, NULL );
        result = (month2 - month1) + (year2 - year1) * 12;
        if( interval_type==XCORE_TIME_INTERVAL_QUARTER )
            result = result / 3;
        break;
    case XCORE_TIME_INTERVAL_DAY_OF_YEAR:
    case XCORE_TIME_INTERVAL_DAY:
        result = (long long) (floor(serial2) - floor(serial1));
        break;
    case XCORE_TIME_INTERVAL_WEEKDAY:
    case XCORE_TIME_INTERVAL_WEEK_OF_YEAR:
        xCore_hDateDecodeSerial ( serial1, &year1, NULL, NULL );
        week = xCore_hGetWeekOfYear( year1,
                                  serial1,
                                  first_day_of_year, first_day_of_week );
        result = xCore_hGetWeekOfYear( year1,
                                    serial2,
                                    first_day_of_year, first_day_of_week );
        if( week > 0 )
            --week;
        if( result > 0 )
            --result;
        result -= week;
        if( interval_type==XCORE_TIME_INTERVAL_WEEKDAY ) {
            int add_value;
            if( serial1 > serial2 ) {
                double serial_tmp = serial1;
                serial1 = serial2;
                serial2 = serial_tmp;
                add_value = 1;
            } else {
                add_value = -1;
            }
            if( xCore_Weekday( serial1, first_day_of_week ) > xCore_Weekday( serial2, first_day_of_week ) )
                result += add_value;
        }
        break;
    case XCORE_TIME_INTERVAL_HOUR:
        serial = serial2 - serial1;
        xCore_hTimeDecodeSerial ( serial, &hour, NULL, NULL, FALSE );
        result = (long long) (hour + floor(serial) * 24.0l);
        break;
    case XCORE_TIME_INTERVAL_MINUTE:
        serial = serial2 - serial1;
        xCore_hTimeDecodeSerial ( serial, &hour, &minute, NULL, FALSE );
        result = (long long) (minute + (hour + floor(serial) * 24.0l) * 60.0l);
        break;
    case XCORE_TIME_INTERVAL_SECOND:
        serial = serial2 - serial1;
        xCore_hTimeDecodeSerial ( serial, &hour, &minute, &second, FALSE );
        result = (long long) (second + (minute + (hour + floor(serial) * 24.0l) * 60.0l) * 60.0l);
        break;
    case XCORE_TIME_INTERVAL_INVALID:
    default:
        //fb_ErrorSetNum( FB_RTERROR_ILLEGALFUNCTIONCALL );
        break;
    }

    return result;
}

int xCore_DatePart( char* interval, double serial, int first_day_of_week, int first_day_of_year )
{
    int result = 0;
    int year, month, day, hour, minute, second;
    int interval_type = xCore_hTimeGetIntervalType( interval );

    switch ( interval_type ) {
    case XCORE_TIME_INTERVAL_YEAR:
        xCore_hDateDecodeSerial ( serial, &year, NULL, NULL );
        result = year;
        break;
    case XCORE_TIME_INTERVAL_QUARTER:
        xCore_hDateDecodeSerial ( serial, NULL, &month, NULL );
        result = ((month - 1) / 3) + 1;
        break;
    case XCORE_TIME_INTERVAL_MONTH:
        xCore_hDateDecodeSerial ( serial, NULL, &month, NULL );
        result = month;
        break;
    case XCORE_TIME_INTERVAL_DAY_OF_YEAR:
        xCore_hDateDecodeSerial ( serial, &year, &month, &day );
        result = xCore_hGetDayOfYearEx( year, month, day );
        break;
    case XCORE_TIME_INTERVAL_DAY:
        xCore_hDateDecodeSerial ( serial, NULL, NULL, &day );
        result = day;
        break;
    case XCORE_TIME_INTERVAL_WEEKDAY:
        result = xCore_Weekday( serial, first_day_of_week );
        break;
    case XCORE_TIME_INTERVAL_WEEK_OF_YEAR:
        xCore_hDateDecodeSerial ( serial, &year, NULL, NULL );
        result = xCore_hGetWeekOfYear( year, serial, first_day_of_year, first_day_of_week );
        if( result < 0 )
            result = xCore_hGetWeekOfYear( year - 1, serial, first_day_of_year, first_day_of_week );
        break;
    case XCORE_TIME_INTERVAL_HOUR:
        xCore_hTimeDecodeSerial ( serial, &hour, NULL, NULL, FALSE );
        result = hour;
        break;
    case XCORE_TIME_INTERVAL_MINUTE:
        xCore_hTimeDecodeSerial ( serial, NULL, &minute, NULL, FALSE );
        result = minute;
        break;
    case XCORE_TIME_INTERVAL_SECOND:
        xCore_hTimeDecodeSerial ( serial, NULL, NULL, &second, FALSE );
        result = second;
        break;
    case XCORE_TIME_INTERVAL_INVALID:
    default:
        //fb_ErrorSetNum( FB_RTERROR_ILLEGALFUNCTIONCALL );
        break;
    }

    return result;
}

static int xCore_hDateOrder( int *pOrderDay, int *pOrderMonth, int *pOrderYear )
{
    int order_month = 0, order_day = 1, order_year = 2, order_index = 0;
    int got_sep;
    char short_format[12] = "MM/dd/yyyy\0";

    got_sep = TRUE;
    for( int tmp=0; short_format[tmp]; ++tmp ) {
        int ch = FB_CHAR_TO_INT( short_format[tmp] );
        if( islower(ch) )
            ch = toupper( ch );
        switch ( ch ) {
        case 'D':
            order_day = order_index;
            got_sep = FALSE;
            break;
        case 'M':
            order_month = order_index;
            got_sep = FALSE;
            break;
        case 'Y':
            order_year = order_index;
            got_sep = FALSE;
            break;
        default:
            if( !got_sep ) {
                ++order_index;
            }
            got_sep = TRUE;
            break;
        }
    }

    if( order_day==order_month || order_day==order_year || order_month==order_year )
        return FALSE;
    if( order_day > 2 || order_month > 2 || order_year > 2 )
        return FALSE;

    if( pOrderDay )
        *pOrderDay = order_day;
    if( pOrderMonth )
        *pOrderMonth = order_month;
    if( pOrderYear )
        *pOrderYear = order_year;

    return TRUE;
}

static const char *pszWeekdayNamesLong[7] = {
    "Sunday",
    "Monday",
    "Tuesday",
    "Wednesday",
    "Thursday",
    "Friday",
    "Saturday"
};

static const char *pszWeekdayNamesShort[7] = {
    "Sun",
    "Mon",
    "Tue",
    "Wed",
    "Thu",
    "Fri",
    "Sat"
};

const char* xCore_IntlGetWeekdayName( int weekday, int short_names )
{
    if( weekday < 1 || weekday > 7 )
        return NULL;
    if( short_names ) {
        return pszWeekdayNamesShort[weekday-1];
    } else {
        return pszWeekdayNamesLong[weekday-1];
    }
    return NULL;
}

static const char *pszMonthNamesLong[12] = {
    "January",
    "February",
    "March",
    "April",
    "May",
    "June",
    "July",
    "August",
    "September",
    "October",
    "November",
    "December"
};

static const char *pszMonthNamesShort[12] = {
    "Jan",
    "Feb",
    "Mar",
    "Apr",
    "May",
    "Jun",
    "Jul",
    "Aug",
    "Sep",
    "Oct",
    "Nov",
    "Dec"
};

const char* xCore_IntlGetMonthName( int month, int short_names )
{
    if( month < 1 || month > 12 )
        return NULL;
    if( short_names ) {
        return pszMonthNamesShort[month-1];
    } else {
        return pszMonthNamesLong[month-1];
    }
    return NULL;
}

void xCore_hGetBeginOfWeek( int *pYear, int *pMonth, int *pDay,
                         int first_day_of_week )
{
    double serial;
    int weekday;

    serial = xCore_DateSerial( *pYear, *pMonth, *pDay );
    weekday = xCore_Weekday( serial, first_day_of_week );
    serial -= weekday - 1;

    xCore_hDateDecodeSerial( serial, pYear, pMonth, pDay );
}

double xCore_hGetFirstWeekOfYear( int year,
                               int first_day_of_year, int first_day_of_week )
{
    int first_week_year, first_week_month, first_week_day;
    double serial_week_begin, serial_year_begin;
    int remaining_weekdays;

    if( first_day_of_year==XCORE_WEEK_FIRST_SYSTEM ) {
        /* FIXME: query system default */
        first_day_of_year = XCORE_WEEK_FIRST_DEFAULT;
    }

    serial_year_begin = xCore_DateSerial( year, 1, 1 );

    first_week_day = 1;
    first_week_month = 1;
    first_week_year = year;
    xCore_hGetBeginOfWeek( &first_week_year, &first_week_month, &first_week_day,
                        first_day_of_week );

    serial_week_begin = xCore_DateSerial( first_week_year,
                                       first_week_month,
                                       first_week_day );
    remaining_weekdays = (int) ((serial_week_begin + 7.0l) - serial_year_begin);

    switch( first_day_of_year ) {
    case XCORE_WEEK_FIRST_JAN_1:
        break;
    case XCORE_WEEK_FIRST_FOUR_DAYS:
        if( remaining_weekdays < 4 ) {
            serial_week_begin += 7.0l;
        }
        break;
    case XCORE_WEEK_FIRST_FULL_WEEK:
        if( remaining_weekdays < 7 ) {
            serial_week_begin += 7.0l;
        }
        break;
    }

    return serial_week_begin;
}

static int xCore_hIsMonth( const char *text, size_t text_len, const char **end_text, int short_name )
{
    const char *txt_end = text;
    int month;
    for( month=1; month!=13; ++month ) {
        const char *sMonthName = xCore_IntlGetMonthName( month, short_name );
        {
            size_t month_len = strlen( sMonthName );
            size_t len = ((text_len < month_len) ? text_len : month_len );
            int is_same = memcmp( text, sMonthName, len ) == 0;
            //fb_hStrDelTemp( sMonthName );
            if( is_same ) {
                if( text_len > len ) {
                    if( !isalpha( FB_CHAR_TO_INT(text[len]) ) ) {
                        txt_end = text + len;
                        break;
                    }
                } else {
                    txt_end = text + len;
                    break;
                }
            }
        }
    }
    if( month!=13 ) {
        if( short_name ) {
            /* There might follow a dot directly after the
             * abbreviated month name */
            if( *txt_end=='.' )
                ++txt_end;
        }
    } else {
        month = 0;
    }
    if( end_text!=NULL )
        *end_text = txt_end;
    return month;
}

static int xCore_hFindMonth( const char *text, size_t text_len, const char **end_text )
{
    int short_name;
    for( short_name=0; short_name!=2; ++short_name ) {
        int localized = 2;
        while( localized-- ) {
            int month = xCore_hIsMonth( text, text_len, end_text, short_name );
            if( month!=0 ) {
                return month;
            }
        }
    }
    return 0;
}

static int xCore_hCheckAMPM( const char *text, size_t *pLength )
{
    const char *text_start = text;
    int result = 0;

    /* skip WS */
    while ( isspace( *text ) )
        ++text;

    switch( *text ) {
    case 'a':
    case 'A':
        result = 1;
        ++text;
        break;
    case 'p':
    case 'P':
        result = 2;
        ++text;
        break;
    }
    if( result!=0 ) {
        char ch = *text;
        if( ch==0 ) {
            // ignore
        } else if( ch=='m' || ch=='M' ) {
            // everything's fine
            ++text;
        } else {
            result = 0;
        }
    }
    if( result!=0 ) {
        if( isalpha( *text ) )
            result = 0;
    }
    if( result!=0 ) {
        /* skip WS */
        while ( isspace( *text ) )
            ++text;
        if( pLength )
            *pLength = text - text_start;
    }
    return result;
}

#define InlineSelect(varn, index, num1, num2, num3) if(index==0){varn = num1;}else if(index==1){varn = num2;}else if(index==2){varn = num3;}
int xCore_hDateParse( const char *text, size_t text_len,
                   int *pDay, int *pMonth, int *pYear,
                   size_t *pLength )
{
    size_t length = 0, len = text_len;
    const char *text_start = text;
    int result = FALSE;
    int year = 1899, month = 12, day = 30;
    int order_year, order_month, order_day;
    const char *end_month;

    if( !xCore_hDateOrder( &order_day, &order_month, &order_year ) ) {
        /* switch to US date format */
        order_month = 0;
        order_day = 1;
        order_year = 2;
    }

    /* skip white spaces */
    while ( isspace( *text ) )
        ++text;
    len = text_len - (text - text_start);

    month = xCore_hFindMonth( text, len, &end_month );
    if( month != 0 ) {
        /* The string has the form: (MMMM|MMM) (d|dd)"," (yy|yyyy)  */
        char *endptr;
        text = end_month;
        day = strtol( text, &endptr, 10 );
        if( day>0 ) {

            /* skip white spaces */
            text = endptr;
            while ( isspace( *text ) )
                ++text;

            if( *text==',' ) {
                size_t year_size;
                year = strtol( ++text, &endptr, 10 );
                year_size = endptr - text;
                if( year_size > 0 ) {
                    if( year_size==2 )
                        year += 1900;

                    result = day <= xCore_DaysInMonth( year, month );
                    text = endptr;
                }
            }
        }
    } else {
        /* The string can be in the short or long format.
         *
         * The short format can be
         * [0-9]{1,2}(/|\-|\.)[0-9]{1,2}\1[0-9]{1,4}
         *                              ^^ reference to first divider
         *
         * The long format can have the form:
         * (d|dd) (MMMM|MM)"," (yy|yyyy)
         */
        size_t day_size;
        char *endptr;
        int valid_divider;
        day = strtol( text, &endptr, 10 );
        day_size = endptr - text;
        if( day_size ) {
            size_t month_size = 0;
            char chDivider;
            int is_short_form;

            /* skip white spaces */
            text = endptr;
            while ( isspace( *text ) )
                ++text;

            /* read month and additional dividers */
            chDivider = *text;
            valid_divider = chDivider=='-' || chDivider=='/' || chDivider=='.';
            if( chDivider=='.' ) {
                ++text;
                /* skip white spaces */
                while ( isspace( *text ) )
                    ++text;
                len = text_len - (text - text_start);
                month = xCore_hFindMonth( text, len, &end_month );
                /* We found a dot but a month name ... so this date
                 * is in LONG format. */
                is_short_form = month==0;
            } else if( valid_divider ) {
                ++text;
                is_short_form = TRUE;
            } else {
                is_short_form = FALSE;
            }
            if( is_short_form ) {
                /* short date */
                /* skip white spaces */
                while ( isspace( *text ) )
                    ++text;
                month = strtol( text, &endptr, 10 );
                month_size = endptr - text;
                if( month_size ) {
                    text = endptr;
                    /* skip white spaces */
                    while ( isspace( *text ) )
                        ++text;
                    if( *text==chDivider ) {
                        ++text;
                        result = TRUE;
                    }
                }
            } else {
                /* long date */
                len = text_len - (text - text_start);
                month = xCore_hFindMonth( text, len, &end_month );
                if( month != 0 ) {
                    text = end_month;
                    /* skip white spaces */
                    while ( isspace( *text ) )
                        ++text;
                    /* this comma is optional */
                    if( *text==',' ) {
                        ++text;
                    }
                    result = TRUE;
                }
            }
            /* read year */
            if( result ) {
                size_t year_size;
                /* skip white spaces */
                while ( isspace( *text ) )
                    ++text;
                year = strtol( text, &endptr, 10 );
                year_size = endptr - text;
                if( year_size > 0 ) {
                    /* adjust short form according to the date format */
                    if( is_short_form ) {
                        int tmp_day;
                        InlineSelect( tmp_day, order_day, day, month, year );
                        int tmp_month;
                        InlineSelect( tmp_month, order_month, day, month, year );
                        int tmp_year;
                        InlineSelect( tmp_year, order_year, day, month, year );
                        InlineSelect( year_size, order_year, day_size, month_size, year_size );
                        day = tmp_day;
                        month = tmp_month;
                        year = tmp_year;
                        if( day < 1 || month < 1 || month > 12 )
                            result = FALSE;
                    }

                    if( result ) {
                        if( year_size==2 )
                            year += 1900;
                        result = day <= xCore_DaysInMonth( year, month );
                    }
                    text = endptr;
                } else {
                    result = FALSE;
                }
            }
        }
    }

    if( result ) {
        /* Update used length */
        length = text - text_start;
    }

    if( result ) {
        if( pDay )
            *pDay = day;
        if( pMonth )
            *pMonth = month;
        if( pYear )
            *pYear = year;
        if( pLength )
            *pLength = length;
    }

    return result;
}

int xCore_hTimeParse( const char *text, size_t text_len, int *pHour, int *pMinute, int *pSecond, size_t *pLength )
{
    size_t length = 0;
    const char *text_start = text;
    int am_pm = 0;
    int result = FALSE;
    int hour = 0, minute = 0, second = 0;
    char *endptr;

    hour = strtol( text, &endptr, 10 );
    if( hour >= 0 && hour < 24 && endptr!=text) {
        int is_ampm_hour = ( hour >= 1 && hour <= 12 );
        /* skip white spaces */
        text = endptr;
        while ( isspace( *text ) )
            ++text;
        if( *text==':' ) {
            ++text;
            minute = strtol( text, &endptr, 10 );
            if( minute >= 0 && minute < 60 && endptr!=text ) {
                text = endptr;
                while ( isspace( *text ) )
                    ++text;

                result = TRUE;
                if( *text==':' ) {
                    ++text;
                    second = strtol( text, &endptr, 10 );
                    if( endptr!=text ) {
                        if( second < 0 || second > 59 ) {
                            result = FALSE;
                        } else {
                            text = endptr;
                        }
                    } else {
                        result = FALSE;
                    }
                }
                if( result && is_ampm_hour ) {
                    am_pm = xCore_hCheckAMPM( text, &length );
                    if( am_pm ) {
                        text += length;
                    }
                }
            }
        } else if( is_ampm_hour ) {
            am_pm = xCore_hCheckAMPM( text, &length );
            if( am_pm ) {
                text += length;
                result = TRUE;
            }
        }
    }
    if( result ) {
        if( am_pm ) {
            /* test for AM/PM */
            if( hour==12 ) {
                if( am_pm==1 )
                    hour -= 12;
            } else {
                if( am_pm==2 )
                    hour += 12;
            }
        }
        /* Update used length */
        length = text - text_start;
    }

    if( result ) {
        if( pHour )
            *pHour = hour;
        if( pMinute )
            *pMinute = minute;
        if( pSecond )
            *pSecond = second;
        if( pLength )
            *pLength = length;
    }

    return result;
}

int xCore_DateTimeParse( char* s,
                             int *pDay, int *pMonth, int *pYear,
                             int *pHour, int *pMinute, int *pSecond,
                             int want_date, int want_time)
{
	
    const char *text;
    int result = FALSE;
    size_t length, text_len;

    text = s;
    text_len = strlen( s );

	if( text == NULL ) {
		return result;
	}

    if( xCore_hDateParse( text, text_len, pDay, pMonth, pYear, &length ) ) {
        text += length;
        text_len -= length;

        /* skip WS */
        while( isspace( *text ) )
            ++text, --text_len;
        /* skip optional comma */
        if( *text==',' )
            ++text, --text_len;

        if( xCore_hTimeParse( text, text_len, pHour, pMinute, pSecond, &length ) ) {
            text += length;
            text_len -= length;
            result = TRUE;
        } else if( !want_time ) {
            result = TRUE;
        }
    } else if( xCore_hTimeParse( text, text_len, pHour, pMinute, pSecond, &length ) ) {
        text += length;
        text_len -= length;

        /* skip WS */
        while( isspace( *text ) )
            ++text, --text_len;
        /* skip optional comma */
        if( *text==',' )
            ++text, --text_len;

        if( xCore_hDateParse( text, text_len, pDay, pMonth, pYear, &length ) ) {
            text += length;
            text_len -= length;
            result = TRUE;
        } else if( !want_date ) {
            result = TRUE;
        }
    }

    if( result ) {
        /* the rest of the text must consist of white spaces */
        while( *text ) {
            if( !isspace( *text++ ) ) {
                result = FALSE;
                break;
            }
        }
    }

    return result;
}

int xCore_DateParse( char* s, int *pDay, int *pMonth, int *pYear )
{
    return xCore_DateTimeParse( s,
                             pDay, pMonth, pYear,
                             NULL, NULL, NULL,
                             TRUE, FALSE );
}

int xCore_TimeParse( char* s, int *pHour, int *pMinute, int *pSecond )
{
    return xCore_DateTimeParse( s, NULL, NULL, NULL, pHour, pMinute, pSecond, FALSE, TRUE );
}

int xCore_hGetDayOfYearEx( int year, int month, int day )
{
    int result = 0;
    int cur_month;
    for( cur_month=1; cur_month!=month; ++cur_month )
        result += xCore_DaysInMonth( year, cur_month );
    return result + day;
}

int xCore_hGetDayOfYear( double serial )
{
    int year, month, day;
    xCore_hDateDecodeSerial( serial, &year, &month, &day );
    return xCore_hGetDayOfYearEx( year, month, day );
}

int xCore_hGetWeekOfYear( int ref_year, double serial,
                       int first_day_of_year, int first_day_of_week )
{
    int sign;
    int year, week;
    double serial_first_week;

    xCore_hDateDecodeSerial( serial, &year, NULL, NULL );

    serial_first_week =
        xCore_hGetFirstWeekOfYear( ref_year,
                                first_day_of_year, first_day_of_week );

    serial = floor( serial - serial_first_week);
    sign = fb_hSign( serial );
    serial /= 7.0l;
    week = (int) (serial + sign);

    return week;
}

int xCore_hGetWeeksOfYear( int ref_year, int first_day_of_year, int first_day_of_week )
{
    double serial_start =
        xCore_hGetFirstWeekOfYear( ref_year,
                                first_day_of_year, first_day_of_week );
    double serial_end =
        xCore_hGetFirstWeekOfYear( ref_year + 1,
                                first_day_of_year, first_day_of_week );
    return (int) ((serial_end - serial_start) / 7.0l);
}
