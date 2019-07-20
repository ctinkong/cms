#include <rfc/cms_rfc1123.h>
#include <common/cms_utility.h>

#define TIMEBUFLEN      128

static const char *days[] =
{ "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
static const char *months[] =
{ "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

static const char *month_names[12] =
{ "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
void cmsRFC1123(char *buf, time_t t)
{
	char *s = buf;
	const char *src;
	struct tm st;
	localtime_r(&t, &st);
	struct tm *gmt = &st;
	// 	struct tm *gmt = gmtime(&t);
	short y;

	/* "%a, %d %b %Y %H:%M:%S GMT" */

	/* Append day */
	for (src = days[gmt->tm_wday]; *src != '\0'; src++, s++)
	{
		*s = *src;
	}

	/* Append ", " */
	*s++ = ',';
	*s++ = ' ';

	/* Append number day (two-digit, padded 0) */
	*s++ = ((gmt->tm_mday / 10) % 10) + '0';
	*s++ = (gmt->tm_mday % 10) + '0';

	/* append space */
	*s++ = ' ';

	/* Append month abbreviation */
	for (src = months[gmt->tm_mon]; *src != '\0'; src++, s++)
	{
		*s = *src;
	}

	/* Space */
	*s++ = ' ';

	/* four-character year */
	y = 1900 + gmt->tm_year;
	*s++ = ((y / 1000) % 10) + '0';
	*s++ = ((y / 100) % 10) + '0';
	*s++ = ((y / 10) % 10) + '0';
	*s++ = (y % 10) + '0';

	/* Space */
	*s++ = ' ';

	/* Two-char hour */
	*s++ = ((gmt->tm_hour / 10) % 10) + '0';
	*s++ = (gmt->tm_hour % 10) + '0';

	/* : */
	*s++ = ':';

	/* Two-char minute */
	*s++ = ((gmt->tm_min / 10) % 10) + '0';
	*s++ = (gmt->tm_min % 10) + '0';

	/* : */
	*s++ = ':';

	/* Two char second */
	*s++ = ((gmt->tm_sec / 10) % 10) + '0';
	*s++ = (gmt->tm_sec % 10) + '0';

	/* " GMT\0" */
	*s++ = ' ';
	*s++ = 'G';
	*s++ = 'M';
	*s++ = 'T';
	*s++ = '\0';
	/* Finito! */
}

static int make_month(const char *s)
{
	int i;
	char month[3];

	month[0] = toupper(*s);
	month[1] = tolower(*(s + 1));
	month[2] = tolower(*(s + 2));

	for (i = 0; i < 12; i++)
		if (!strncmp(month_names[i], month, 3))
			return i;
	return -1;
}

static int make_num(const char *s)
{
	if (*s >= '0' && *s <= '9')
		return 10 * (*s - '0') + *(s + 1) - '0';
	else
		return *(s + 1) - '0';
}

static int tm_sane_values(struct tm *tm)
{
	if (tm->tm_sec < 0 || tm->tm_sec > 59)
		return 0;
	if (tm->tm_min < 0 || tm->tm_min > 59)
		return 0;
	if (tm->tm_hour < 0 || tm->tm_hour > 23)
		return 0;
	if (tm->tm_mday < 1 || tm->tm_mday > 31)
		return 0;
	if (tm->tm_mon < 0 || tm->tm_mon > 11)
		return 0;
	return 1;
}

static int parse_date_elements(const char *day, const char *month, const char *year,
	const char *time, const char *zone, struct tm * tm)
{
	const char *t;

	memset(tm, 0, sizeof(*tm));

	if (!day || !month || !year || !time)
		return CMS_ERROR;
	tm->tm_mday = atoi(day);
	tm->tm_mon = make_month(month);
	if (tm->tm_mon < 0)
		return CMS_ERROR;
	tm->tm_year = atoi(year);
	if (strlen(year) == 4)
		tm->tm_year -= 1900;
	else if (tm->tm_year < 70)
		tm->tm_year += 100;
	else if (tm->tm_year > 19000)
		tm->tm_year -= 19000;
	tm->tm_hour = make_num(time);
	t = strchr(time, ':');
	if (!t)
		return CMS_ERROR;
	t++;
	tm->tm_min = atoi(t);
	t = strchr(t, ':');
	if (t)
		tm->tm_sec = atoi(t + 1);
	return tm_sane_values(tm) ? CMS_OK : CMS_ERROR;
}

static int parse_date(const char *str, int len, struct tm * tm)
{
	char tmp[TIMEBUFLEN];
	char *t;
	char *wday = NULL;
	char *day = NULL;
	char *month = NULL;
	char *year = NULL;
	char *time = NULL;
	char *zone = NULL;
	int bl = cmsMin(len, TIMEBUFLEN - 1);

	memcpy(tmp, str, bl);
	tmp[bl] = '\0';

	char *saveptr;
	for (t = strtok_r(tmp, ", ", &saveptr); t; t = strtok_r(NULL, ", ", &saveptr))
	{
		if (isdigit(*t))
		{
			if (!day)
			{
				day = t;
				t = strchr(t, '-');
				if (t)
				{
					*t++ = '\0';
					month = t;
					t = strchr(t, '-');
					if (!t)
						return CMS_ERROR;
					*t++ = '\0';
					year = t;
				}
			}
			else if (strchr(t, ':'))
				time = t;
			else if (!year)
				year = t;
		}
		else if (!wday)
			wday = t;
		else if (!month)
			month = t;
		else if (!zone)
			zone = t;
	}
	return parse_date_elements(day, month, year, time, zone, tm);
}

time_t cmsParseRFC1123(const char *str, int len)
{
	struct tm tm;
	time_t t;

	if (NULL == str)
		return -1;
	int rc = parse_date(str, len, &tm);
	if (rc == CMS_ERROR)
		return -1;
	tm.tm_isdst = -1;
	t = timegm(&tm);
	return t;
}
