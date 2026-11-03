/* SPDX-License-Identifier: AGPL-3.0-or-later

Copyright (c) 2024 Adrian Cruceru - https://github.com/AdrianCX/picohive-firmware

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

#pragma once

#include "os_platform.h"
#include "http_client.h"
#include "http_request.h"
#include "ical_parser.h"
#include "epaper_config.h"

#include <string>

class CalendarManager : public IHttpCallback
{
public:
    bool fetchCalendars();

    bool fetchDemo(uint32_t slot);

    bool busy() const { return m_state != CalendarState::INIT; }

    void update();

    static CalendarManager& getInstance();

protected:
    bool onHeaderReceived(HTTPHeader& header) override;
    bool onHttpData(u8_t *data, size_t len) override;
    void onResponseComplete(bool ok) override;

    bool sendRequest(char *url);
    void fetchMain();
    void failed();
    void finishCalendars();
    void processEvents();
    void appendTitle(const char *title, bool mask, std::string& out);

private:
    CalendarManager();

    IcalParser m_parser;

    enum CalendarState
    {
        INIT,
        FETCH_MAIN,
        OTHER_PENDING,
        FETCH_OTHER,
        RETRY_PENDING,
    };

    CalendarState m_state = CalendarState::INIT;
    uint32_t m_retries = 0;
    uint64_t m_retry_us = 0;
    uint32_t m_bytesReceived = 0;

    bool m_demo = false;
    uint32_t m_demoSlot = 0;

    char m_host[MAX_EPAPER_CONFIG_STR] = {};
    HTTPClient m_client;

    static CalendarManager *s_instance;
};
