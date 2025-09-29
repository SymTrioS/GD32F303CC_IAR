/*!
    \file    gw1nr.c
    \version 2025-09-28, V2.2.0, firmware for GD32F303 Prime-S73P board
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

#include "gd32f30x.h"
#include "prime-s73p.h"
#include "systick.h"
#include "main.h"

//---------------------------------------------------------------------------//

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
   // set latching mode of shift register
   gpio_bit_reset(CP_Port,Cout_Pin);
   // latch impuls
   gpio_bit_set(CK_Port,CKC_Pin);
   gpio_bit_reset(CK_Port,CKC_Pin);
   // set shifting mode
   gpio_bit_set(CP_Port,Cout_Pin);
   // 24-bit total:
   getData(&ADCx2[0]); // ADC1(12bit) 
   getData(&ADCx2[1]); // ADC2(12bit)
   // delay for LED visibility
   delay_1ms(200);     // witch active Cout_pin
   gpio_bit_reset(CP_Port,Cout_Pin);
}
