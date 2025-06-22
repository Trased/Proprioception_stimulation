#include "da7280_driver.h"
#include "da7280_patterns.h"
#include "gatt_db.h"
#include "sl_sleeptimer.h"
#include "app.h"
// local includes
#include "math.h"
#include <stdio.h>

#define ARR_MAX_LEN 128

static uint8_t snpMemCopy[100] = {0};
static sl_i2cspm_t* _i2cPort;
static const uint8_t _address = DEF_ADDR;

static uint8_t _weight = 0;
static uint8_t _available_patterns[ARR_MAX_LEN] = {0xFF};
static uint8_t _parser_idx = 0;
static uint8_t _pattern_idx = 0xFF;
static uint32_t _activity_time_ms = 0;
static bool     _activity_done = false;

static STATE_MACHINE _current_state = IDLE;
static BOOT_STATUS _boot_sts = BOOT_IN_PROGRESS;

static bool _writeRegister(uint8_t, uint8_t, uint8_t, uint8_t);
static bool _writeWaveFormMemory(uint8_t waveFormArray[]);
static uint8_t _readRegister(uint8_t);
static void _resetStateMachine();


void da7280_setActivityDone(bool status)
{
  _activity_done = status;
}

bool da7280_getActivityDone()
{
  return _activity_done;
}

void da7280_setBootStatus(BOOT_STATUS status)
{
  _boot_sts = status;
}

bool da7280_isActivityTimeSet()
{
  return 0 != _activity_time_ms;
}

void da7280_performActivity()
{
  if (_current_state != ACTIVE) {
    return;
  }
  const PatternMapEntry *entry = &pattern_map[_pattern_idx];
  const PatternStep    *steps = entry->steps;
  size_t                count = entry->count;

  if (count == 0) {
    // no steps? something happened, let's leave :D
    _current_state = IDLE;
    return;
  }

  if (_activity_time_ms == 0)
    {
      da7280_setVibrate(0);
      da7280_processUserInput(0xFF, 0xFF, NULL, 0);
      return;
    }

  for(uint8_t i = 0; i < count; i++)
    {
      da7280_setVibrate(steps[i].force_pct);
      sl_sleeptimer_delay_millisecond(steps[i].duration_ms);

      if(steps[i].duration_ms > _activity_time_ms)
        {
          _activity_time_ms = 0;
          break;
        }
      _activity_time_ms -= steps[i].duration_ms;
    }
  da7280_setVibrate(0);
  if (500 > _activity_time_ms)
    {
      _activity_time_ms = 0;
      da7280_setActivityDone(true);
      da7280_processUserInput(0xFF, 0xFF, NULL, 0);
      app_proceed();
    }
  else
    {
      _activity_time_ms -= 500;
    }
  sl_sleeptimer_delay_millisecond(500);
}

uint8_t da7280_processUserInput(uint32_t event_value, uint16_t characteristic, char *out_buf, size_t out_buf_len)
{
  if(BOOT_COMPLETED != _boot_sts)
    {
      snprintf(out_buf, out_buf_len,
             "The controller is not initialized. Please try again later. If issue persists, please restart the device!");
    }
  switch(_current_state)
  {
    case IDLE:
    {
      if (gattdb_weight_value == characteristic)
        {
          _resetStateMachine();
          int match_count = 0;
          int written = snprintf(out_buf, out_buf_len,
                                     "Available patterns are:");
          if (written < 0)
            {
              out_buf[0] = '\0';
              return 0;
            }
          size_t offset = (size_t)written;

          for(size_t i = 0; i < PATTERN_MAP_SIZE; i++)
            {
              if(pattern_map[i].mass_g == event_value)
                {
                  int w = snprintf(out_buf + offset,
                                   out_buf_len - offset,
                                   " %u, ",
                                   (unsigned)i);
                  if (w < 0 || (size_t)w >= out_buf_len - offset)
                    {
                      break;
                    }
                  offset += (size_t)w;
                  match_count++;
                  _available_patterns[_parser_idx] = i;
                  _parser_idx++;
                }
            }

          if(match_count > 0)
            {
              if(offset > 0 && out_buf[offset-2] == ',')
                {
                  out_buf[offset-2] = '.';
                  out_buf[offset-1] = '\0';
                }
              _weight = event_value;
              _current_state = WEIGHT_INPUT_RECEIVED;
            }
          else
            {
              snprintf(out_buf, out_buf_len,
                       "No patterns available for %u.",
                       (unsigned)event_value);
            }
        }
      else
        {
          snprintf(out_buf, out_buf_len,
                   "The received value is ignored in state %u. Please enter the actuator weight.",
                   (uint8_t)_current_state);
        }
      break;
    }
    case WEIGHT_INPUT_RECEIVED:
    {
      if (gattdb_pattern_value == characteristic)
        {
          for(uint8_t i = 0; i < ARR_MAX_LEN; i++)
            {
              if(0xFF == _available_patterns[i])
                {
                  break;
                }
              if(event_value == _available_patterns[i])
                {
                  _pattern_idx = event_value;
                  _current_state = PATTERN_INPUT_RECEIVED;
                }
            }
          if(0xFF == _pattern_idx)
            {
              snprintf(out_buf, out_buf_len,
                       "The received pattern is not available for specified weight. Try again.");
            }
          else
            {
              snprintf(out_buf, out_buf_len,
                       "Pattern %u will be used!",
                       _pattern_idx);
            }
        }
      else
        {
          snprintf(out_buf, out_buf_len,
                   "The received value is ignored in state %u. Please enter the vibration pattern.",
                   (uint8_t)_current_state);
        }
      break;
    }
    case PATTERN_INPUT_RECEIVED:
    {
      if (gattdb_activity_value == characteristic)
        {
          _activity_time_ms = (uint32_t)event_value*1000;
          _current_state = ACTIVE;
          snprintf(out_buf, out_buf_len,
                   "Activity started! It will last for %u seconds!",
                   (uint8_t)event_value);
        }
      else
        {
          snprintf(out_buf, out_buf_len,
                   "The received value is ignored in state %u. Please enter wanted activity time in seconds.",
                   (uint8_t)_current_state);
        }
      break;
    }
    case ACTIVE:
    {
      if (0xFF == characteristic)
        {
          _current_state = IDLE;
        }
      else
        {
          snprintf(out_buf, out_buf_len,
                   "The received value is ignored in state %u. Please wait for activity to end or send reset signal.",
                   (uint8_t)_current_state);
        }
      break;
    }
    default:
    {
      _current_state = IDLE;
    }
  }

  return 0;
}

bool da7280_begin(sl_i2cspm_t *i2cPort)
{
    _i2cPort = i2cPort;
    uint8_t chipRev = _readRegister(CHIP_REV_REG);

    return (chipRev == CHIP_REV);
}

bool da7280_setActuatorType(uint8_t type)
{
    if (type != LRA_TYPE && type != ERM_TYPE)
        return false;

    return _writeRegister(TOP_CFG1, 0xDF, type, 5);
}

bool da7280_setMotorSettings(hapticSettings userSettings)
{
    return (da7280_setActuatorType(userSettings.motorType) && da7280_setActuatorABSVolt(userSettings.absVolt) &&
        da7280_setActuatorNOMVolt(userSettings.nomVolt) && da7280_setActuatorIMAX(userSettings.currMax) &&
        da7280_setActuatorImpedance(userSettings.impedance) && da7280_setActuatorLRAfreq(userSettings.lraFreq));
}

hapticSettings da7280_getMotorSettings()
{
    hapticSettings temp;
    uint16_t v2i_factor;
    temp.nomVolt = _readRegister(ACTUATOR1) * (23.4 * pow(10.0, -3.0));
    temp.absVolt = _readRegister(ACTUATOR2) * (23.4 * pow(10.0, -3.0));
    temp.currMax = (_readRegister(ACTUATOR3) * 7.2) + 28.6;
    v2i_factor = (_readRegister(CALIB_V2I_H) << 8) | _readRegister(CALIB_IMP_L);
    temp.impedance = (v2i_factor * 1.6104) / (_readRegister(ACTUATOR3) + 4);
    return temp;
}

bool da7280_setOperationMode(OPERATION_MODES mode)
{
    if (mode > RTWM_MODE)
    {
        return false;
    }

    return _writeRegister(TOP_CTL1, 0xF8, mode, 0);

}

uint8_t da7280_getOperationMode()
{
    uint8_t mode = _readRegister(TOP_CTL1);
    return (mode & 0x07);
}

bool da7280_setActuatorABSVolt(float absVolt)
{
    if (absVolt < 0.0f || absVolt > 6.0f) {
        return false;
    }

    uint32_t scaled = (uint32_t)(absVolt / 23.4e-3f + 0.5f);
    if (scaled > 0xFF) scaled = 0xFF;
    return _writeRegister(ACTUATOR2, 0x00, (uint8_t)(scaled), 0);
}

float da7280_getActuatorABSVolt()
{
    uint8_t regVal = _readRegister(ACTUATOR2);

    return (regVal * (23.4 * pow(10.0, -3.0)));
}

bool da7280_setActuatorNOMVolt(float rmsVolt)
{
    if (rmsVolt < 0.0f || rmsVolt > 3.3f) {
        return false;
    }
    uint32_t scaled = (uint32_t)(rmsVolt / 23.4e-3f + 0.5f);
    if (scaled > 0xFF) scaled = 0xFF;
    return _writeRegister(ACTUATOR1, 0x00, (uint8_t)(scaled), 0);
}

float da7280_getActuatorNOMVolt()
{
    uint8_t regVal = _readRegister(ACTUATOR1);

    return (regVal * (23.4 * pow(10.0, -3.0)));
}

bool da7280_setActuatorIMAX(float maxCurr)
{
    if (maxCurr < 0 || maxCurr > 300.0) // Random upper limit
        return false;

    maxCurr = (maxCurr - 28.6) / 7.2;

    return _writeRegister(ACTUATOR3, 0xE0, (uint8_t)(maxCurr), 0);
}

uint16_t da7280_getActuatorIMAX()
{

    uint8_t regVal = _readRegister(ACTUATOR3);
    regVal &= 0x1F;

    return (regVal * 7.2) + 28.6;
}

bool da7280_setActuatorImpedance(float motorImpedance)
{
    if (motorImpedance < 0 || motorImpedance > 50.0)
        return false;

    uint8_t msbImpedance;
    uint8_t lsbImpedance;
    uint16_t v2iFactor;
    uint8_t maxCurr = _readRegister(ACTUATOR3) | 0x1F;

    v2iFactor = (motorImpedance * (maxCurr + 4)) / 1.6104;
    msbImpedance = (v2iFactor - (v2iFactor & 0x00FF)) / 256;
    lsbImpedance = (v2iFactor - (256 * (v2iFactor & 0x00FF)));

    return _writeRegister(CALIB_V2I_L, 0x00, lsbImpedance, 0) && _writeRegister(CALIB_V2I_H, 0x00, msbImpedance, 0);
}

uint16_t da7280_getActuatorImpedance()
{

    uint16_t regValMSB = _readRegister(CALIB_V2I_H);
    uint8_t regValLSB = _readRegister(CALIB_V2I_L);
    uint8_t currVal = _readRegister(ACTUATOR3) & 0x1F;

    uint16_t v2iFactor = (regValMSB << 8) | regValLSB;

    return (v2iFactor * 1.6104) / (currVal + 4);
}

uint16_t da7280_readImpAdjus()
{
    uint8_t tempMSB = _readRegister(CALIB_IMP_H);
    uint8_t tempLSB = _readRegister(CALIB_IMP_L);

    uint16_t totalImp = (4 * 62.5 * pow(10.0, -3.0) * tempMSB) + (62.5 * pow(10.0, -3.0) * tempLSB);
    return totalImp;
}

bool da7280_setActuatorLRAfreq(float frequency)
{
    if (frequency < 0 || frequency > 500.0)
        return false;

    uint8_t msbFrequency;
    uint8_t lsbFrequency;
    uint16_t lraPeriod;

    lraPeriod = 1 / (frequency * (1333.32 * pow(10.0, -9.0)));
    msbFrequency = (lraPeriod - (lraPeriod & 0x007F)) / 128;
    lsbFrequency = (lraPeriod - 128 * (lraPeriod & 0xFF00));

    return _writeRegister(FRQ_LRA_PER_H, 0x00, msbFrequency, 0) && _writeRegister(FRQ_LRA_PER_L, 0x80, lsbFrequency, 0);
}

bool da7280_enableCoinERM()
{
    return (da7280_enableAcceleration(false) &&
          da7280_enableRapidStop(false) &&
          da7280_enableAmpPid(false) &&
          da7280_enableV2iFactorFreeze(true) &&
          da7280_calibrateImpedanceDistance(true) &&
          da7280_setBemfFaultLimit(true));
}

bool da7280_enableAcceleration(bool enable)
{
    return _writeRegister(TOP_CFG1, 0xFB, enable, 2);
}

bool da7280_enableRapidStop(bool enable)
{
    return _writeRegister(TOP_CFG1, 0xFD, enable, 1);
}

bool da7280_enableAmpPid(bool enable)
{
    return _writeRegister(TOP_CFG1, 0xFE, enable, 0);
}

bool da7280_enableFreqTrack(bool enable)
{
    return _writeRegister(TOP_CFG1, 0xF7, enable, 3);
}

bool da7280_setBemfFaultLimit(bool enable)
{
    return _writeRegister(TOP_CFG1, 0xEF, enable, 4);
}

bool da7280_enableV2iFactorFreeze(bool enable)
{
    return _writeRegister(TOP_CFG4, 0x7F, enable, 7);
}

bool da7280_calibrateImpedanceDistance(bool enable)
{
    return _writeRegister(TOP_CFG4, 0xBF, enable, 6);
}

bool da7280_setVibrate(uint8_t val)
{
    uint8_t accelState = _readRegister(TOP_CFG1);
    accelState &= 0x04;
    accelState = accelState >> 2;

    if ((accelState == ENABLE) && (val > 0x7F))
    {
        val = 0x7F;
    }

    return _writeRegister(TOP_CTL2, 0x00, val, 0);
}

uint8_t da7280_getVibrate()
{
    return _readRegister(TOP_CTL2);
}

float da7280_getFullBrake()
{
    uint8_t tempThresh = _readRegister(TOP_CFG2);
    return (tempThresh & 0x0F) * 6.66;
}

bool da7280_setFullBrake(uint8_t thresh)
{
    if (thresh > 15)
        return false;

    return _writeRegister(TOP_CFG2, 0xF0, thresh, 0);
}

bool da7280_setMask(uint8_t mask)
{
    return _writeRegister(IRQ_MASK1, 0x00, mask, 0);
}

uint8_t da7280_getMask()
{
    return _readRegister(IRQ_MASK1);
}

bool da7280_setBemf(uint8_t val)
{
    if (val > 3)
        return false;

    return _writeRegister(TOP_INT_CFG1, 0xFC, val, 0);
}

float da7280_getBemf()
{
    int bemf = _readRegister(TOP_INT_CFG1);

    switch (bemf)
    {
    case 0x00:
        return 0.0;
    case 0x01:
        return 4.9;
    case 0x02:
        return 27.9;
    case 0x03:
        return 49.9;
    default:
        return 0.0;
    }
}

void da7280_clearIrq(uint8_t irq)
{
    _writeRegister(IRQ_EVENT1, ~irq, irq, 0);
}

uint8_t da7280_addFrame(uint8_t gain, uint8_t timeBase, uint8_t snipIdLow)
{
    uint8_t commandByteZero = 0; // Command byte zero is mandatory, snip-id begins at one
    commandByteZero = (gain << 5) | (timeBase << 3) | (snipIdLow << 0);
    return commandByteZero;
}

bool da7280_playFromMemory(bool enable)
{
    return _writeRegister(TOP_CTL1, 0xEF, enable, 4);
}

void da7280_eraseWaveformMemory()
{
    memset(snpMemCopy, 0, sizeof(snpMemCopy));
    _writeWaveFormMemory(snpMemCopy);
}

event_t da7280_getIrqEvent()
{
    uint8_t irqEvent = _readRegister(IRQ_EVENT1);

    if (!irqEvent)
        return HAPTIC_SUCCESS;

    switch (irqEvent)
    {
    case E_SEQ_CONTINUE:
        return E_SEQ_CONTINUE;
    case E_UVLO:
        return E_UVLO;
    case E_SEQ_DONE:
        return E_SEQ_DONE;
    case E_OVERTEMP_CRIT:
        return E_OVERTEMP_CRIT;
    case E_SEQ_FAULT:
        return E_SEQ_FAULT;
    case E_WARNING:
        return E_WARNING;
    case E_ACTUATOR_FAULT:
        return E_ACTUATOR_FAULT;
    case E_OC_FAULT:
        return E_OC_FAULT;
    default:
        return (event_t)(irqEvent);
    }
}

diag_status_t da7280_getEventDiag()
{
    uint8_t diag = _readRegister(IRQ_EVENT_SEQ_DIAG);

    switch (diag)
    {
    case E_PWM_FAULT:
        return E_PWM_FAULT;
    case E_MEM_FAULT:
        return E_MEM_FAULT;
    case E_SEQ_ID_FAULT:
        return E_SEQ_ID_FAULT;
    default:
        return NO_DIAG;
    }
}

status_t da7280_getIrqStatus()
{
    uint8_t status = _readRegister(IRQ_STATUS1);

    switch (status)
    {
    case STA_SEQ_CONTINUE:
        return STA_SEQ_CONTINUE;
    case STA_UVLO_VBAT_OK:
        return STA_UVLO_VBAT_OK;
    case STA_PAT_DONE:
        return STA_PAT_DONE;
    case STA_OVERTEMP_CRIT:
        return STA_OVERTEMP_CRIT;
    case STA_PAT_FAULT:
        return STA_PAT_FAULT;
    case STA_WARNING:
        return STA_WARNING;
    case STA_ACTUATOR:
        return STA_ACTUATOR;
    case STA_OC:
        return STA_OC;
    default:
        return STATUS_NOM;
    }
}

bool da7280_setSeqControl(uint8_t repetitions, uint8_t sequenceID)
{
    if (sequenceID > 15 || repetitions > 15)
        return false;

    return _writeRegister(SEQ_CTL2, 0xF0, sequenceID, 0) && _writeRegister(SEQ_CTL2, 0x0F, repetitions, 4);
}

static void _resetStateMachine()
{
  _weight = 0;
  _pattern_idx = 0xFF;
  _parser_idx = 0;

  for(uint8_t i = 0; i < ARR_MAX_LEN; i++)
    {
      _available_patterns[i] = 0xFF;
    }
}

static bool _writeRegister(uint8_t reg, uint8_t mask, uint8_t bits, uint8_t startPos)
{
    uint8_t value = _readRegister(reg);
    value &= mask;
    value |= (bits << startPos);

    uint8_t buf[2] = { reg, value };

    I2C_TransferSeq_TypeDef seq = {0};
    seq.addr        = _address << 1;
    seq.flags       = I2C_FLAG_WRITE;
    seq.buf[0].data = buf;
    seq.buf[0].len  = sizeof(buf);

    return (I2CSPM_Transfer(_i2cPort, &seq) == i2cTransferDone);
}

static uint8_t _readRegister(uint8_t reg)
{
    uint8_t result = 0;

    I2C_TransferSeq_TypeDef seq = {0};
    seq.addr        = _address << 1;
    seq.flags       = I2C_FLAG_WRITE_READ;
    seq.buf[0].data = &reg;
    seq.buf[0].len  = 1;
    seq.buf[1].data = &result;
    seq.buf[1].len  = 1;

    if (I2CSPM_Transfer(_i2cPort, &seq) == i2cTransferDone) {
        return result;
    }
    return 0;
}

static bool _writeWaveFormMemory(uint8_t waveFormArray[])
{
    enum { BUF_LEN = 1 + TOTAL_MEM_REGISTERS };
    uint8_t buf[BUF_LEN];
    buf[0] = NUM_SNIPPETS_REG;
    memcpy(&buf[1], waveFormArray, TOTAL_MEM_REGISTERS);

    I2C_TransferSeq_TypeDef seq = {0};
    seq.addr        = _address << 1;
    seq.flags       = I2C_FLAG_WRITE;
    seq.buf[0].data = buf;
    seq.buf[0].len  = BUF_LEN;

    _writeRegister(CIF_I2C1, 0x7F, 0, 7);
    bool ok = (I2CSPM_Transfer(_i2cPort, &seq) == i2cTransferDone);
    _writeRegister(CIF_I2C1, 0x7F, 1, 7);
    return ok;
}
