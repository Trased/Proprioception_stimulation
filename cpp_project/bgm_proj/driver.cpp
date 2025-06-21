#include "driver.h"

// local includes
#include "math.h"

Haptic_Driver::Haptic_Driver(uint8_t address):
_address(address),
_i2cPort(nullptr)
{}

bool Haptic_Driver::begin(sl_i2cspm_t *i2cPort)
{
    _i2cPort = i2cPort;
    uint8_t chipRev = _readRegister(CHIP_REV_REG);

    return (chipRev == CHIP_REV);
}

bool Haptic_Driver::setActuatorType(uint8_t type)
{
    if (type != LRA_TYPE && type != ERM_TYPE)
        return false;

    return _writeRegister(TOP_CFG1, 0xDF, type, 5);
}

bool Haptic_Driver::setMotorSettings(hapticSettings userSettings)
{
    return (setActuatorType(userSettings.motorType) && setActuatorABSVolt(userSettings.absVolt) &&
        setActuatorNOMVolt(userSettings.nomVolt) && setActuatorIMAX(userSettings.currMax) &&
        setActuatorImpedance(userSettings.impedance) && setActuatorLRAfreq(userSettings.lraFreq));
}

hapticSettings Haptic_Driver::getMotorSettings()
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

bool Haptic_Driver::setOperationMode(OPERATION_MODES mode)
{
    if (mode < INACTIVE || mode > RTWM_MODE)
        return false;

    return _writeRegister(TOP_CTL1, 0xF8, mode, 0);

}

uint8_t Haptic_Driver::getOperationMode()
{
    uint8_t mode = _readRegister(TOP_CTL1);
    return (mode & 0x07);
}

bool Haptic_Driver::setActuatorABSVolt(float absVolt)
{
    if (absVolt < 0.0f || absVolt > 6.0f) {
        return false;
    }

    uint32_t scaled = static_cast<uint32_t>(absVolt / 23.4e-3f + 0.5f);
    if (scaled > 0xFF) scaled = 0xFF;
    return _writeRegister(ACTUATOR2, 0x00, static_cast<uint8_t>(scaled), 0);
}

float Haptic_Driver::getActuatorABSVolt()
{
    uint8_t regVal = _readRegister(ACTUATOR2);

    return (regVal * (23.4 * pow(10.0, -3.0)));
}

bool Haptic_Driver::setActuatorNOMVolt(float rmsVolt)
{
    if (rmsVolt < 0.0f || rmsVolt > 3.3f) {
        return false;
    }
    uint32_t scaled = static_cast<uint32_t>(rmsVolt / 23.4e-3f + 0.5f);
    if (scaled > 0xFF) scaled = 0xFF;
    return _writeRegister(ACTUATOR1, 0x00, static_cast<uint8_t>(scaled), 0);
}

float Haptic_Driver::getActuatorNOMVolt()
{
    uint8_t regVal = _readRegister(ACTUATOR1);

    return (regVal * (23.4 * pow(10.0, -3.0)));
}

bool Haptic_Driver::setActuatorIMAX(float maxCurr)
{
    if (maxCurr < 0 || maxCurr > 300.0) // Random upper limit
        return false;

    maxCurr = (maxCurr - 28.6) / 7.2;

    return _writeRegister(ACTUATOR3, 0xE0, static_cast<uint8_t>(maxCurr), 0);
}

uint16_t Haptic_Driver::getActuatorIMAX()
{

    uint8_t regVal = _readRegister(ACTUATOR3);
    regVal &= 0x1F;

    return (regVal * 7.2) + 28.6;
}

bool Haptic_Driver::setActuatorImpedance(float motorImpedance)
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

uint16_t Haptic_Driver::getActuatorImpedance()
{

    uint16_t regValMSB = _readRegister(CALIB_V2I_H);
    uint8_t regValLSB = _readRegister(CALIB_V2I_L);
    uint8_t currVal = _readRegister(ACTUATOR3) & 0x1F;

    uint16_t v2iFactor = (regValMSB << 8) | regValLSB;

    return (v2iFactor * 1.6104) / (currVal + 4);
}

uint16_t Haptic_Driver::readImpAdjus()
{
    uint8_t tempMSB = _readRegister(CALIB_IMP_H);
    uint8_t tempLSB = _readRegister(CALIB_IMP_L);

    uint16_t totalImp = (4 * 62.5 * pow(10.0, -3.0) * tempMSB) + (62.5 * pow(10.0, -3.0) * tempLSB);
    return totalImp;
}

bool Haptic_Driver::setActuatorLRAfreq(float frequency)
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

bool Haptic_Driver::enableCoinERM()
{
    return (enableAcceleration(false) &&
            enableRapidStop(false) &&
            enableAmpPid(false) &&
            enableV2iFactorFreeze(true) &&
            calibrateImpedanceDistance(true) &&
            setBemfFaultLimit(true));
}

bool Haptic_Driver::enableAcceleration(bool enable)
{
    return _writeRegister(TOP_CFG1, 0xFB, enable, 2);
}

bool Haptic_Driver::enableRapidStop(bool enable)
{
    return _writeRegister(TOP_CFG1, 0xFD, enable, 1);
}

bool Haptic_Driver::enableAmpPid(bool enable)
{
    return _writeRegister(TOP_CFG1, 0xFE, enable, 0);
}

bool Haptic_Driver::enableFreqTrack(bool enable)
{
    return _writeRegister(TOP_CFG1, 0xF7, enable, 3);
}

bool Haptic_Driver::setBemfFaultLimit(bool enable)
{
    return _writeRegister(TOP_CFG1, 0xEF, enable, 4);
}

bool Haptic_Driver::enableV2iFactorFreeze(bool enable)
{
    return _writeRegister(TOP_CFG4, 0x7F, enable, 7);
}

bool Haptic_Driver::calibrateImpedanceDistance(bool enable)
{
    return _writeRegister(TOP_CFG4, 0xBF, enable, 6);
}

bool Haptic_Driver::setVibrate(uint8_t val)
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

uint8_t Haptic_Driver::getVibrate()
{
    return _readRegister(TOP_CTL2);
}

float Haptic_Driver::getFullBrake()
{
    uint8_t tempThresh = _readRegister(TOP_CFG2);
    return (tempThresh & 0x0F) * 6.66;
}

bool Haptic_Driver::setFullBrake(uint8_t thresh)
{
    if (thresh > 15)
        return false;

    return _writeRegister(TOP_CFG2, 0xF0, thresh, 0);
}

bool Haptic_Driver::setMask(uint8_t mask)
{
    return _writeRegister(IRQ_MASK1, 0x00, mask, 0);
}

uint8_t Haptic_Driver::getMask()
{
    return _readRegister(IRQ_MASK1);
}

bool Haptic_Driver::setBemf(uint8_t val)
{
    if (val > 3)
        return false;

    return _writeRegister(TOP_INT_CFG1, 0xFC, val, 0);
}

float Haptic_Driver::getBemf()
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

void Haptic_Driver::clearIrq(uint8_t irq)
{
    _writeRegister(IRQ_EVENT1, ~irq, irq, 0);
}

uint8_t Haptic_Driver::addFrame(uint8_t gain, uint8_t timeBase, uint8_t snipIdLow)
{
    uint8_t commandByteZero = 0; // Command byte zero is mandatory, snip-id begins at one
    commandByteZero = (gain << 5) | (timeBase << 3) | (snipIdLow << 0);
    return commandByteZero;
}

bool Haptic_Driver::playFromMemory(bool enable)
{
    return _writeRegister(TOP_CTL1, 0xEF, enable, 4);
}

void Haptic_Driver::eraseWaveformMemory()
{
    memset(snpMemCopy, 0, sizeof(snpMemCopy));
    _writeWaveFormMemory(snpMemCopy);
}

event_t Haptic_Driver::getIrqEvent()
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
        return static_cast<event_t>(irqEvent);
    }
}

diag_status_t Haptic_Driver::getEventDiag()
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

status_t Haptic_Driver::getIrqStatus()
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

bool Haptic_Driver::setSeqControl(uint8_t repetitions, uint8_t sequenceID)
{
    if (sequenceID > 15 || repetitions > 15)
        return false;

    return _writeRegister(SEQ_CTL2, 0xF0, sequenceID, 0) && _writeRegister(SEQ_CTL2, 0x0F, repetitions, 4);
}

bool Haptic_Driver::_writeRegister(uint8_t reg,
                                   uint8_t mask,
                                   uint8_t bits,
                                   uint8_t startPos)
{
    uint8_t value = _readRegister(reg);
    value &= mask;
    value |= (bits << startPos);

    uint8_t buf[2] = { reg, value };

    I2C_TransferSeq_TypeDef seq{};
    seq.addr        = _address << 1;
    seq.flags       = I2C_FLAG_WRITE;
    seq.buf[0].data = buf;
    seq.buf[0].len  = sizeof(buf);

    return (I2CSPM_Transfer(_i2cPort, &seq) == i2cTransferDone);
}

uint8_t Haptic_Driver::_readRegister(uint8_t reg)
{
    uint8_t result = 0;

    I2C_TransferSeq_TypeDef seq{};
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


bool Haptic_Driver::_writeConsReg(uint8_t regs[], size_t numWrites)
{
    _writeRegister(CIF_I2C1, 0x7F, 0, 7);

    I2C_TransferSeq_TypeDef seq{};
    seq.addr        = _address << 1;
    seq.flags       = I2C_FLAG_WRITE;
    seq.buf[0].data = regs;
    seq.buf[0].len  = numWrites + 1;

    bool ok = (I2CSPM_Transfer(_i2cPort, &seq) == i2cTransferDone);
    _writeRegister(CIF_I2C1, 0x7F, 1, 7);
    return ok;
}

bool Haptic_Driver::_writeNonConsReg(uint8_t regs[], size_t numWrites)
{
    _writeRegister(CIF_I2C1, 0x7F, 1, 7);

    I2C_TransferSeq_TypeDef seq{};
    seq.addr        = _address << 1;
    seq.flags       = I2C_FLAG_WRITE;
    seq.buf[0].data = regs;
    seq.buf[0].len  = numWrites + 1;

    bool ok = (I2CSPM_Transfer(_i2cPort, &seq) == i2cTransferDone);
    _writeRegister(CIF_I2C1, 0x7F, 0, 7);
    return ok;
}

bool Haptic_Driver::_writeWaveFormMemory(uint8_t waveFormArray[])
{
    enum { BUF_LEN = 1 + TOTAL_MEM_REGISTERS };
    uint8_t buf[BUF_LEN];
    buf[0] = NUM_SNIPPETS_REG;
    memcpy(&buf[1], waveFormArray, TOTAL_MEM_REGISTERS);

    I2C_TransferSeq_TypeDef seq{};
    seq.addr        = _address << 1;
    seq.flags       = I2C_FLAG_WRITE;
    seq.buf[0].data = buf;
    seq.buf[0].len  = BUF_LEN;

    _writeRegister(CIF_I2C1, 0x7F, 0, 7);
    bool ok = (I2CSPM_Transfer(_i2cPort, &seq) == i2cTransferDone);
    _writeRegister(CIF_I2C1, 0x7F, 1, 7);
    return ok;
}
