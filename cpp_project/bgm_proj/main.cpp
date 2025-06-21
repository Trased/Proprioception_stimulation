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
#include "app.h"

#include "sl_sleeptimer.h"
#include "sl_i2cspm.h"
#include "sl_i2cspm_instances.h"
#include "driver.h"

#if defined(SL_CATALOG_POWER_MANAGER_PRESENT)
#include "sl_power_manager.h"
#endif
#if defined(SL_CATALOG_KERNEL_PRESENT)
#include "sl_system_kernel.h"
#else // SL_CATALOG_KERNEL_PRESENT
#include "sl_system_process_action.h"
#endif // SL_CATALOG_KERNEL_PRESENT

int main(void)
{
  // Initialize Silicon Labs device, system, service(s) and protocol stack(s).
  // Note that if the kernel is present, processing task(s) will be created by
  // this call.
  sl_system_init();
  sl_i2cspm_init_instances();

  do
  {
    Haptic_Driver haptics(I2C_ADDR);
    if (!haptics.begin(sl_i2cspm_da7280)) {
      // error: wrong chip at 0x4A?
      while (1);
    }
    // 3) Configure motor
    hapticSettings motorSettings;
    motorSettings.motorType = LRA_TYPE;
    motorSettings.nomVolt = 1.4;
    motorSettings.absVolt = 1.45;
    motorSettings.currMax = 213;
    motorSettings.impedance = 8.0;
    motorSettings.lraFreq = 80;

    if(!haptics.setMotorSettings(motorSettings))
    {
        break;
    }

    if(!haptics.setOperationMode(DRO_MODE))
    {
        break;
    }

    haptics.enableFreqTrack(false);
    haptics.enableAcceleration(false);
    haptics.enableRapidStop(false);
    // 4) Main loop: on 0.5 s, off 0.5 s
    haptics.setVibrate(0);
    while (1) {}
  }while(0);
  // Initialize the application. For example, create periodic timer(s) or
  // task(s) if the kernel is present.
  app_init();

#if defined(SL_CATALOG_KERNEL_PRESENT)
  // Start the kernel. Task(s) created in app_init() will start running.
  sl_system_kernel_start();
#else // SL_CATALOG_KERNEL_PRESENT
  while (1) {
    // Do not remove this call: Silicon Labs components process action routine
    // must be called from the super loop.
    sl_system_process_action();

    // Application process.
    app_process_action();

#if defined(SL_CATALOG_POWER_MANAGER_PRESENT)
    // Let the CPU go to sleep if the system allows it.
    sl_power_manager_sleep();
#endif
  }
#endif // SL_CATALOG_KERNEL_PRESENT
}
