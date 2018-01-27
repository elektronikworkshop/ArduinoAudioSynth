/** Granular synthesizer.
 * 
 * Mostly based on the original Auduino from Peter Knight.
 * 
 * Audio output: 3
 * MIDI RX:      RX
 * Grain Freq:   A0
 * Grain Decay:  A1
 * Grain Freq2:  A2
 * Grain Decay2: A3
 * 
 * NOTE: Don't forget to temporarily unplug the MIDI RX when uploading your sketch!
 * 
 */

#include <EwAudioSynth.h>
#include <MIDI.h>
#include "maps.h"

const uint8_t MidiChannel = 1;
MIDI_CREATE_DEFAULT_INSTANCE();

uint16_t syncPhaseAcc;
uint16_t syncPhaseInc;
uint16_t grainPhaseAcc;
uint16_t grainPhaseInc;
uint16_t grainAmp;
uint8_t grainDecay;
uint16_t grain2PhaseAcc;
uint16_t grain2PhaseInc;
uint16_t grain2Amp;
uint8_t grain2Decay;

// Map Analogue channels
#define GRAIN_FREQ_CONTROL   (0)
#define GRAIN_DECAY_CONTROL  (1)
#define GRAIN2_FREQ_CONTROL  (2)
#define GRAIN2_DECAY_CONTROL (3)
//#define SYNC_CONTROL         (6)

const uint8_t LED_PIN = 13;
#define LED_PORT  PORTB
#define LED_BIT   5
/* Flip to 0 for standard chromatic mapping of MIDI notes */
#define USE_PENTATONIC  1

uint8_t attenuation = 0;

/** Gets called when a MIDI message has been received on this instrument's
 * channel
 */
void midiNoteOn(byte /* channel */,
                byte note,
                byte velocity)
{
#if USE_PENTATONIC
  uint8_t index = map(note, 0, 127, 0, sizeof(pentatonicTable)/sizeof(pentatonicTable[0]) - 1);
  syncPhaseInc = pentatonicTable[index];
#else
  syncPhaseInc = midiTable[note];
#endif

  attenuation = ((127 - velocity) >> 4);
}

void midiNoteOff(byte /* channel  */,
                 byte /* note     */,
                 byte /* velocity */)
{
  syncPhaseInc = 0;
}


void setup()
{
  MIDI.setHandleNoteOn(midiNoteOn);
  MIDI.setHandleNoteOff(midiNoteOff);
  MIDI.begin(MidiChannel);
  MIDI.turnThruOff();

  EwAudioSynth::begin();

  pinMode(LED_PIN, OUTPUT);
}


void loop()
{
  MIDI.read();
    
  //uint16_t rawSyncCtrl    = 1023 - analogRead(SYNC_CONTROL);
  uint16_t rawGrainFreq1  = 1023 - analogRead(GRAIN_FREQ_CONTROL);
  uint16_t rawGrainDecay1 = 1023 - analogRead(GRAIN_DECAY_CONTROL);
  uint16_t rawGrainFreq2  = 1023 - analogRead(GRAIN2_FREQ_CONTROL);
  uint16_t rawGrainDecay2 = 1023 - analogRead(GRAIN2_DECAY_CONTROL);
  
  // The loop is pretty simple - it just updates the parameters for the
  // oscillators.
  //
  // Avoid using any functions that make extensive use of interrupts,
  // or turn interrupts off, this will cause clicks and pops in the audio.
  
  // Smooth frequency mapping
  //syncPhaseInc = mapPhaseInc(rawSyncCtrl) / 4;
  
  // Stepped mapping to midi notes: C, Db, D, Eb, E, F...
  //syncPhaseInc = mapMidi(rawSyncCtrl);
  
  // Stepped pentatonic mapping: D, E, G, A, B
  //syncPhaseInc = mapPentatonic(rawSyncCtrl);

  grainPhaseInc  = mapPhaseInc(rawGrainFreq1) / 2;
  grainDecay     = rawGrainDecay1 / 8;
  grain2PhaseInc = mapPhaseInc(rawGrainFreq2) / 2;
  grain2Decay    = rawGrainDecay2 / 4;
}

EwAudioSynthLoop()
{
  uint8_t value;
  uint16_t output;

  syncPhaseAcc += syncPhaseInc;
  if (syncPhaseAcc < syncPhaseInc) {
    // Time to start the next grain
    grainPhaseAcc = 0;
    grainAmp = 0x7fff;
    grain2PhaseAcc = 0;
    grain2Amp = 0x7fff;
    LED_PORT ^= 1 << LED_BIT; // Faster than using digitalWrite
  }
  
  // Increment the phase of the grain oscillators
  grainPhaseAcc += grainPhaseInc;
  grain2PhaseAcc += grain2PhaseInc;

  // Convert phase into a triangle wave
  value = (grainPhaseAcc >> 7) & 0xff;
  if (grainPhaseAcc & 0x8000) value = ~value;
  // Multiply by current grain amplitude to get sample
  output = value * (grainAmp >> 8);

  // Repeat for second grain
  value = (grain2PhaseAcc >> 7) & 0xff;
  if (grain2PhaseAcc & 0x8000) value = ~value;
  output += value * (grain2Amp >> 8);

  // Make the grain amplitudes decay by a factor every sample (exp. decay)
  grainAmp -= (grainAmp >> 8) * grainDecay;
  grain2Amp -= (grain2Amp >> 8) * grain2Decay;

  // Scale output to the available range, clipping if necessary
  // output >>= 9;
  // output >>= 8 
  output >>= 8 + attenuation;

  EwAudioSynth::audioWrite(output);
}
