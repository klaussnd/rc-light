/*  RC servo signal decoding with AVR
 *
 *  Uses a ring buffer to save the last decoded values.
 *  A median filter is then applied to remove incorrect values
 *  caused when an edge of the input signal is not captured correctly
 *  (spike-like aberations).
 *
 *  (c) Klaus Schneider-Zapp klaus underscore snd at web dot de
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3
 *  as published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 */

#include "decode_servo.h"

#include <hal/avr/pin_io.h>
#include <hal/avr/pin_names.h>
#include <hal/avr/timer.h>
#include <utils/ringbuffer.h>

#include <avr/interrupt.h>
#include <util/atomic.h>

#include <stdlib.h>  // qsort

/* use 8bit timer0
 * receiver signal: pulses of length 0.5ms ... 1.5ms
 * with CPU/8, 1.5ms represents less than 256 at more than 1.3MHz
 * (1/1.5ms = 667Hz, 256*667Hz = 0.17MHz, 8*0.17MHz = 1.36MHz)
 * thus need to have CPU/64 at higher speed
 *
 * timer values:
 * 1.0MHz or 8.0MHz: 62 ... 187
 * 3.6864MHz: 28 ... 86
 */
#define RC_TMR_NO 0

#if ((F_CPU >= 1000000UL) && (F_CPU < 1300000UL))
#define RC_TMR_PRESCALER 8
#else
#define RC_TMR_PRESCALER 64
#endif

/** convert time to timer ticks
 *  n = f_cpu/prescaler * time
 *  1 / 1ms = 1000 1/s */
#define RC_MS2TICK(_t) (_t * (F_CPU / MAKE_UL(RC_TMR_PRESCALER) / 1000UL))

/** theoretical zero value of timer, time = 1ms */
#define RC_ZERO RC_MS2TICK(1)

namespace
{
uint8_t getMedianServoValue();

/* Ring buffer for signal denoising */
RingBuffer<uint8_t, 4u> m_servo;
}  // namespace

void rcInit(void)
{
   /* configure INT0 as input and enable pullup */
   confPinAsInputWithPullup(PIN_INT0);
   /* setup interrupts and timer */
#ifdef MCUCR
   MCUCR |= (1 << ISC00);  /* interrupt on logical change on INT0 pin */
   MCUCR &= ~(1 << ISC01); /* " */
#else
   EICRA |= (1 << ISC00);  /* interrupt on logical change on INT0 pin */
   EICRA &= ~(1 << ISC01); /* " */
#endif
#if defined __AVR_ATtiny84__ || defined __AVR_ATtiny44__
   GIMSK |= (1 << INT0);
#elif defined GICR
   GICR |= (1 << INT0);    /* enable interrupt on INT0 */
#else
   EIMSK |= (1 << INT0);  /* enable interrupt on INT0 */
#endif
   TMR_SET_NORMAL(RC_TMR_NO); /* timer in normal mode */
   TMR_STOP(RC_TMR_NO);
   TMR_SET_VAL(RC_TMR_NO, 0); /* initial timer value */
}

void rcStop(void)
{
#if defined __AVR_ATtiny84__ || defined __AVR_ATtiny44__
   GIMSK &= ~(1 << INT0); /* disable interrupt on INT0 */
#elif defined GICR
   GICR &= ~(1 << INT0);   /* disable interrupt on INT0 */
#else
   EIMSK &= ~(1 << INT0); /* disable interrupt on INT0 */
#endif
   TMR_STOP(RC_TMR_NO); /* timer off */
}

uint8_t rcGetValue(void)
{
   const uint8_t value = getMedianServoValue();
   return value < RC_ZERO ? 0 : (value - RC_ZERO);
}

/* interrupt routine for INT0 / PD2 pinchange
   if pin goes high, start timer0
   if pin goes low, stop timer0 and copy interval to global variable */
ISR(INT0_vect)
{
   if (pinIsSet(PIN_INT0))  // INT0 got high
   {
      TMR_SET_VAL(RC_TMR_NO, 0);                       // reset timer
      TMR_SET_PRESCALER(RC_TMR_NO, RC_TMR_PRESCALER);  // start timer
   }
   else  // INT0 got low
   {
      TMR_STOP(RC_TMR_NO);
      m_servo.add(TMR_GET_VAL(RC_TMR_NO));
   }
}

namespace
{
/* comparison function for qsort() */
int median_cmp_val(const void* it1, const void* it2)
{
   const uint8_t* num1 = reinterpret_cast<const uint8_t*>(it1);
   const uint8_t* num2 = reinterpret_cast<const uint8_t*>(it2);
   return (static_cast<int>(*num1) - static_cast<int>(*num2));
}

uint8_t getMedianServoValue()
{
   if (!m_servo.isFull())
   {
      // need to wait until ring buffer is full before having a meaningful value
      return 0;
   }

   constexpr uint8_t size = m_servo.capacity();
   uint8_t buf[size];
   ATOMIC_BLOCK(ATOMIC_FORCEON)
   {
      for (uint8_t index = 0; index < size; ++index)
      {
         buf[index] = m_servo.elementAt(index);
      }
   }
   qsort(buf, size, sizeof(uint8_t), median_cmp_val);
   return buf[size / 2];
}
}  // namespace
