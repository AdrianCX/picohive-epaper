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

#include <stdint.h>

#include "command_registration.h"
#include "epaper_handler.h"

inline int32_t image_slot(const CommandContext& ctx)
{
    if (ctx.args.empty())
    {
        return -1;
    }

    uint32_t slot = 0;

    for (char c : ctx.args)
    {
        if ((c < '0') || (c > '9'))
        {
            return -1;
        }

        slot = slot * 10 + (c - '0');

        if (slot > (uint32_t)EpaperHandler::MAX_SLOT)
        {
            return -1;
        }
    }

    return (int32_t)slot;
}
