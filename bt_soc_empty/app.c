/***************************************************************************
 * @file
 * @brief Core application logic.
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/
#include <stdlib.h>
#include <errno.h>

#include "sl_bt_api.h"
#include "sl_main_init.h"
#include "app_assert.h"
#include "app.h"
#include "gatt_db.h"

#include "sl_sleeptimer.h"
#include "sl_i2cspm.h"
#include "sl_i2cspm_instances.h"
#include "da7280_driver.h"

#define MSG_MAX_LEN 128
// The advertising set handle allocated from Bluetooth stack.
static uint8_t advertising_set_handle = 0xff;
static uint8_t _conn_handle = 0xff;

// Application Init.
void app_init(void)
{
    do
    {
        da7280_setBootStatus(BOOT_IN_PROGRESS);

        hapticSettings motorSettings;
        motorSettings.motorType = LRA_TYPE;
        motorSettings.nomVolt = 1.4;
        motorSettings.absVolt = 1.45;
        motorSettings.currMax = 213;
        motorSettings.impedance = 8.0;
        motorSettings.lraFreq = 80;

        if (!da7280_begin(sl_i2cspm_da7280) ||
            !da7280_setMotorSettings(motorSettings) ||
            !da7280_setOperationMode(DRO_MODE))
        {
            da7280_setBootStatus(BOOT_FAILED);
            break;
        }

        da7280_setBootStatus(BOOT_COMPLETED);
        da7280_enableFreqTrack(true);
        da7280_enableAcceleration(true);
        da7280_enableRapidStop(true);
        da7280_setVibrate(0);
    } while (0);
}

// Application Process Action.
void app_process_action(void)
{
    if (app_is_process_required())
    {
        if (da7280_getActivityDone())
        {
            const char msg[] = "Activity time ended. Please enter the specifications again!";
            sl_status_t sc = sl_bt_gatt_server_send_notification(
                _conn_handle,
                gattdb_message_response,
                sizeof(msg) - 1,
                (const uint8_t *)msg);
            app_assert_status(sc);

            da7280_setActivityDone(false);
        }
        /////////////////////////////////////////////////////////////////////////////
        // Put your additional application code here!                              //
        // This is will run each time app_proceed() is called.                     //
        // Do not call blocking functions from here!                               //
        /////////////////////////////////////////////////////////////////////////////
    }
}

/**************************************************************************
 * Bluetooth stack event handler.
 * This overrides the default weak implementation.
 *
 * @param[in] evt Event coming from the Bluetooth stack.
 *****************************************************************************/
void sl_bt_on_event(sl_bt_msg_t *evt)
{
    sl_status_t sc;

    switch (SL_BT_MSG_ID(evt->header))
    {
    // -------------------------------
    // This event indicates the device has started and the radio is ready.
    // Do not call any stack command before receiving this boot event!
    case sl_bt_evt_system_boot_id:
        // Create an advertising set.
        sc = sl_bt_advertiser_create_set(&advertising_set_handle);
        app_assert_status(sc);

        // Generate data for advertising
        sc = sl_bt_legacy_advertiser_generate_data(
            advertising_set_handle, sl_bt_advertiser_general_discoverable);
        app_assert_status(sc);

        // Set advertising interval to 100ms.
        sc = sl_bt_advertiser_set_timing(advertising_set_handle, 160, // min. adv. interval (milliseconds * 1.6)
                                         160,                         // max. adv. interval (milliseconds * 1.6)
                                         0,                           // adv. duration
                                         0);                          // max. num. adv. events
        app_assert_status(sc);
        // Start advertising and enable connections.
        sc = sl_bt_legacy_advertiser_start(advertising_set_handle,
                                           sl_bt_legacy_advertiser_connectable);
        app_assert_status(sc);
        break;
    case sl_bt_evt_connection_opened_id:
        _conn_handle = evt->data.evt_connection_opened.connection;
        break;
    case sl_bt_evt_gatt_server_user_write_request_id:
        const sl_bt_evt_gatt_server_user_write_request_t *wr = &evt->data.evt_gatt_server_user_write_request;
        if (wr->characteristic == gattdb_weight_value ||
            wr->characteristic == gattdb_pattern_value ||
            wr->characteristic == gattdb_activity_value)
        {
            char buf[16] = {0};
            bool parse_ok = true;
            size_t len = wr->value.len;

            if (len > sizeof(buf) - 1)
            {
                len = sizeof(buf) - 1;
            }

            memcpy(buf, wr->value.data, len);

            char *endptr = NULL;
            errno = 0;
            unsigned long val = strtoul(buf, &endptr, 10);

            if (endptr == buf       // no digits
                || *endptr != '\0'  // junk after number
                || errno == ERANGE  // out of range
                || val > UINT8_MAX) // bigger than we can store
            {
                parse_ok = false;
            }

            if (!parse_ok)
            {
                const char msg[] = "Invalid value received";
                sl_status_t sc;

                sc = sl_bt_gatt_server_send_notification(
                    wr->connection,
                    gattdb_message_response,
                    sizeof(msg) - 1,
                    (const uint8_t *)msg);
                app_assert_status(sc);

                sc = sl_bt_gatt_server_send_user_write_response(
                    wr->connection,
                    wr->characteristic,
                    SL_STATUS_OK);
                app_assert_status(sc);
            }
            else
            {
                char msg[MSG_MAX_LEN];
                da7280_processUserInput(val, wr->characteristic, msg, MSG_MAX_LEN);

                sl_status_t sc;

                sc = sl_bt_gatt_server_send_notification(
                    wr->connection,
                    gattdb_message_response,
                    strlen(msg),
                    (const uint8_t *)msg);
                app_assert_status(sc);

                sc = sl_bt_gatt_server_send_user_write_response(
                    wr->connection,
                    wr->characteristic,
                    SL_STATUS_OK);
                app_assert_status(sc);
            }
        }
        break;
        // -------------------------------
        // This event indicates that a connection was closed.
    case sl_bt_evt_connection_closed_id:
        _conn_handle = 0xff;
        // Generate data for advertising
        sc = sl_bt_legacy_advertiser_generate_data(
            advertising_set_handle, sl_bt_advertiser_general_discoverable);
        app_assert_status(sc);

        // Restart advertising after client has disconnected.
        sc = sl_bt_legacy_advertiser_start(advertising_set_handle,
                                           sl_bt_legacy_advertiser_connectable);
        app_assert_status(sc);
        break;

        ///////////////////////////////////////////////////////////////////////////
        // Add additional event handlers here as your application requires!      //
        ///////////////////////////////////////////////////////////////////////////

        // -------------------------------
        // Default event handler.
    default:
        break;
    }
}
