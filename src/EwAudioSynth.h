#ifndef _EW_AUDIO_SYNTH_H_
#define _EW_AUDIO_SYNTH_H_

#include <Arduino.h>

// Didsabled since it currently only contains the following typedef
//#include <EwAudio.h>
typedef int8_t Sample;

#if defined(__AVR_ATmega8__)
//
// On old ATmega8 boards.
//    Output is on pin 11
//
#   define PWM_PIN       11
#   define PWM_VALUE     OCR2
#   define PWM_INTERRUPT TIMER2_OVF_vect
#elif defined(__AVR_ATmega1280__)
//
// On the Arduino Mega
//    Output is on pin 3
//
#   define PWM_PIN       3
#   define PWM_VALUE     OCR3C
#   define PWM_INTERRUPT TIMER3_OVF_vect
#else
//
// For modern ATmega168 and ATmega328 boards
//    Output is on pin 3
//
#   define PWM_PIN       3
#   define PWM_VALUE     OCR2B
#   define PWM_INTERRUPT TIMER2_OVF_vect
#endif

class EwAudioSynth
{
public:
    static void begin();

    static void audioWrite(Sample sample)
    {
        PWM_VALUE = sample + 128;
    }
    static void audioWrite(uint16_t sample)
    {
        /* Clip */
        if (sample > 255) {
            sample = 255;
        }

        PWM_VALUE = sample & 0x00FF;
    }
    static void audioWrite(int16_t sample)
    {
        /* Clip */
        sample = sample >  127 ?  127 :
                 sample < -128 ? -128 :
                 sample;

        PWM_VALUE = sample + 128;
    }
};

#define EwAudioSynthLoop()    SIGNAL(PWM_INTERRUPT)

#endif /* _EW_AUDIO_SYNTH_H_ */
