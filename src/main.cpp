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

#include <string.h>
#include <time.h>

#include "common_functions.h"
#include "listener.h"
#include "os_platform.h"
#include "pico_logger.h"
#include "tls_listener.h"
#include "epaper_handler.h"

#include <string>
#include "pico_sleep.h"

#include "app_config.h"
#include "epaper_config.h"
#include "wifi.h"

#include "general_config.h"
#include "screen_state_machine_color.h"
#include "screen_state_machine_rb.h"
#include "calendar_manager.h"
#include "command_request_handler.h"
#include "upgrade_helper.h"

#include "rc/index.html.gz.hex.h"
static const uint8_t *HTTP_BODY = __rc_index_html_gz;
static const unsigned int HTTP_BODY_LEN = __rc_index_html_gz_len;

enum AppState
{
    BOOT,
    SETUP_MODE_START,
    SETUP_MODE_MESSAGE_SHOW,
    CONFIG_MODE_START,
    CONFIG_MODE_MESSAGE_SHOW,
    CALENDAR_MODE_START,
    CALENDAR_MODE_RUNNING,
    CALENDAR_MODE_FAIL,
    CALENDAR_MODE_FAIL_SHOW,
};

AppState state = AppState::BOOT;

bool access_point = false;
bool wifi_complete = false;
bool boot_key_pressed = false;
bool sleep_enabled = true;
bool sleep_requested = false;

void enter_sleep();

void gpio_callback(uint gpio, uint32_t events)
{
    if (wifi_complete)
    {
        return;
    }

    if (!access_point && (gpio == KEY2))
    {
        force_restart(RESTART_REBOOT_COMMAND);
    }
}

void on_boot()
{
#if EPAPER_VARIANT == 1
    new ScreenStateMachineColor();
#elif EPAPER_VARIANT == 2
    new ScreenStateMachineRb();
#else
#error "Please define EPAPER_VARIANT"
#endif

    gpio_init(KEY0);
    gpio_set_dir(KEY0, GPIO_IN);
    gpio_pull_up(KEY0);

    gpio_init(KEY1);
    gpio_set_dir(KEY1, GPIO_IN);
    gpio_pull_up(KEY1);

    gpio_init(KEY2);
    gpio_set_dir(KEY2, GPIO_IN);
    gpio_pull_up(KEY2);
    gpio_set_irq_enabled_with_callback(KEY2, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &gpio_callback);

    if (gpio_get(KEY1) == 0)
    {
        state = AppState::CONFIG_MODE_START;

        boot_key_pressed = true;
        sleep_enabled = false;
        pico_deepsleep_clear_reason();
    }
    else if (gpio_get(KEY2) == 0)
    {
        state = AppState::SETUP_MODE_START;

        sleep_enabled = false;
        access_point = true;
        boot_key_pressed = true;
        pico_deepsleep_clear_reason();
    }
    else
    {
        state = AppState::CALENDAR_MODE_START;

        sleep_enabled = true;
    }
}

void on_wifi_failed()
{
    update_watchdog();

    enter_sleep();
}

void calendar_failure()
{
    state = AppState::CALENDAR_MODE_FAIL;
}

bool is_access_point()
{
    return (access_point || (gpio_get(KEY2) == 0));
}

bool can_restart()
{
    return !EpaperHandler::getInstance().busy();
}

bool on_wifi_ready()
{
    wifi_complete = true;

    Session::create_client_tls_config(NULL, 0);

    CommandRequestHandler::set_config(HTTP_BODY, HTTP_BODY_LEN);
    Listener *tcp_listener = new Listener();
    tcp_listener->listen(80, CommandRequestHandler::create);

    if (!is_access_point())
    {
        CommandRequestHandler::require_auth(true);

#if !defined(PICO_RP2040) || (PICO_RP2040 == 0)
        TLSListener *tls_listener = new TLSListener();
        tls_listener->listen(443, CommandRequestHandler::create);
#endif
    }

    EpaperHandler::getInstance().set_sleep_enabled(sleep_enabled);

    if (state == AppState::CALENDAR_MODE_START)
    {
        state = AppState::CALENDAR_MODE_RUNNING;

        if (!CalendarManager::getInstance().fetchCalendars())
        {
            calendar_failure();
        }
    }

    return true;
}

void on_update(uint64_t now_us)
{
    EpaperHandler::getInstance().update(now_us);

    if ((state == AppState::CALENDAR_MODE_RUNNING) || CalendarManager::getInstance().busy())
    {
        CalendarManager::getInstance().update();
    }

    if (state == AppState::SETUP_MODE_START)
    {
        state = AppState::SETUP_MODE_MESSAGE_SHOW;

        EpaperHandler::getInstance().show_calendar("Setup mode\n\nWifi: " WIFI_AP_SSID "\nPass: " WIFI_AP_PASSWORD "\n\nConnect to wifi and access:\nhttp://192.168.4.1", get_rand_32(), true);
    }
    else if (state == AppState::CONFIG_MODE_START)
    {
        state = CONFIG_MODE_MESSAGE_SHOW;

        char buffer[256] = {};
        snprintf(buffer, sizeof(buffer), "Config mode\n\nOpen URL in browser:\n\n http://%s", get_logging_id());

        EpaperHandler::getInstance().show_calendar(buffer, get_rand_32(), true);
    }
    else if (state == AppState::CALENDAR_MODE_FAIL)
    {
        state = CALENDAR_MODE_FAIL_SHOW;
        EpaperHandler::getInstance().show_calendar("Calendar Failure\n\nFailed to download calendar\nCheck if config is correct", get_rand_32(), false);
    }

    static bool should_reset = false;
    static uint64_t should_reset_time = 0;

    if (!boot_key_pressed)
    {
        if  ((gpio_get(KEY1) == 0) || (gpio_get(KEY2) == 0))
        {
            if (!should_reset)
            {
                trace("on_main_update: reset requested key1[%d] key2[%d]", gpio_get(KEY1), gpio_get(KEY2));
                should_reset = true;
                should_reset_time = now_us;
            }
        }
    }
    else if ((gpio_get(KEY2) == 1) && (gpio_get(KEY1) == 1))
    {
        boot_key_pressed = false;
    }

    if (should_reset && (now_us - should_reset_time > 1000000LL) && !UpgradeHelper::isUpgrading() && !restart_pending())
    {
        trace("on_main_update: reset triggered");
        request_restart(RESTART_REBOOT_COMMAND);
    }

    if (sleep_requested && !UpgradeHelper::isUpgrading() && !restart_pending() && !EpaperHandler::getInstance().busy())
    {
        sleep_requested = false;
        enter_sleep();
    }
}

extern "C" void general_sleep()
{
    if (sleep_enabled && !sleep_requested)
    {
        trace("general_sleep: requested");
        sleep_requested = true;
    }
}

void enter_sleep()
{
    trace("enter_sleep: Heap[%lu/%lu] Free[%lu] Stack[%lu/%lu]", (unsigned long)getUsedHeap(), (unsigned long)getTotalHeap(), (unsigned long)getFreeHeap(), (unsigned long)getMaxUsedStack(), (unsigned long)getTotalStack());

    if (!sleep_enabled)
    {
        trace("enter_sleep: sleep disabled, will not sleep");
        return;
    }

    static const uint64_t SLEEP_TIME_MS = 4 * 3600 * 1000;
    set_watchdog(0);

    if (pico_deepsleep_init(SLEEP_TIME_MS))
    {
        pico_deepsleep_set_gpio(0, KEY0, false, false);
        pico_deepsleep_set_gpio(1, KEY1, false, false);
        pico_deepsleep_set_gpio(2, KEY2, false, false);
        pico_deepsleep_execute();
    }
}

void on_config_update(const JsonView& config)
{
    load_epaper_config(config);
}

void get_app_info(AppConfig *config)
{
    config->on_boot = on_boot;
    config->access_point = is_access_point;
    config->on_wifi_failed = on_wifi_failed;
    config->on_wifi_ready = on_wifi_ready;
    config->on_update = on_update;
    config->on_config_update = on_config_update;
    config->can_restart = can_restart;
#if EPAPER_VARIANT == 1
    config->name = "build/" PICO_BOARD "/epaper_color";
#else
    config->name = "build/" PICO_BOARD "/epaper_rb";
#endif

    config->watchdog_timeout = 60000;
}
