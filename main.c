/*!
    \file    main.c
    \version 2025-09-21, V2.2.0, firmware for GD32F303 Prime-S73P board
    \author: SymTrioS
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

#include "gd32f30x.h"
#include "systick.h"
#include <stdio.h>
#include "main.h"
#include "ads1120.h"
#include "dac7311.h"
#include "prime-s73p.h"

void getData(uint16_t *pdata)
{
   uint16_t data=0;
   for (int i=0; i<12; i++)
   {
     data>>=1;
     if (gpio_input_bit_get(CK_Port,CKR_Pin)) data |= 0x800;
     gpio_bit_set(CK_Port,CKC_Pin);
     gpio_bit_reset(CK_Port,CKC_Pin); 
   }
   *pdata=data;
}

void getADCgw(uint16_t *ADCx2)
{
   // latching mode of shift register
   gpio_bit_reset(CP_Port,Cout_Pin);
   // latch impuls
   gpio_bit_set(CK_Port,CKC_Pin);
   gpio_bit_reset(CK_Port,CKC_Pin);
   // shifting mode
   gpio_bit_set(CP_Port,Cout_Pin);
   // 24-bit total:
   getData(&ADCx2[0]); // ADC1(12bit) 
   getData(&ADCx2[1]); // ADC2(12bit)
   // delay for viewing LED
   delay_1ms(200);
   gpio_bit_reset(CP_Port,Cout_Pin);
}

/*!
    \brief      main function
    \param[in]  none
    \param[out] none
    \retval     none
*/

ADS1120_params dataADS;
ads1120Mux     chanMux=ADS1120_MUX_1_3;
float          adc13fmV;
uint16_t       dacAdat=0;
uint16_t       dacBdat=0;
uint16_t       adc13dat;
uint16_t       gwADCx2[2];

int main(void)
{
    /* configure systick */
    systick_config();
    /* initilize the PINs */
    board_ports_init();
    /* initilize the USART */
    board_com_init(COM2);
    /* initilize the SPI */
    board_spi_init();
    /* ADS1120 init */
    ads1120_init(&dataADS);
    ads1120_setCompareChannels(&dataADS, chanMux);
    /* print out the clock frequency of system, AHB, APB1 and APB2 */
    printf("CK_SYS  is %d\r\n", rcu_clock_freq_get(CK_SYS));
    printf("CK_AHB  is %d\r\n", rcu_clock_freq_get(CK_AHB));
    printf("CK_APB1 is %d\r\n", rcu_clock_freq_get(CK_APB1));
    printf("CK_APB2 is %d\r\n", rcu_clock_freq_get(CK_APB2));
    
    while (1) {
        dac7311_writeDAC(DACA, dacAdat);
        dac7311_writeDAC(DACB, dacBdat);
        delay_1ms(200);
        adc13dat = ads1120_readResult(&dataADS);
        adc13fmV = ads1120_getVoltage_mV(&dataADS);
        adc13fmV = -adc13fmV; // ( AIN1 <-> AIN3 ) inversion
        delay_1ms(200);
        getADCgw(gwADCx2);
        printf("DacA=%d, ADC_13=%d, DacB=%d, ADC_mV=%f\r\n", dacAdat, adc13dat, dacBdat, adc13fmV);
        printf("eADC1=%d, eADC2=%d\r\n", gwADCx2[0], gwADCx2[1]);
        dacAdat+=5; if (dacAdat>4095) dacAdat=0;
        dacBdat+=5; if (dacBdat>4095) dacBdat=0;
    }
}

/* retarget the C library printf function to the USART */
int fputc(int ch, FILE *f)
{
    usart_data_transmit(COM2, (uint8_t)ch);
    while(RESET == usart_flag_get(COM2, USART_FLAG_TBE));

    return ch;
}
