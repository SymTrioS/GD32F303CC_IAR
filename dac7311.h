/*
 * dac7311.h
 *
 *  Created on: Sep 21, 2025
 *      Author: SymTrioS
 */

#ifndef _DAC7311_H_
#define _DAC7311_H_

typedef enum DAC7311_CHANNEL {
    DACA                    = 0x00,
    DACB                    = 0x01,
} dac7311channel;

typedef enum DAC7311_PDN_MODE {
    DAC7311_NORMAL_MODE     = 0x00,    // default
    DAC7311_1k_TO_GND       = 0x01,
    DAC7311_100k_TO_GND     = 0x02
} dac7311pdnMode;

/* Commands */
void dac7311_powerdown(dac7311channel ch, dac7311pdnMode pmod);
void dac7311_writeDAC(dac7311channel ch, uint16_t val);

#endif /* _DAC7311_H_ */
