/* SPDX-License-Identifier: AGPL-3.0-or-later

Copyright (c) 2026 Adrian Cruceru - https://github.com/AdrianCX/picohive-firmware

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Affero General Public License as published
by the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "common_functions.h"
#include "pico_logger.h"
#include "calendar_manager.h"
#include "http_header_helper.h"
#include "calendar_util.h"
#include "epaper_config.h"
#include <string>
#include <string.h>
#include "epaper_handler.h"
#include "os_platform.h"

extern void calendar_failure();

static const uint32_t CALENDAR_RETRIES = 3;
static const uint64_t CALENDAR_RETRY_DELAY_US = 10000000ULL;

CalendarManager *CalendarManager::s_instance = NULL;

CalendarManager::CalendarManager()
    : m_client(m_host, 443, true)
{
    m_client.addWantedHeader("date");
    m_client.setHttpCallback(this);
}

CalendarManager& CalendarManager::getInstance()
{
    if (s_instance == NULL)
    {
        s_instance = new CalendarManager();
    }

    return *s_instance;
}

bool CalendarManager::fetchCalendars()
{
    if (m_state != CalendarState::INIT)
    {
        trace("CalendarManager::fetchCalendars, this=%p failed, state=%d", this, m_state);
        return false;
    }

    EpaperConfig& config = get_epaper_config();
    trace("CalendarManager::fetchCalendars, this=%p main=%d other=%d", this, config.calendar_main[0] != 0, config.calendar_other[0] != 0);

    m_demo = false;
    m_retries = 0;
    fetchMain();
    return true;
}

bool CalendarManager::fetchDemo(uint32_t slot)
{
    if (!fetchCalendars())
    {
        return false;
    }

    m_demo = true;
    m_demoSlot = slot;
    return true;
}

void CalendarManager::fetchMain()
{
    m_state = CalendarState::FETCH_MAIN;

    if (!sendRequest(get_epaper_config().calendar_main))
    {
        failed();
    }
}

void CalendarManager::failed()
{
    if (m_retries >= CALENDAR_RETRIES)
    {
        trace("CalendarManager::failed, this=%p giving up after %u retries", this, (unsigned int)m_retries);
        m_state = CalendarState::INIT;
        calendar_failure();
        return;
    }

    ++m_retries;
    trace("CalendarManager::failed, this=%p state=%d, retry %u of %u in %us", this, m_state,
          (unsigned int)m_retries, (unsigned int)CALENDAR_RETRIES, (unsigned int)(CALENDAR_RETRY_DELAY_US / 1000000));

    m_state = CalendarState::RETRY_PENDING;
    m_retry_us = time_us_64() + CALENDAR_RETRY_DELAY_US;
}

void CalendarManager::update()
{
    m_client.update();

    if (m_state == CalendarState::OTHER_PENDING)
    {
        m_state = CalendarState::FETCH_OTHER;

        if (!sendRequest(get_epaper_config().calendar_other))
        {
            failed();
        }
    }
    else if ((m_state == CalendarState::RETRY_PENDING) && (time_us_64() >= m_retry_us))
    {
        fetchMain();
    }
}

bool CalendarManager::sendRequest(char *url)
{
    char host[MAX_EPAPER_CONFIG_STR] = {};
    char path[MAX_EPAPER_CONFIG_STR] = {};

    if (!HTTPHeaderHelper::getHost(url, host, sizeof(host)) || !HTTPHeaderHelper::getPath(url, path, sizeof(path)))
    {
        trace("CalendarManager::sendRequest, this=%p failed to get host or path", this);
        return false;
    }

    if (strcmp(host, m_host) != 0)
    {
        if (m_host[0] != 0)
        {
            m_client.close();
        }

        memcpy(m_host, host, sizeof(m_host));
    }

    m_bytesReceived = 0;
    trace("CalendarManager::sendRequest, this=%p host=%s", this, m_host);

    if (!m_client.begin("GET", path) || !m_client.send())
    {
        trace("CalendarManager::sendRequest, this=%p failed to send request", this);
        return false;
    }

    return true;
}

bool CalendarManager::onHeaderReceived(HTTPHeader& header)
{
    trace("CalendarManager::onHeaderReceived, this=%p code=%d", this, header.getResponseCode());

    if (header.getResponseCode() != 200)
    {
        trace("CalendarManager::onHeaderReceived, this=%p, unexpected response code %d", this, header.getResponseCode());
        return false;
    }

    const char *date = header.getHeaderValue("date");
    if (date == NULL)
    {
        trace("CalendarManager::onHeaderReceived, this=%p, missing date header", this);
        return false;
    }

    int64_t epochUtc = httpDateToUtc(date);
    int32_t tzOffset = timezoneOffset((Timezone)get_epaper_config().timezone, epochUtc);
    int64_t epochLocalTz = epochUtc + tzOffset;
    int64_t epochStartDayUtc = getStartDayEpochUtc(epochUtc);
    int64_t epochStartDayLocalTz = epochStartDayUtc + tzOffset;

    char currentTimeUtc[64] = {};
    char currentTimeLocal[64] = {};
    char startTimeUtc[64] = {};
    char startTimeLocal[64] = {};

    formatEpoch(epochUtc, currentTimeUtc);
    formatEpoch(epochLocalTz, currentTimeLocal);

    formatEpoch(epochStartDayUtc, startTimeUtc);
    formatEpoch(epochStartDayLocalTz, startTimeLocal);

    static const int WEEK_SECONDS = 7 * 86400;
    int64_t epochEndDayUtc = epochStartDayUtc + WEEK_SECONDS - 1;

    if (m_state == CalendarState::FETCH_MAIN)
    {
        m_parser.begin(epochStartDayUtc, epochEndDayUtc, tzOffset);
    }

    trace("CalendarManager::onHeaderReceived, this=%p, tzOffset=%d, curTimeUtc=%s, curTimeLocal=%s, startTimeUtc=%s, startTimeLocal=%s", this, (int)tzOffset, currentTimeUtc, currentTimeLocal, startTimeUtc, startTimeLocal);
    return true;
}

bool CalendarManager::onHttpData(u8_t *data, size_t len)
{
    m_bytesReceived += len;
    m_parser.feed((const char *)data, len);
    return true;
}

void CalendarManager::onResponseComplete(bool ok)
{
    trace("CalendarManager::onResponseComplete, this=%p state=%d ok=%d bytes=%u events so far=%d", this, m_state, ok, (unsigned int)m_bytesReceived, (int)m_parser.count());

    if (m_state == CalendarState::FETCH_MAIN)
    {
        if (!ok)
        {
            failed();
            return;
        }

        if (get_epaper_config().calendar_other[0] != 0)
        {
            m_state = CalendarState::OTHER_PENDING;
            return;
        }

        finishCalendars();
    }
    else if (m_state == CalendarState::FETCH_OTHER)
    {
        if (!ok)
        {
            failed();
            return;
        }

        finishCalendars();
    }
}

void CalendarManager::finishCalendars()
{
    m_parser.end();
    m_state = CalendarState::INIT;

    processEvents();
}

void CalendarManager::processEvents()
{
    int32_t eventCount = m_parser.count();

    static const char* WD[] = {
        "Sunday    ",
        "Monday    ",
        "Tuesday   ",
        "Wednesday ",
        "Thursday  ",
        "Friday    ",
        "Saturday  "
    };

    static const char* Month[] = {
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
        "Dec",
    };

    std::string calendar;

    int y = 0;
    int m = 0;
    int d = 0;
    int wkday = 0;

    m_parser.getRangeStartCivil(y, m, d, wkday);

    calendar += (4 + 1);
    calendar += WD[wkday];

#if EPAPER_VARIANT == 2
    calendar += "          ";
#else
    calendar += "        ";
#endif

    calendar += (0 + 1);
    calendar +='0'+(d/10)%10;
    calendar +='0'+(d%10);
    calendar +=' ';
    calendar += Month[m-1];
    calendar +=' ';
    calendar += '2';
    calendar += '0';
    calendar +='0'+(y/10)%10;
    calendar +='0'+(y%10);

    calendar += '\n';

    int lastDayPrinted = wkday;

    for (int i=0;i<eventCount;++i)
    {
        const IcalEvent& ev = m_parser.at(i);

        if (ev.weekday != lastDayPrinted)
        {
            calendar += '\n';

            calendar += (4 + 1);
            calendar += WD[ev.weekday];

            calendar += '\n';

            lastDayPrinted = ev.weekday;
        }

        calendar += (0 + 1);

        char temp[7];
        temp[0]='0'+(ev.hour/10)%10;
        temp[1]='0'+(ev.hour%10);
        temp[2]=':';
        temp[3]='0'+(ev.minute/10)%10;
        temp[4]='0'+(ev.minute%10);
        temp[5]=' ';
        temp[6]=0;

        calendar += temp;
        appendTitle(ev.title, m_demo, calendar);

        calendar += '\n';
    }

    trace("CalendarManager::processEvents() this=%p finished, numevents[%d] demo[%d] range[%04d-%02d-%02d] textlen[%u]", this, (int)eventCount, m_demo, y, m, d, (unsigned int)calendar.size());

    if (m_demo)
    {
        m_demo = false;
        EpaperHandler::getInstance().show_calendar(calendar.c_str(), m_demoSlot, true);
        return;
    }

    EpaperHandler::getInstance().show_calendar(calendar.c_str(), get_rand_32());
}

void CalendarManager::appendTitle(const char *title, bool mask, std::string& out)
{
    for (const unsigned char *c = (const unsigned char *)title; *c != 0; ++c)
    {
        if ((*c & 0xC0) == 0x80)
        {
            continue;
        }

        out += ((*c == ' ') || (!mask && (*c > ' ') && (*c < 0x7F))) ? (char)*c : '.';
    }
}
