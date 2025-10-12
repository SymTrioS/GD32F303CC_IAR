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

#include <stdio.h>
#include "prime-s73p.h"
#include "gd32f30x.h"
#include "systick.h"
#include "ads1120.h"
#include "dac7311.h"
#include "gw1nr.h"
#include "main.h"

//---------------------------------------------------------------------------//

ADS1120_params dataADS;
ads1120Mux     chanMux=ADS1120_MUX_1_3;
float          ads13fmV;
uint16_t       ads13dat;
uint16_t       dacAdat=0;
uint16_t       dacBdat=0;
uint16_t       gwADC121[2];

int main(void)
{
//  configure systick
    systick_config();
//  initilize the ports peripheral
    board_ports_init();
//  initilize the USART
    board_com_init();
//  initilize the SPI
    board_spi_init();
//  initilize the ADS1120 chip
    ads1120_init(&dataADS);
    ads1120_setCompareChannels(&dataADS, chanMux);

//  The print operator outputs to COM2 (fputc)
//  print out the clock frequency of system, AHB, APB1 and APB2
    printf("\r\nGD32F303CC Demo Program of Board Prime-S73P:\r\n");
    printf("CK_SYS  is %d\r\n", rcu_clock_freq_get(CK_SYS));
    printf("CK_AHB  is %d\r\n", rcu_clock_freq_get(CK_AHB));
    printf("CK_APB1 is %d\r\n", rcu_clock_freq_get(CK_APB1));
    printf("CK_APB2 is %d\r\n", rcu_clock_freq_get(CK_APB2));
    
    while (1) 
    {
        dac7311_writeDAC(DACA, dacAdat);
        dac7311_writeDAC(DACB, dacBdat);
        delay_1ms(200);
        ads13dat = ads1120_readResult(&dataADS);
        ads13fmV = ads1120_getVoltage_mV(&dataADS);
        ads13fmV = -ads13fmV; // ADS1120: AIN1<->AIN3 inversion
        delay_1ms(800);
        getADCgw(gwADC121);   // Get data from two ADC121 connected to the FPGA GW1NR 
        printf("DacB=%d, ADS_13=%d, ADS_mV=%f\r\n", dacBdat, ads13dat, ads13fmV);
        printf("eADC1=%d, eADC2=%d\r\n", gwADC121[0], gwADC121[1]);
        dacAdat+=5; if (dacAdat>4095) dacAdat=0;
        dacBdat+=5; if (dacBdat>4095) dacBdat=0;
    }
}

/* retarget the C library printf function to the USART */
int fputc(int ch, FILE *f)
{
    while(RESET == usart_flag_get(COM2, USART_FLAG_TBE));
    usart_data_transmit(COM2, (uint16_t)ch);
    return ch;
}

