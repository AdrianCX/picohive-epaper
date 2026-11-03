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

#include "image_save.h"
#include "image_slot.h"
#include "pico_logger.h"

bool ImageSave::on_header(CommandContext& ctx)
{
    m_slot = image_slot(ctx);

    if (m_slot < 0)
    {
        return ctx.reply_fmt(104, "error: expected image/save/<0-%d>, got [%.*s]", EpaperHandler::MAX_SLOT, (int)ctx.args.size(), ctx.args.data());
    }

    EpaperHandler::getInstance().clear_image();
    return true;
}

bool ImageSave::on_chunk(CommandContext& ctx, const uint8_t *data, uint32_t len, bool last_chunk)
{
    if (m_slot < 0)
    {
        return true;
    }

    EpaperHandler& epaper = EpaperHandler::getInstance();
    epaper.store_image_data(data, len);

    if (!last_chunk)
    {
        return true;
    }

    return ctx.reply_fmt(32, epaper.save_picture(m_slot) ? "saved in slot %d" : "error: saving slot %d failed", (int)m_slot);
}

void ImageSave::on_abort(CommandContext&, const char *reason)
{
    trace("ImageSave: abandoned (%s)", reason);
    EpaperHandler::getInstance().clear_image();
}

WS_COMMAND(ImageSave);
