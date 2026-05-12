/*
    Copyright (c) 2007-2010 Cyrus Daboo. All rights reserved.
    
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
	CICalendarManager.cpp

	Author:
	Description:	<describe the CICalendarManager class here>
*/

#include "CICalendarManager.h"

#include "CICalendar.h"
#if defined(__MULBERRY) || defined(__MULBERRY_CONFIGURE)
#include "CConnectionManager.h"
#include "CLocalCommon.h"
#include "CPluginManager.h"
#else
#include "CLocalUtils.h"
#endif

#include "diriterator.h"
#include "cdfstream.h"

#if __dest_os == __mac_os || __dest_os == __mac_os_x || __dest_os == __linux_os
#include <unistd.h>
#endif

using namespace iCal;

CICalendarManager* CICalendarManager::sICalendarManager = NULL;

CICalendarManager::CICalendarManager()
{
	sICalendarManager = this;
}

CICalendarManager::~CICalendarManager()
{
	sICalendarManager = NULL;
}

const char* cTimezonesDir = "Timezones";

void CICalendarManager::InitManager()
{
#ifdef __MULBERRY
	// Need to have timezones cached before starting any UI work as timezone popup needs them
	for(cdstrvect::const_iterator iter = CPluginManager::sPluginManager.GetPluginDirs().begin(); iter != CPluginManager::sPluginManager.GetPluginDirs().end(); iter++)
	{
		cdstring tzpath = *iter;
		::addtopath(tzpath, cTimezonesDir);
		ScanDirectoryForTimezones(tzpath);
	}
	
	// Need to have timezones cached before starting any UI work as timezone popup needs them
	{
		cdstring tzpath = CConnectionManager::sConnectionManager.GetTimezonesDirectory();
		ScanDirectoryForTimezones(tzpath);
	}
	
	// Detect system timezone for the default
	cdstring sys_tz;
#if __dest_os == __mac_os || __dest_os == __mac_os_x || __dest_os == __linux_os
	// Resolve /etc/localtime symlink to get Olson timezone name
	char linkbuf[256];
	ssize_t len = ::readlink("/etc/localtime", linkbuf, sizeof(linkbuf) - 1);
	if (len > 0)
	{
		linkbuf[len] = 0;
		// Extract timezone name after "zoneinfo/"
		const char* p = ::strstr(linkbuf, "zoneinfo/");
		if (p)
			sys_tz = p + 9;
	}
	// Fallback: read /etc/timezone (Debian/Ubuntu)
	if (sys_tz.empty())
	{
		cdifstream tzfile("/etc/timezone");
		if (tzfile.good())
		{
			getline(tzfile, sys_tz);
			sys_tz.trimspace();
		}
	}
#elif __dest_os == __win32_os
	// Win32 registry key TimeZoneKeyName gives Windows names
	// ("Eastern Standard Time") not IANA names ("America/New_York").
	// Proper conversion requires CLDR windowsZones.xml mapping table
	// or Windows 10 ICU API (ucal_getDefaultTimeZone).
	// Falls through to UTC until implemented.
#endif
	if (sys_tz.empty())
		sys_tz = "UTC";
	SetDefaultTimezone(CICalendarTimezone(false, sys_tz));
#endif
}

void CICalendarManager::ScanDirectoryForTimezones(const cdstring& dir)
{
	diriterator iter(dir, true, ".ics");
	iter.set_return_hidden_files(false);
	const char* fname = NULL;
	while(iter.next(&fname))
	{
		// Get full path
		cdstring fpath = dir;
		::addtopath(fpath, fname);
		
		// Check for directory
		if (iter.is_dir())
		{
			// Scan more
			ScanDirectoryForTimezones(fpath);
		}
		else
		{
			cdifstream fin(fpath.c_str());
			iCal::CICalendar::getSICalendar().Parse(fin);
		}
	}
}

void CICalendarManager::SetDefaultTimezoneID(const cdstring& tzid)
{
	// Check for UTC
	if (tzid == "UTC")
	{
		CICalendarTimezone temp(true);
		SetDefaultTimezone(temp);
	}
	else
	{
		CICalendarTimezone temp(false, tzid);
		SetDefaultTimezone(temp);
	}
}

cdstring CICalendarManager::GetDefaultTimezoneID() const
{
	if (mDefaultTimezone.GetUTC())
		return "UTC";
	else
		return mDefaultTimezone.GetTimezoneID();
}
