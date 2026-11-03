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

#include "image_show.h"
#include "pico_logger.h"

bool ImageShow::on_header(CommandContext& ctx)
{
    EpaperHandler::getInstance().clear_image();
    return true;
}

bool ImageShow::on_chunk(CommandContext& ctx, const uint8_t *data, uint32_t len, bool last_chunk)
{
    EpaperHandler& epaper = EpaperHandler::getInstance();
    epaper.store_image_data(data, len);

    if (!last_chunk)
    {
        return true;
    }

    return ctx.reply(epaper.show_image() ? "showing" : "error: not shown");
}

void ImageShow::on_abort(CommandContext&, const char *reason)
{
    trace("ImageShow: abandoned (%s)", reason);
    EpaperHandler::getInstance().clear_image();
}

WS_COMMAND(ImageShow);
