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

#pragma once

#include "stdint.h"

#include "calendar_util.h"
#include "pico_mjson.h"

#define MAX_EPAPER_CONFIG_STR 256

struct EpaperConfig
{
    char calendar_main[MAX_EPAPER_CONFIG_STR] = {};
    char calendar_other[MAX_EPAPER_CONFIG_STR] = {};
    uint32_t timezone = TZ_CENTRAL_EUROPE;
};

EpaperConfig& get_epaper_config();

void load_epaper_config(const JsonView& config);
