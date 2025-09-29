/*!
    \file    dac7311.c
    \version 2025-09-21, V2.2.0, firmware for GD32F303 Prime-S73P board
    \author  SymTrioS
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

#include <stdbool.h>
#include "gd32f30x.h"
#include "gd32f30x_gpio.h"
#include "gd32f30x_spi.h"
#include "prime-s73p.h"
#include "systick.h"
#include "ads1120.h"
#include "dac7311.h"

extern spi_parameter_struct hspi;

void dac7311_powerdown(dac7311channel ch, dac7311pdnMode pmod) {
    uint8_t obyte;
    
    if ((pmod != DAC7311_NORMAL_MODE)&&(pmod != DAC7311_1k_TO_GND)&&(pmod != DAC7311_100k_TO_GND))
      return;
    
    GPIO_BOP(ADS1120_GPIO_Port)  = ADS1120_CS_PIN;    // Set CS pin = 1
    GPIO_BOP(DAC7311A_GPIO_Port) = DAC7311A_CS_PIN;   // Set CS A pin = 1
    GPIO_BOP(DAC7311B_GPIO_Port) = DAC7311B_CS_PIN;   // Set CS B pin = 1

    if (ch==DACA)
      GPIO_BC(DAC7311A_GPIO_Port)= DAC7311A_CS_PIN;  // Set CS A pin = 0
    else if (ch==DACB)
      GPIO_BC(DAC7311B_GPIO_Port)= DAC7311B_CS_PIN;  // Set CS B pin = 0

    obyte = 0;
    SPIioByte(obyte);
    obyte |= (uint8_t)((pmod<<6)&0xC0);
    SPIioByte(obyte);

    GPIO_BOP(DAC7311A_GPIO_Port) = DAC7311A_CS_PIN;  // Set CS A pin = 1
    GPIO_BOP(DAC7311B_GPIO_Port) = DAC7311B_CS_PIN;  // Set CS B pin = 1
    return;
}

void dac7311_writeDAC(dac7311channel ch, uint16_t val) {
    uint8_t obyte;
    
    if (val>4095) val = 4095;
    
    GPIO_BOP(ADS1120_GPIO_Port)  = ADS1120_CS_PIN;   // Set CS pin = 1
    GPIO_BOP(DAC7311A_GPIO_Port) = DAC7311A_CS_PIN;  // Set CS A pin = 1
    GPIO_BOP(DAC7311B_GPIO_Port) = DAC7311B_CS_PIN;  // Set CS B pin = 1
    
    if (ch==DACA)
      GPIO_BC(DAC7311A_GPIO_Port)= DAC7311A_CS_PIN;  // Set CS A pin = 0
    else if (ch==DACB)
      GPIO_BC(DAC7311B_GPIO_Port)= DAC7311B_CS_PIN;  // Set CS B pin = 0
    
    obyte = (uint8_t)((val>>6)&0x3F);
    SPIioByte(obyte);
    obyte = (uint8_t)((val<<2)&0xFC);
    SPIioByte(obyte);

    GPIO_BOP(DAC7311A_GPIO_Port) = DAC7311A_CS_PIN;  // Set CS A pin = 1
    GPIO_BOP(DAC7311B_GPIO_Port) = DAC7311B_CS_PIN;  // Set CS B pin = 1
    return;
}
