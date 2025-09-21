/*!
    \file    prime-s73p.c
    \version 2025-09-21, V2.2.0, firmware for GD32F303 Prime-S73P board
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

#include <gd32f30x.h>
#include <gd32f30x_gpio.h>
#include "prime-s73p.h"

spi_parameter_struct hspi;

void board_ports_init(void)
{
    /* Enable the port clock */
    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_GPIOB);
   
    /* Configure CK-CP pins */
    gpio_init(CK_Port, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_10MHZ, CKR_Pin);
    gpio_init(CK_Port, GPIO_MODE_OUT_PP, GPIO_OSPEED_10MHZ, CKC_Pin | CKT_Pin);
    GPIO_BC(CK_Port) |= CKC_Pin | CKT_Pin;
    gpio_init(CP_Port, GPIO_MODE_OUT_PP, GPIO_OSPEED_10MHZ, Cout_Pin);
    GPIO_BC(CP_Port) |= Cout_Pin;
    /* Configure contolled ADS1120 pin as output */
    gpio_init(ADS1120_GPIO_Port, GPIO_MODE_OUT_PP, GPIO_OSPEED_10MHZ, ADS1120_CS_PIN);
    GPIO_BOP(ADS1120_GPIO_Port)  |= ADS1120_CS_PIN;
    /* Configure contolled DAC7311 pin as output */
    gpio_init(DAC7311A_GPIO_Port, GPIO_MODE_OUT_PP, GPIO_OSPEED_10MHZ, DAC7311A_CS_PIN);
    GPIO_BOP(DAC7311A_GPIO_Port) |= DAC7311A_CS_PIN;
    gpio_init(DAC7311B_GPIO_Port, GPIO_MODE_OUT_PP, GPIO_OSPEED_10MHZ, DAC7311B_CS_PIN);
    GPIO_BOP(DAC7311B_GPIO_Port) |= DAC7311B_CS_PIN;
   /* Configure controlled ADS1120 pin as input */
    gpio_init(ADS1120_GPIO_Port, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_10MHZ, ADS1120_DRDY_PIN);
}

/* private variables */
static rcu_periph_enum COM_CLK[COMn] = {COM1_CLK, COM2_CLK};
static uint32_t COM_TX_PIN[COMn] = {COM1_TX_PIN, COM2_TX_PIN};
static uint32_t COM_RX_PIN[COMn] = {COM1_RX_PIN, COM2_RX_PIN};
static uint32_t COM_GPIO_PORT[COMn] = {COM1_GPIO_PORT, COM2_GPIO_PORT};
static rcu_periph_enum COM_GPIO_CLK[COMn] = {COM1_GPIO_CLK, COM2_GPIO_CLK};

/*!
    \brief      configure COM port
    \param[in]  com: COM on the board
      \arg        PRIME_COM1: COM1 on the board
      \arg        PRIME_COM2: COM2 on the board
    \param[out] none
    \retval     none
*/
void board_com_init(uint32_t com)
{
    uint32_t com_id = 0U;
    if (com == COM2) com_id = 1U;

    /* enable GPIO clock */
    rcu_periph_clock_enable(COM_GPIO_CLK[com_id]);

    /* enable USART clock */
    rcu_periph_clock_enable(COM_CLK[com_id]);

    /* connect port to USARTx_Tx */
    gpio_init(COM_GPIO_PORT[com_id], GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, COM_TX_PIN[com_id]);

    /* connect port to USARTx_Rx */
    gpio_init(COM_GPIO_PORT[com_id], GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, COM_RX_PIN[com_id]);

    /* USART configure */
    usart_deinit(com);
    usart_baudrate_set(com, 115200U);
    usart_receive_config(com, USART_RECEIVE_ENABLE);
    usart_transmit_config(com, USART_TRANSMIT_ENABLE);
    usart_enable(com);
}

//#define HARDSPI
#define  SOFTSPI

void board_spi_init(void)
{
    /* Configure SPI1 GPIO: SCK/PB13, MISO/PB14, MOSI/PB15 */
    rcu_periph_clock_enable(RCU_GPIOB);
    // Configure pin MISO as input
    gpio_init(SPI_Port, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, SPI_MISO);
#ifdef SOFTSPI
    // Software SPI
    gpio_init(SPI_Port, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, SPI_SCK | SPI_MOSI);
    GPIO_BC(SPI_Port)  = SPI_SCK | SPI_MOSI;
#endif
#ifdef HARDSPI
    rcu_periph_clock_enable(RCU_SPI1);
    rcu_periph_clock_enable(RCU_AF);
    // Hardware SPI
    gpio_init(SPI_Port, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, SPI_SCK | SPI_MOSI);
    
    // Initialization of SPI
    spi_struct_para_init(&hspi);
    hspi.device_mode          = SPI_MASTER;
    hspi.clock_polarity_phase = SPI_CK_PL_LOW_PH_2EDGE;
    hspi.nss                  = SPI_NSS_SOFT;
    hspi.prescale             = SPI_PSC_256;
    spi_init(SPI1, &hspi);
    spi_enable(SPI1);
#endif
}

// Receive byte, simultaneously send data
uint8_t SPIioByte(uint8_t txData)
{
    uint8_t rxData=0;
        
#ifdef SOFTSPI
    // Software SPI
    uint8_t i;
    for ( i=0; i<8; i++ )
    {
        if ( txData & 0x80 ) {
	   GPIO_BOP(SPI_Port) = SPI_MOSI;
        } else {
	   GPIO_BC(SPI_Port)  = SPI_MOSI;
        }
        txData <<= 1;
        GPIO_BOP(SPI_Port) = SPI_SCK;
        rxData = (rxData << 1) | gpio_input_bit_get(SPI_Port, SPI_MISO);
        GPIO_BC(SPI_Port) = SPI_SCK;
    }
    GPIO_BC(SPI_Port) = SPI_MOSI;
#endif
#ifdef HARDSPI1
    // Hardware SPI1
    while (RESET == spi_i2s_flag_get(SPI1, SPI_FLAG_TBE));
    spi_i2s_data_transmit(SPI1, txData);
    while (RESET == spi_i2s_flag_get(SPI1, SPI_FLAG_RBNE));
    rxData = spi_i2s_data_receive(SPI1);
#endif
    return rxData;
}

void SPIioBytes(uint8_t* txData, uint8_t* rxData, uint16_t lenght)
{
    for (uint16_t i=0; i<lenght; i++)
    {
        rxData[i] = SPIioByte(txData[i]);
    }
    return;
}
