/***************************************************************************//**
 * @file
 * @brief Application interface.
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef APP_H
#define APP_H

#include <stdbool.h>

/**************************************************************************//**
 * Proceed with execution. (Indicate that it is required to run the application
 * process action.)
 *****************************************************************************/
void app_proceed(void);

/**************************************************************************//**
 * Check if it is required to process with execution.
 * @return true if required, false otherwise.
 *****************************************************************************/
bool app_is_process_required(void);

/**************************************************************************//**
 * Acquire access to protected variables.
 *
 * Acquire the guard to operate on the internal state variables.
 * Guard is implemented using mutexing (RTOS).
 *
 * @note Must not be used from ISR context.
 *
 * @return true if operation was successful.
 *****************************************************************************/
bool app_mutex_acquire(void);

/**************************************************************************//**
 * Finish access to protected variables.
 *
 * Release the guard to stop working on the internal state variables.
 * Guard is implemented using mutexing (RTOS).
 *
 * @note Must not be used from ISR context.
 *****************************************************************************/
void app_mutex_release(void);

/**************************************************************************//**
 * Initialize the application.
 *
 * This function initializes the application components.
 *
 * @note Must not be used from ISR context.
 *****************************************************************************/
void app_init_bt(void);

#endif // APP_H
