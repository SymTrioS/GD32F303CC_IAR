/*!
    \file    ads1120.c
    \version 2025-09-21, V2.2.0, firmware for GD32F303 Prime-S73P board
    \author  Parsa (minimal modify by SymTrioS)
*/
/*
    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT 

NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR 
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
OF SUCH DAMAGE.
*/

#include <stdint.h>
#include <stdbool.h>
#include "gd32f30x.h"
#include "gd32f30x_gpio.h"
#include "gd32f30x_spi.h"
#include "prime-s73p.h"
#include "systick.h"
#include "ads1120.h"
#include "dac7311.h"

extern spi_parameter_struct hspi;

uint8_t ads1120_init(ADS1120_params *adsParam){
    adsParam->vRef = 2.048;
    adsParam->gain = 1;
    adsParam->refMeasurement = false;
    adsParam->convMode = ADS1120_SINGLE_SHOT;

    GPIO_BOP(ADS1120_GPIO_Port)=ADS1120_CS_PIN; // Set CS pin = 1

    ads1120_reset(adsParam);
//    ads1120_start(adsParam);
    uint8_t ctrlVal = 0;
    ads1120_bypassPGA(adsParam, true); // just a test if the ADS1220 is connected
    ctrlVal = ads1120_readRegister(adsParam, ADS1120_CONF_REG_0);
    ctrlVal = ctrlVal & 0x01;
    ads1120_bypassPGA(adsParam, false);
    return ctrlVal;
}

void ads1120_start(ADS1120_params *adsParam){
    ads1120_command(adsParam, ADS1120_START);
}

void ads1120_reset(ADS1120_params *adsParam){
    ads1120_command(adsParam, ADS1120_RESET);
    delay_1ms(1);
}

void ads1120_powerDown(ADS1120_params *adsParam){
    ads1120_command(adsParam, ADS1120_PWRDOWN);
}

/* Configuration Register 0 settings */

void ads1120_setCompareChannels(ADS1120_params *adsParam, ads1120Mux mux){
    if ((mux == ADS1120_MUX_REFPX_REFNX_4) || (mux == ADS1120_MUX_AVDD_M_AVSS_4)){
    	adsParam->gain = 1;       // under these conditions gain is one by definition
    	adsParam->refMeasurement = true;
    } else {                      // otherwise read gain from register
    	adsParam->regValue = ads1120_readRegister(adsParam, ADS1120_CONF_REG_0);
    	adsParam->regValue = adsParam->regValue & 0x0E;
    	adsParam->regValue = adsParam->regValue>>1;
    	adsParam->gain = 1 << adsParam->regValue;
    	adsParam->refMeasurement = false;
    }
    adsParam->regValue = ads1120_readRegister(adsParam, ADS1120_CONF_REG_0);
    adsParam->regValue &= ~0xF1;
    adsParam->regValue |= mux;
    adsParam->regValue |= !(adsParam->doNotBypassPgaIfPossible & 0x01);
    ads1120_writeRegister(adsParam, ADS1120_CONF_REG_0, adsParam->regValue);
    if ((mux >= 0x80) && (mux <=0xD0)){
        if (adsParam->gain > 4){
            adsParam->gain = 4;   // max gain is 4 if single-ended input is chosen or PGA is bypassed
        }
        ads1120_forcedBypassPGA(adsParam);
    }
}


void ads1120_setGain(ADS1120_params *adsParam, ads1120Gain enumGain){
    adsParam->regValue = ads1120_readRegister(adsParam, ADS1120_CONF_REG_0);
    ads1120Mux mux = (ads1120Mux)(adsParam->regValue & 0xF0);
    adsParam->regValue &= ~0x0E;
    adsParam->regValue |= enumGain;
    ads1120_writeRegister(adsParam, ADS1120_CONF_REG_0, adsParam->regValue);

    adsParam->gain = 1<<(enumGain>>1);
    if ((mux >= 0x80) && (mux <=0xD0)){
        if (adsParam->gain > 4){
            adsParam->gain = 4;   // max gain is 4 if single-ended input is chosen or PGA is bypassed
        }
        ads1120_forcedBypassPGA(adsParam);
    }
}


void ads1120_bypassPGA(ADS1120_params *adsParam, bool bypass){
    adsParam->regValue = ads1120_readRegister(adsParam, ADS1120_CONF_REG_0);
    adsParam->regValue &= ~0x01;
    adsParam->regValue |= bypass;
    adsParam->doNotBypassPgaIfPossible = !(bypass & 0x01);
    ads1120_writeRegister(adsParam, ADS1120_CONF_REG_0, adsParam->regValue);
}

bool ads1120_isPGABypassed(ADS1120_params *adsParam){
    adsParam->regValue = ads1120_readRegister(adsParam, ADS1120_CONF_REG_0);
    return adsParam->regValue & 0x01;
}


/* Configuration Register 1 settings */

void ads1120_setDataRate(ADS1120_params *adsParam, ads1120DataRate rate){
    adsParam->regValue = ads1120_readRegister(adsParam, ADS1120_CONF_REG_1);
    adsParam->regValue &= ~0xE0;
    adsParam->regValue |= rate;
    ads1120_writeRegister(adsParam, ADS1120_CONF_REG_1, adsParam->regValue);
}

void ads1120_setOperatingMode(ADS1120_params *adsParam, ads1120OpMode mode){
    adsParam->regValue = ads1120_readRegister(adsParam, ADS1120_CONF_REG_1);
    adsParam->regValue &= ~0x18;
    adsParam->regValue |= mode;
    ads1120_writeRegister(adsParam, ADS1120_CONF_REG_1, adsParam->regValue);
}

void ads1120_setConversionMode(ADS1120_params *adsParam, ads1120ConvMode mode){
    adsParam->convMode = mode;
    adsParam->regValue = ads1120_readRegister(adsParam, ADS1120_CONF_REG_1);
    adsParam->regValue &= ~0x04;
    adsParam->regValue |= mode;
    ads1120_writeRegister(adsParam, ADS1120_CONF_REG_1, adsParam->regValue);
}

void ads1120_enableTemperatureSensor(ADS1120_params *adsParam, bool enable){
    adsParam->regValue = ads1120_readRegister(adsParam, ADS1120_CONF_REG_1);
    if (enable){
        adsParam->regValue |= 0x02;
    } else {
        adsParam->regValue &= ~0x02;
    }
    ads1120_writeRegister(adsParam, ADS1120_CONF_REG_1, adsParam->regValue);
}

void ADS1120_enableBurnOutCurrentSources(ADS1120_params *adsParam, bool enable){
    adsParam->regValue = ads1120_readRegister(adsParam, ADS1120_CONF_REG_1);
    if (enable){
        adsParam->regValue |= 0x01;
    } else {
        adsParam->regValue &= ~0x01;
    }
    ads1120_writeRegister(adsParam, ADS1120_CONF_REG_1, adsParam->regValue);
}

/* Configuration Register 2 settings */

void ads1120_setVRefSource(ADS1120_params *adsParam, ads1120VRef vRefSource){
    adsParam->regValue = ads1120_readRegister(adsParam, ADS1120_CONF_REG_2);
    adsParam->regValue &= ~0xC0;
    adsParam->regValue |= vRefSource;
    ads1120_writeRegister(adsParam, ADS1120_CONF_REG_2, adsParam->regValue);
}

void ads1120_setFIRFilter(ADS1120_params *adsParam, ads1120FIR fir){
    adsParam->regValue = ads1120_readRegister(adsParam, ADS1120_CONF_REG_2);
    adsParam->regValue &= ~0x30;
    adsParam->regValue |= fir;
    ads1120_writeRegister(adsParam, ADS1120_CONF_REG_2, adsParam->regValue);
}

void ads1120_setLowSidePowerSwitch(ADS1120_params *adsParam, ads1120PSW psw){
    adsParam->regValue = ads1120_readRegister(adsParam, ADS1120_CONF_REG_2);
    adsParam->regValue &= ~0x08;
    adsParam->regValue |= psw;
    ads1120_writeRegister(adsParam, ADS1120_CONF_REG_2, adsParam->regValue);
}

void ads1120_setIdacCurrent(ADS1120_params *adsParam, ads1120IdacCurrent current){
    adsParam->regValue = ads1120_readRegister(adsParam, ADS1120_CONF_REG_2);
    adsParam->regValue &= ~0x07;
    adsParam->regValue |= current;
    ads1120_writeRegister(adsParam, ADS1120_CONF_REG_2, adsParam->regValue);
    delay_1ms(1);
}

/* Configuration Register 3 settings */

void ads1120_setIdac1Routing(ADS1120_params *adsParam, ads1120IdacRouting route){
    adsParam->regValue = ads1120_readRegister(adsParam, ADS1120_CONF_REG_3);
    adsParam->regValue &= ~0xE0;
    adsParam->regValue |= (route<<5);
    ads1120_writeRegister(adsParam, ADS1120_CONF_REG_3, adsParam->regValue);
}

void ads1120_setIdac2Routing(ADS1120_params *adsParam, ads1120IdacRouting route){
    adsParam->regValue = ads1120_readRegister(adsParam, ADS1120_CONF_REG_3);
    adsParam->regValue &= ~0x1C;
    adsParam->regValue |= (route<<2);
    ads1120_writeRegister(adsParam, ADS1120_CONF_REG_3, adsParam->regValue);
}

void ads1120_setDrdyMode(ADS1120_params *adsParam, ads1120DrdyMode mode){
    adsParam->regValue = ads1120_readRegister(adsParam, ADS1120_CONF_REG_3);
    adsParam->regValue &= ~0x02;
    adsParam->regValue |= mode;
    ads1120_writeRegister(adsParam, ADS1120_CONF_REG_3, adsParam->regValue);
}

/* Other settings */

void ads1120_setAvddAvssAsVrefAndCalibrate(ADS1120_params *adsParam){
    float avssVoltage = 0.0;
    ads1120_setVRefSource(adsParam, ADS1120_VREF_AVDD_AVSS);
    ads1120_setCompareChannels(adsParam, ADS1120_MUX_AVDD_M_AVSS_4);
    for (int i = 0; i<10; i++){
        avssVoltage += ads1120_getVoltage_mV(adsParam);
    }
    adsParam->vRef = avssVoltage * 4.0 / 10000.0;
}

void ads1120_setRefp0Refn0AsVefAndCalibrate(ADS1120_params *adsParam){
    float ref0Voltage = 0.0;
    ads1120_setVRefSource(adsParam, ADS1120_VREF_REFP0_REFN0);
    ads1120_setCompareChannels(adsParam, ADS1120_MUX_REFPX_REFNX_4);
    for (int i = 0; i<10; i++){
        ref0Voltage += ads1120_getVoltage_mV(adsParam);
    }
    adsParam->vRef = ref0Voltage * 4.0 / 10000.0;
}

void ads1120_setRefp1Refn1AsVefAndCalibrate(ADS1120_params *adsParam){
    float ref1Voltage = 0.0;
    ads1120_setVRefSource(adsParam, ADS1120_VREF_REFP1_REFN1);
    ads1120_setCompareChannels(adsParam, ADS1120_MUX_REFPX_REFNX_4);
    for (int i = 0; i<10; i++){
        ref1Voltage += ads1120_getVoltage_mV(adsParam);
    }
    adsParam->vRef = ref1Voltage * 4.0 / 10000.0;
}

void ads1120_setIntVRef(ADS1120_params *adsParam){
    ads1120_setVRefSource(adsParam, ADS1120_VREF_INT);
    adsParam->vRef = 2.048;
}

/* Results */
float ads1120_getVoltage_mV(ADS1120_params *adsParam){
    int32_t rawData = ads1120_getData(adsParam);
    float resultInMV = 0.0;
    if (adsParam->refMeasurement){
        resultInMV = (rawData / ADS1120_RANGE) * 2.048 * 1000.0 / (adsParam->gain * 1.0);
    } else {
        resultInMV = (rawData / ADS1120_RANGE) * adsParam->vRef * 1000.0 / (adsParam->gain * 1.0);
    }
    return resultInMV;
}

float ads1120_getVoltage_muV(ADS1120_params *adsParam){
    return ads1120_getVoltage_mV(adsParam) * 1000.0;
}

int16_t ads1120_getRawData(ADS1120_params *adsParam){
    return ads1120_getData(adsParam);
}

float ads1120_getTemperature(ADS1120_params *adsParam){
    ads1120_enableTemperatureSensor(adsParam, true);
    int16_t rawResult = ads1120_readResult(adsParam);
    ads1120_enableTemperatureSensor(adsParam, false);

    uint16_t result = (rawResult >> 2);
    if (result>>13){
        result = ~(result-1) & 0x3777;
        return result * (-0.03125);
    }
    return result * 0.03125;
}

/************************************************
    private functions
*************************************************/

void ads1120_forcedBypassPGA(ADS1120_params *adsParam){
    adsParam->regValue = ads1120_readRegister(adsParam, ADS1120_CONF_REG_0);
    adsParam->regValue |= 0x01;
    ads1120_writeRegister(adsParam, ADS1120_CONF_REG_0, adsParam->regValue);
}

int16_t ads1120_getData(ADS1120_params *adsParam){
    union Data {
        uint16_t rawResult;
        int16_t result;
    };
    union Data data;
    data.rawResult = ads1120_readResult(adsParam);

    return data.result;
}

uint16_t ads1120_readResult(ADS1120_params *adsParam){
  uint8_t buf[3]={0,0,0};
    uint32_t rawResult = 0;

    if (adsParam->convMode == ADS1120_SINGLE_SHOT){
    	ads1120_start(adsParam);
    }
    while (gpio_input_bit_get(ADS1120_GPIO_Port, ADS1120_DRDY_PIN) == SET);

    GPIO_BOP(DAC7311A_GPIO_Port)=DAC7311A_CS_PIN;  // Set CS A pin = 1
    GPIO_BOP(DAC7311B_GPIO_Port)=DAC7311B_CS_PIN;  // Set CS B pin = 1
    GPIO_BC(ADS1120_GPIO_Port)=ADS1120_CS_PIN;     // Set CS pin = 0
    buf[0]=SPIioByte(buf[2]);
    buf[1]=SPIioByte(buf[2]);
    GPIO_BOP(ADS1120_GPIO_Port)=ADS1120_CS_PIN;    // Set CS pin = 1

    rawResult = buf[0];
    rawResult = (rawResult << 8) | buf[1];

    return rawResult;
}

uint8_t ads1120_readRegister(ADS1120_params *adsParam, uint8_t reg){
    adsParam->regValue = 0;

    uint8_t buf = {ADS1120_RREG | (reg<<2)};

    GPIO_BOP(DAC7311A_GPIO_Port)=DAC7311A_CS_PIN;  // Set CS A pin = 1
    GPIO_BOP(DAC7311B_GPIO_Port)=DAC7311B_CS_PIN;  // Set CS B pin = 1
    GPIO_BC(ADS1120_GPIO_Port)=ADS1120_CS_PIN;     // Set CS pin = 0

    SPIioByte(buf);
    buf=0;
    adsParam->regValue=SPIioByte(buf);

    GPIO_BOP(ADS1120_GPIO_Port)=ADS1120_CS_PIN;    // Set CS pin = 1

    return adsParam->regValue;
}

void ads1120_writeRegister(ADS1120_params *adsParam, uint8_t reg, uint8_t val){
    GPIO_BOP(DAC7311A_GPIO_Port)=DAC7311A_CS_PIN;  // Set CS A pin = 1
    GPIO_BOP(DAC7311B_GPIO_Port)=DAC7311B_CS_PIN;  // Set CS B pin = 1
    GPIO_BC(ADS1120_GPIO_Port)=ADS1120_CS_PIN;     // Set CS pin = 0

    uint8_t buf = {(ADS1120_WREG | (reg<<2))};
    SPIioByte(buf);
    SPIioByte(val);

    GPIO_BOP(ADS1120_GPIO_Port)=ADS1120_CS_PIN;    // Set CS pin = 1
}

void ads1120_command(ADS1120_params *adsParam, uint8_t cmd){
    GPIO_BOP(DAC7311A_GPIO_Port)=DAC7311A_CS_PIN;  // Set CS A pin = 1
    GPIO_BOP(DAC7311B_GPIO_Port)=DAC7311B_CS_PIN;  // Set CS B pin = 1
    GPIO_BC(ADS1120_GPIO_Port)=ADS1120_CS_PIN;     // Set CS pin = 0

    SPIioByte(cmd);

    GPIO_BOP(ADS1120_GPIO_Port)=ADS1120_CS_PIN;    // Set CS pin = 1
}
