
#include "EwAudioSynth.h"

void
EwAudioSynth::begin()
{
    pinMode(PWM_PIN, OUTPUT);

#if defined(__AVR_ATmega8__)
    // ATmega8 has different registers
    TCCR2 = _BV(WGM20) | _BV(COM21) | _BV(CS20);
    TIMSK = _BV(TOIE2);
#elif defined(__AVR_ATmega1280__)
    TCCR3A = _BV(COM3C1) | _BV(WGM30);
    TCCR3B = _BV(CS30);
    TIMSK3 = _BV(TOIE3);
#else
    // Set up PWM to 31.25kHz, phase accurate
    TCCR2A = _BV(COM2B1) | _BV(WGM20);
    TCCR2B = _BV(CS20);
    TIMSK2 = _BV(TOIE2);
#endif
}