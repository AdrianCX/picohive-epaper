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

#include "command_registration.h"
#include "epaper_handler.h"

struct ImageShow
{
    static constexpr std::string_view name = "image/show";
    static constexpr std::string_view help = "image/show - put the image in the body on the display";

    static constexpr Payload payload = Payload::Streamed;
    static constexpr uint32_t max_bytes = EpaperHandler::IMAGE_SIZE;

    bool on_header(CommandContext& ctx);
    bool on_chunk(CommandContext& ctx, const uint8_t *data, uint32_t len, bool last_chunk);
    void on_abort(CommandContext& ctx, const char *reason);
};
