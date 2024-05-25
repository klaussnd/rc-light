/*  RC plane light
 *
 *  RC servo signal decoding with INT0 (D2, timer0)
 *  ACL: B7
 *  Landing light: C0
 *  Position lights: green D7 red D6 white D5
 *
 *  (c) Klaus Schneider-Zapp klaus_snd at web dot de
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License version 3
 *  as published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 */

#include "decode_servo.h"

#include <hal/avr/pin_io.h>

#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <inttypes.h>
#include <stdlib.h>
#include <util/atomic.h>
#include <util/delay.h>

#ifdef DEBUG
#include <hal/usart.h>
#include <hal/usart_stdout.h>

#include <avr/pgmspace.h>
#include <stdio.h>
#endif

// pins
#define PIN_NAV_GREEN D, 7
#define PIN_NAV_RED D, 6
#define PIN_NAV_WHITE D, 5
#define PIN_ACL B, 7
#define PIN_LANDING_L C, 0

// RC values for triggering actions
#define RC_VAL_ACL 0         // ACL on
#define RC_VAL_NAVLIGHT 20   // navigation lights on
#define RC_VAL_LANDLIGHT 80  // landing light on

int main(void)
{
   uint8_t cnt = 0;

   confPinAsOutput(PIN_NAV_GREEN);
   pinSetLow(PIN_NAV_GREEN);
   confPinAsOutput(PIN_NAV_RED);
   pinSetLow(PIN_NAV_RED);
   confPinAsOutput(PIN_NAV_WHITE);
   pinSetLow(PIN_NAV_WHITE);
   confPinAsOutput(PIN_ACL);
   pinSetLow(PIN_ACL);
   confPinAsOutput(PIN_LANDING_L);
   pinSetLow(PIN_LANDING_L);

   rcInit();

#ifdef DEBUG
   usartInit<4800>();
#endif

   sei();  // enable irq

   while (1)
   {
      const uint8_t rc_val = rcGetValue();
      const bool acl_is_on = (rc_val >= RC_VAL_ACL);

      if (rc_val >= RC_VAL_NAVLIGHT)
      {
         pinSetHigh(PIN_NAV_GREEN);
         pinSetHigh(PIN_NAV_RED);
         pinSetHigh(PIN_NAV_WHITE);
      }
      else
      {
         pinSetLow(PIN_NAV_GREEN);
         pinSetLow(PIN_NAV_RED);
         pinSetLow(PIN_NAV_WHITE);
      }

      if (rc_val >= RC_VAL_LANDLIGHT)
      {
         confPinAsInput(PIN_LANDING_L);
      }
      else
      {
         confPinAsOutput(PIN_LANDING_L);
         pinSetLow(PIN_LANDING_L);
      }

      if (acl_is_on)
      {
         if (cnt >= 10)
         {
            cnt = 0;

            confPinAsInput(PIN_ACL);
            _delay_ms(30);
            confPinAsOutput(PIN_ACL);
            _delay_ms(100);
            confPinAsInput(PIN_ACL);
            _delay_ms(30);
            confPinAsOutput(PIN_ACL);
         }
         else
         {
            _delay_ms(160);
            ++cnt;
         }
      }

#ifdef DEBUG
      fprintf_P(usart_stdout, PSTR("RC %u pos %u\n"), rc_val, pinIsSet(PIN_POS_GREEN));
#endif
   }
}
