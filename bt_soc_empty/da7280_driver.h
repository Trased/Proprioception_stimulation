#ifndef DRIVER_DA7280_H
#define DRIVER_DA7280_H

#include "sl_i2cspm.h"

#include <stdint.h>   // for uint8_t, uint32_t
#include <stddef.h>   // for size_t
#include <string.h>   // for memcpy

typedef enum
{
    DEF_ADDR                = 0x4A,
    CHIP_REV                = 0xBA,
    ENABLE                  = 0x01,
    UNLOCKED                = 0x01,
    DISABLE                 = 0x00,
    LOCKED                  = 0x00,
    LRA_TYPE                = 0x00,
    ERM_TYPE                = 0x01,
    RAMP                    = 0x01,
    STEP                    = 0x00
}GLOBAL_VARS;

typedef enum
{
    BOOT_IDLE               = 0x00,
    BOOT_IN_PROGRESS        = BOOT_IDLE + 0x00,
    BOOT_FAILED             = BOOT_IDLE + 0x01,
    BOOT_COMPLETED          = BOOT_IDLE + 0x02
}BOOT_STATUS;

typedef enum
{
    IDLE                    = 0x00,
    WEIGHT_INPUT_RECEIVED   = IDLE + 0x01,
    PATTERN_INPUT_RECEIVED  = IDLE + 0x02,
    ACTIVITY_TIME_RECEIVED  = IDLE + 0x03,
    ACTIVE                  = IDLE + 0x04
}STATE_MACHINE;

typedef struct
{
    uint8_t motorType;
    float nomVolt;
    float absVolt;
    float currMax;
    float impedance;
    float lraFreq;
}hapticSettings;

typedef enum
{
    HAPTIC_SUCCESS,
    E_SEQ_CONTINUE = 0x01,
    E_UVLO = 0x02,
    HAPTIC_HW_ERROR,
    E_SEQ_DONE = 0x04,
    HAPTIC_INCORR_PARAM,
    HAPTIC_UNKNOWN_ERROR,
    E_OVERTEMP_CRIT = 0x08,
    E_SEQ_FAULT = 0x10,
    E_WARNING = 0x20,
    E_ACTUATOR_FAULT = 0x40,
    E_OC_FAULT = 0x80
} event_t;

typedef enum
{
    NO_DIAG                   = 0x00,
    E_PWM_FAULT               = 0x20,
    E_MEM_FAULT               = 0x40,
    E_SEQ_ID_FAULT            = 0x80
} diag_status_t;

typedef enum
{
    STATUS_NOM                = 0x00,
    STA_SEQ_CONTINUE          = 0x01,
    STA_UVLO_VBAT_OK          = 0x02,
    STA_PAT_DONE              = 0x04,
    STA_OVERTEMP_CRIT         = 0x08,
    STA_PAT_FAULT             = 0x10,
    STA_WARNING               = 0x20,
    STA_ACTUATOR              = 0x40,
    STA_OC                    = 0x80
} status_t;

typedef enum
{
    INACTIVE = 0x00,
    DRO_MODE,
    PWM_MODE,
    RTWM_MODE,
    ETWM_MODE
} OPERATION_MODES;

typedef enum
{
    BEGIN_SNP_MEM             = 0x00,
    NUM_SNIPPETS              = 0x00,
    NUM_SEQUENCES             = 0x01,
    ENDPOINTERS               = 0x02,
    SNP_ENDPOINTERS           = 0x02,
    TOTAL_MEM_REGISTERS       = 0x64,
} SNPMEM_ARRAY_POS;

typedef enum
{
    NUM_SNIPPETS_REG          = 0x84,
    NUM_SEQUENCES_REG         = 0x85,
    SNP_ENDPOINTERS_REGS      = 0x88,
    END_OF_MEM                = 0xE7
} SNPMEM_REGS;

typedef enum
{
    CHIP_REV_REG = 0x00,
    IRQ_EVENT1 = 0x03,
    IRQ_EVENT_WARN_DIAG,
    IRQ_EVENT_SEQ_DIAG,
    IRQ_STATUS1,
    IRQ_MASK1,
    CIF_I2C1,
    FRQ_LRA_PER_H = 0x0A,
    FRQ_LRA_PER_L,
    ACTUATOR1,
    ACTUATOR2,
    ACTUATOR3,
    CALIB_V2I_H,
    CALIB_V2I_L = 0x10,
    CALIB_IMP_H,
    CALIB_IMP_L,
    TOP_CFG1,
    TOP_CFG2,
    TOP_CFG3,
    TOP_CFG4,
    TOP_INT_CFG1,
    TOP_INT_CFG6_H = 0x1C,
    TOP_INT_CFG6_L,
    TOP_INT_CFG7_H,
    TOP_INT_CFG7_L,
    TOP_INT_CFG8 = 0x20,
    TOP_CTL1 = 0x22,
    TOP_CTL2,
    SEQ_CTL1,
    SWG_C1,
    SWG_C2,
    SWG_C3,
    SEQ_CTL2,
    GPI_0_CTL,
    GPI_1_CTL,
    GPI_2_CTL,
    MEM_CTL1,
    MEM_CTL2,
    ADC_DATA_H1,
    ADC_DATA_L1,
    POLARITY = 0x43,
    LRA_AVR_H,
    LRA_AVR_L,
    FRQ_LRA_PER_ACT_H,
    FRQ_LRA_PER_ACT_L,
    FRQ_PHASE_H,
    FRQ_PHASE_L,
    FRQ_CTL = 0x4C,
    TRIM3 = 0x5F,
    TRIM4,
    TRIM6 = 0x62,
    TOP_CFG5 = 0x6E,
    IRQ_EVENT_ACTUATOR_FAULT = 0x81,
    IRQ_STATUS2,
    IRQ_MASK2,
    SNP_MEM_X
} REGISTERS;

void da7280_setActivityDone(bool status);
bool da7280_getActivityDone();
bool da7280_isActivityTimeSet();
void da7280_performActivity();
void da7280_resetStateMachine();
void da7280_setBootStatus(BOOT_STATUS status);
uint8_t da7280_processUserInput(uint32_t event_value, uint16_t characteristic, char *out_buf, size_t out_buf_len);
bool da7280_begin(sl_i2cspm_t *i2c_port);
bool da7280_setActuatorType(uint8_t type);
bool da7280_setMotorSettings(hapticSettings settings);
hapticSettings da7280_getMotorSettings(void);
bool da7280_setOperationMode(OPERATION_MODES opMode);
uint8_t da7280_getOperationMode(void);
bool da7280_setActuatorABSVolt(float);
float da7280_getActuatorABSVolt();
bool da7280_setActuatorNOMVolt(float);
float da7280_getActuatorNOMVolt();
bool da7280_setActuatorIMAX(float);
uint16_t da7280_getActuatorIMAX();
bool da7280_setActuatorImpedance(float);
uint16_t da7280_getActuatorImpedance();
uint16_t da7280_readImpAdjus();
bool da7280_setActuatorLRAfreq(float);
bool da7280_enableCoinERM();
bool da7280_enableAcceleration(bool);
bool da7280_enableRapidStop(bool);
bool da7280_enableAmpPid(bool);
bool da7280_enableFreqTrack(bool);
bool da7280_setBemfFaultLimit(bool);
bool da7280_enableV2iFactorFreeze(bool);
bool da7280_calibrateImpedanceDistance(bool);
bool da7280_setVibrate(uint8_t);
uint8_t da7280_getVibrate();
bool da7280_setFullBrake(uint8_t);
float da7280_getFullBrake();
bool da7280_setMask(uint8_t);
uint8_t da7280_getMask();
bool da7280_setBemf(uint8_t val);
float da7280_getBemf();
void da7280_clearIrq(uint8_t);
void da7280_eraseWaveformMemory(void);
event_t da7280_getIrqEvent();
diag_status_t da7280_getEventDiag();
status_t da7280_getIrqStatus();
bool da7280_playFromMemory(bool enable);
bool da7280_setSeqControl(uint8_t, uint8_t);
uint8_t da7280_addFrame(uint8_t, uint8_t, uint8_t);

#endif // DRIVER_DA7280_H
