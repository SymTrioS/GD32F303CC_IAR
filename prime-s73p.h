/*!
    \file    prime-s73p.h
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

#ifndef _PRIME_S73P_H
#define _PRIME_S73P_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "gd32f30x.h"
   
// Prime-S73P board low layer pins

#define SPI_Port                        GPIOB
#define SPI_SCK                         GPIO_PIN_13
#define SPI_MISO                        GPIO_PIN_14
#define SPI_MOSI                        GPIO_PIN_15

// ADS1120 pins
#define ADS1120_GPIO_Port     	        GPIOB
#define ADS1120_DRDY_PIN                GPIO_PIN_2
#define ADS1120_CS_PIN                  GPIO_PIN_1

// DAC7311 pins
#define DAC7311A_GPIO_Port     	        GPIOA
#define DAC7311A_CS_PIN                 GPIO_PIN_6
#define DAC7311B_GPIO_Port     	        GPIOB
#define DAC7311B_CS_PIN                 GPIO_PIN_0

// CK-CP ports for GW1NR
#define CK_Port                         GPIOB
#define CKC_Pin                         GPIO_PIN_12
#define CKR_Pin                         GPIO_PIN_11
#define CKT_Pin                         GPIO_PIN_10

#define CP_Port                         GPIOA
#define Cout_Pin                        GPIO_PIN_9
#define Cin_Pin                         GPIO_PIN_10

/* Exported types */

#define COM1                             USART0
#define COM1_CLK                         RCU_USART0
#define COM1_TX_PIN                      GPIO_PIN_9
#define COM1_RX_PIN                      GPIO_PIN_10
#define COM1_GPIO_PORT                   GPIOA
#define COM1_GPIO_CLK                    RCU_GPIOA

#define COM2                             USART1
#define COM2_CLK                         RCU_USART1
#define COM2_TX_PIN                      GPIO_PIN_2
#define COM2_RX_PIN                      GPIO_PIN_3
#define COM2_GPIO_PORT                   GPIOA
#define COM2_GPIO_CLK                    RCU_GPIOA

void board_ports_init(void);
void board_com_init(void);
int outComStr(uint32_t COMn, char *pBuf);
void board_spi_init(void);
uint8_t SPIioByte(uint8_t txData);
void SPIioBytes(uint8_t* txData, uint8_t* rxData, uint16_t lenght);

#ifdef __cplusplus
}
#endif

#endif /* _PRIME_S73P_H */
