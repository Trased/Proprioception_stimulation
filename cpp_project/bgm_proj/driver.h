#ifndef DRIVER_DA7280_H
#define DRIVER_DA7280_H

#include "sl_i2cspm.h"
#include <cstdint> // for size_t
#include <cstddef> // for uint8_t
#include <cstring> // for memcpy

#define DEF_ADDR 0x4A
#define CHIP_REV 0xBA
#define ENABLE 0x01
#define UNLOCKED 0x01
#define DISABLE 0x00
#define LOCKED 0x00
#define LRA_TYPE 0x00
#define ERM_TYPE 0x01
#define RAMP 0x01
#define STEP 0x00

struct hapticSettings
{
    uint8_t motorType;
    float nomVolt;
    float absVolt;
    float currMax;
    float impedance;
    float lraFreq;
};

typedef enum
{
  HAPTIC_SUCCESS,
  E_SEQ_CONTINUE            = 0x01,
  E_UVLO                    = 0x02,
  HAPTIC_HW_ERROR           = 0x03,
  E_SEQ_DONE                = 0x04,
  HAPTIC_INCORR_PARAM       = 0x05,
  HAPTIC_UNKNOWN_ERROR      = 0x06,
  E_OVERTEMP_CRIT           = 0x08,
  E_SEQ_FAULT               = 0x10,
  E_WARNING                 = 0x20,
  E_ACTUATOR_FAULT          = 0x40,
  E_OC_FAULT                = 0x80
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

enum OPERATION_MODES
{
  INACTIVE                  = 0x00,
  DRO_MODE                  = 0x01,
  PWM_MODE                  = 0x02,
  RTWM_MODE                 = 0x03,
  ETWM_MODE                 = 0x04
};

enum SNPMEM_ARRAY_POS
{
  BEGIN_SNP_MEM             = 0x00,
  NUM_SNIPPETS              = 0x00,
  NUM_SEQUENCES             = 0x01,
  ENDPOINTERS               = 0x02,
  SNP_ENDPOINTERS           = 0x02,
  TOTAL_MEM_REGISTERS       = 0x64,
};

enum SNPMEM_REGS
{
  NUM_SNIPPETS_REG          = 0x84,
  NUM_SEQUENCES_REG         = 0x85,
  SNP_ENDPOINTERS_REGS      = 0x88,
  END_OF_MEM                = 0xE7
};

enum REGISTERS
{
  CHIP_REV_REG              = 0x00,
  IRQ_EVENT1                = 0x03,
  IRQ_EVENT_WARN_DIAG       = 0x04,
  IRQ_EVENT_SEQ_DIAG        = 0x05,
  IRQ_STATUS1               = 0x06,
  IRQ_MASK1                 = 0x07,
  CIF_I2C1                  = 0x08,
  FRQ_LRA_PER_H             = 0x0A,
  FRQ_LRA_PER_L             = 0x0B,
  ACTUATOR1                 = 0x0C,
  ACTUATOR2                 = 0x0D,
  ACTUATOR3                 = 0x0E,
  CALIB_V2I_H               = 0x0F,
  CALIB_V2I_L               = 0x10,
  CALIB_IMP_H               = 0x11,
  CALIB_IMP_L               = 0x12,
  TOP_CFG1                  = 0x13,
  TOP_CFG2                  = 0x14,
  TOP_CFG3                  = 0x15,
  TOP_CFG4                  = 0x16,
  TOP_INT_CFG1              = 0x17,
  TOP_INT_CFG6_H            = 0x1C,
  TOP_INT_CFG6_L            = 0x1D,
  TOP_INT_CFG7_H            = 0x1E,
  TOP_INT_CFG7_L            = 0x1F,
  TOP_INT_CFG8              = 0x20,
  TOP_CTL1                  = 0x22,
  TOP_CTL2                  = 0x23,
  SEQ_CTL1                  = 0x24,
  SWG_C1                    = 0x25,
  SWG_C2                    = 0x26,
  SWG_C3                    = 0x27,
  SEQ_CTL2                  = 0x28,
  GPI_0_CTL                 = 0x29,
  GPI_1_CTL                 = 0x2A,
  GPI_2_CTL                 = 0x2B,
  MEM_CTL1                  = 0x2C,
  MEM_CTL2                  = 0x2D,
  ADC_DATA_H1               = 0x2E,
  ADC_DATA_L1               = 0x2F,
  POLARITY                  = 0x43,
  LRA_AVR_H                 = 0x44,
  LRA_AVR_L                 = 0x45,
  FRQ_LRA_PER_ACT_H         = 0x46,
  FRQ_LRA_PER_ACT_L         = 0x47,
  FRQ_PHASE_H               = 0x48,
  FRQ_PHASE_L               = 0x49,
  FRQ_CTL                   = 0x4C,
  TRIM3                     = 0x5F,
  TRIM4                     = 0x60,
  TRIM6                     = 0x62,
  TOP_CFG5                  = 0x6E,
  IRQ_EVENT_ACTUATOR_FAULT  = 0x81,
  IRQ_STATUS2               = 0x82,
  IRQ_MASK2                 = 0x83,
  SNP_MEM_X                 = 0x84
};


// DA7280 I2C address
static constexpr uint8_t I2C_ADDR = 0x4A;

class Haptic_Driver
{
public:
  uint8_t snpMemCopy[100]{};
  uint8_t lastPosWritten = 0;

  Haptic_Driver(uint8_t address = I2C_ADDR);

  bool begin(sl_i2cspm_t &i2c_port);

  bool setActuatorType(uint8_t type);

  bool setMotorSettings(hapticSettings settings);
  hapticSettings getMotorSettings(void);

  bool setOperationMode(OPERATION_MODES opMode = DRO_MODE);
  uint8_t getOperationMode(void);

  bool setActuatorABSVolt(float);
  float getActuatorABSVolt();

  bool setActuatorNOMVolt(float);
  float getActuatorNOMVolt();

  bool setActuatorIMAX(float);
  uint16_t getActuatorIMAX();

  bool setActuatorImpedance(float);
  uint16_t getActuatorImpedance();

  uint16_t readImpAdjus();

  bool setActuatorLRAfreq(float);
  bool enableCoinERM();
  bool enableAcceleration(bool);
  bool enableRapidStop(bool);
  bool enableAmpPid(bool);
  bool enableFreqTrack(bool);
  bool setBemfFaultLimit(bool);
  bool enableV2iFactorFreeze(bool);
  bool calibrateImpedanceDistance(bool);

  bool setVibrate(uint8_t);
  uint8_t getVibrate();

  bool setFullBrake(uint8_t);
  float getFullBrake();

  bool setMask(uint8_t);
  uint8_t getMask();

  bool setBemf(uint8_t val);
  float getBemf();

  void clearIrq(uint8_t);
  void eraseWaveformMemory(void);
  event_t getIrqEvent();
  diag_status_t getEventDiag();
  status_t getIrqStatus();
  bool playFromMemory(bool enable = true);
  bool setSeqControl(uint8_t, uint8_t);
  uint8_t addFrame(uint8_t, uint8_t, uint8_t);

  hapticSettings sparkSettings;

private:
  uint8_t _address;

  bool _writeRegister(uint8_t, uint8_t, uint8_t, uint8_t);

  bool _writeConsReg(uint8_t regs[], size_t);

  bool _writeNonConsReg(uint8_t regs[], size_t);

  void _writeCommand(uint8_t);

  bool _writeWaveFormMemory(uint8_t waveFormArray[]);

  uint8_t _readRegister(uint8_t);

  bool _readConsReg(uint8_t regs[], size_t);
  bool _readNonConsReg(uint8_t regs[], size_t);

  uint8_t _readCommand(uint8_t);

  sl_i2cspm_t* _i2cPort;
};

#endif // DRIVER_DA7280_H
