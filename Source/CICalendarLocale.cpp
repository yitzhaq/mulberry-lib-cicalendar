/*
    Copyright (c) 2007 Cyrus Daboo. All rights reserved.
    
    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at
    
        http://www.apache.org/licenses/LICENSE-2.0
    
    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

/*
	CICalendarLocale.cpp

	Author:
	Description:	<describe the CICalendarLocale class here>
*/

#include "CICalendarLocale.h"

#if defined(__MULBERRY) || defined(__MULBERRY_CONFIGURE)
#include "CXStringResources.h"
#else
#include "CStringResources.h"
#endif

#include <cctype>
#include <cstring>

#if __dest_os == __linux_os
#include <langinfo.h>
#endif

using namespace iCal;

bool CICalendarLocale::s24HourTime = false;
bool CICalendarLocale::sDDMMDate = false;

// 0..6 - Sunday - Saturday
const cdstring& CICalendarLocale::GetDay(int32_t day, EStringLength strl)
{
	const char* cLongDays[] = { "Day::Sunday::Long", "Day::Monday::Long", "Day::Tuesday::Long", "Day::Wednesday::Long", "Day::Thursday::Long", "Day::Friday::Long", "Day::Saturday::Long" };
	const char* cShortDays[] = { "Day::Sunday::Short", "Day::Monday::Short", "Day::Tuesday::Short", "Day::Wednesday::Short", "Day::Thursday::Short", "Day::Friday::Short", "Day::Saturday::Short" };
	const char* cAbbrevDays[] = { "Day::Sunday::Abbrev", "Day::Monday::Abbrev", "Day::Tuesday::Abbrev", "Day::Wednesday::Abbrev", "Day::Thursday::Abbrev", "Day::Friday::Abbrev", "Day::Saturday::Abbrev" };
	const char** cDays = NULL;

	switch(strl)
	{
	case eLong:
		cDays = cLongDays;
		break;
	case eShort:
		cDays = cShortDays;
		break;
	case eAbbreviated:
		cDays = cAbbrevDays;
		break;
	}
	return rsrc::GetString(cDays[day]);
}

// 1..12 - January - December
const cdstring& CICalendarLocale::GetMonth(int32_t month, EStringLength strl)
{
	const char* cLongMonths[] = { "", "Month::January::Long", "Month::February::Long", "Month::March::Long", "Month::April::Long", "Month::May::Long", "Month::June::Long", "Month::July::Long", "Month::August::Long", "Month::September::Long", "Month::October::Long", "Month::November::Long", "Month::December::Long" };
	const char* cShortMonths[] = { "", "Month::January::Short", "Month::February::Short", "Month::March::Short", "Month::April::Short", "Month::May::Short", "Month::June::Short", "Month::July::Short", "Month::August::Short", "Month::September::Short", "Month::October::Short", "Month::November::Short", "Month::December::Short" };
	const char* cAbbrevMonths[] = { "", "Month::January::Abbrev", "Month::February::Abbrev", "Month::March::Abbrev", "Month::April::Abbrev", "Month::May::Abbrev", "Month::June::Abbrev", "Month::July::Abbrev", "Month::August::Abbrev", "Month::September::Abbrev", "Month::October::Abbrev", "Month::November::Abbrev", "Month::December::Abbrev" };
	const char** cMonths = NULL;

	switch(strl)
	{
	case eLong:
		cMonths = cLongMonths;
		break;
	case eShort:
		cMonths = cShortMonths;
		break;
	case eAbbreviated:
		cMonths = cAbbrevMonths;
		break;
	}
	return rsrc::GetString(cMonths[month]);
}

// Use 24 hour time display
bool CICalendarLocale::Use24HourTime()
{
	static bool _init_done = false;
	
	if (!_init_done)
	{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
		// Get current locale
		CFLocaleRef locale = ::CFLocaleCopyCurrent();
		if (locale != NULL)
		{
			// Get formatter for current locale
			CFDateFormatterRef formatter = ::CFDateFormatterCreate(kCFAllocatorDefault, locale, kCFDateFormatterLongStyle, kCFDateFormatterLongStyle);
			if (formatter != NULL)
			{
				CFStringRef format = ::CFDateFormatterGetFormat(formatter);
				if (format != NULL)
				{
					// Look for specific characters in string
					CFRange found = ::CFStringFind(format, CFSTR("H"), 0);
					if (found.location == kCFNotFound)
					{
						found = ::CFStringFind(format, CFSTR("K"), 0);
					}
					s24HourTime = (found.location != kCFNotFound);
				}
				
				::CFRelease(formatter);
			}
			::CFRelease(locale);
		}
#elif __dest_os == __win32_os
		TCHAR buffer[256];
		::GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_STIMEFORMAT, buffer, 256);
#ifdef _UNICODE
		s24HourTime = (::wcschr(buffer, 'H') != 0);
#endif
#else
		// Locale time format may have text before the hour (e.g. "kl. %H.%M") or a
		// day period marker with a 24 hour clock (e.g. "%OH:%OM:%OS %p") so look for
		// a 12 hour conversion rather than inspecting formatted output
		s24HourTime = (FindConversion(::nl_langinfo(T_FMT), "Ilr") == NULL);
#endif

		_init_done = true;
	}
	
	return s24HourTime;
}

// Use dd/mm order for dates
bool CICalendarLocale::UseDDMMDate()
{
	static bool _init_done_date = false;
	
	if (!_init_done_date)
	{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
		// Get current locale
		CFLocaleRef locale = ::CFLocaleCopyCurrent();
		if (locale != NULL)
		{
			// Get formatter for current locale
			CFDateFormatterRef formatter = ::CFDateFormatterCreate(kCFAllocatorDefault, locale, kCFDateFormatterShortStyle, kCFDateFormatterShortStyle);
			if (formatter != NULL)
			{
				CFStringRef format = ::CFDateFormatterGetFormat(formatter);
				if (format != NULL)
				{
					// Look for specific characters in string
					sDDMMDate = ::CFStringHasPrefix(format, CFSTR("d"));
				}
				
				::CFRelease(formatter);
			}
			::CFRelease(locale);
		}
#elif __dest_os == __win32_os
		TCHAR buffer[256];
		::GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_IDATE, buffer, 256);
#ifdef _UNICODE
		sDDMMDate = (::wcschr(buffer, '1') != 0);
#endif
#else
		// Locale date format may start with a weekday or year so compare the positions
		// of the day and month conversions (%D and %F put month before day)
		const char* format = ::nl_langinfo(D_FMT);
		const char* day = FindConversion(format, "de");
		const char* month = FindConversion(format, "mbBhDF");
		sDDMMDate = (day != NULL) && ((month == NULL) || (day < month));
#endif

		_init_done_date = true;
	}
	
	return sDDMMDate;
}

// Find first strftime conversion in format matching one of conversions
const char* CICalendarLocale::FindConversion(const char* format, const char* conversions)
{
	for(const char* p = format; *p != 0; p++)
	{
		if (*p != '%')
			continue;

		// Skip flags, field width and E/O modifiers
		p++;
		while((*p != 0) && ((::strchr("_-0^#", *p) != NULL) || ::isdigit((unsigned char)*p)))
			p++;
		if ((*p == 'E') || (*p == 'O'))
			p++;
		if (*p == 0)
			break;

		if (::strchr(conversions, *p) != NULL)
			return p;
	}

	return NULL;
}
