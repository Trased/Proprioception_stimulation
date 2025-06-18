/***************************************************************************//**
 * @file main.cpp
 * @brief main() function.
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/
#include "sl_component_catalog.h"
#include "sl_system_init.h"

#include "sl_i2cspm.h"
#include "sl_sleeptimer.h"
#include "app.h"
#include "driver.h"


#if defined(SL_CATALOG_POWER_MANAGER_PRESENT)
#include "sl_power_manager.h"
#endif
#if defined(SL_CATALOG_KERNEL_PRESENT)
#include "sl_system_kernel.h"
#else // SL_CATALOG_KERNEL_PRESENT
#include "sl_system_process_action.h"
#endif // SL_CATALOG_KERNEL_PRESENT

extern sl_i2cspm_t sl_i2cspm_mikroe;

int main(void)
{
  // 1) Board & peripheral init
  sl_system_init();               // clocks, board, DCDC, etc.
  // sl_i2cspm_init_instances();  // if your project didn’t auto‐init I2CSPM

  // 2) DA7280 driver setup
  Haptic_Driver haptic;
  if (!haptic.begin(sl_i2cspm_mikroe)) {
      while(1);
  }

  // 3) Configure motor
  hapticSettings motorSettings;
  motorSettings.motorType = LRA_TYPE;
  motorSettings.nomVolt = 1.4;
  motorSettings.absVolt = 1.45;
  motorSettings.currMax = 213;
  motorSettings.impedance = 8.0;
  motorSettings.lraFreq = 80;

  if(!haptic.setMotorSettings(motorSettings))
  {
    while (1);
  }

  // 4) Main loop: on 0.5 s, off 0.5 s
  while (1) {
    haptic.setVibrate(70);    // 70% power :contentReference[oaicite:15]{index=15}
    sl_sleeptimer_delay_millisecond(500);

    haptic.setVibrate(0);        // off
    sl_sleeptimer_delay_millisecond(500);
  }
}
