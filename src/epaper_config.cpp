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

#include "epaper_config.h"

EpaperConfig epaper_config = {};

EpaperConfig& get_epaper_config()
{
    return epaper_config;
}

void load_epaper_config(const JsonView& config)
{
    config.get_string("$.epaper.secret_calendar", epaper_config.calendar_main, sizeof(epaper_config.calendar_main));
    config.get_string("$.epaper.secret_calendar_other", epaper_config.calendar_other, sizeof(epaper_config.calendar_other));

    if (!config.get_uint("$.epaper.timezone", epaper_config.timezone) || (epaper_config.timezone >= (uint32_t)TZ_COUNT))
    {
        epaper_config.timezone = TZ_CENTRAL_EUROPE;
    }
}
