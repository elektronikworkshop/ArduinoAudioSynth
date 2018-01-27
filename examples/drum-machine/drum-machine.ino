/** Drum Machine/Sample Player
 * 
 * Audio output: 3
 * MIDI RX:      RX
 * 
 * NOTE: Don't forget to temporarily unplug the MIDI RX when uploading your sketch!
 */

#include <EwAudioSynth.h>
#include <MIDI.h>

#include "data-bd.h"
#include "data-sd.h"
#include "data-oh.h"
#include "data-cp.h"

MIDI_CREATE_DEFAULT_INSTANCE();

/* The MIDI channel of the first sample. The next channel plays the second
 * sample and so on.
 */
const uint8_t FirstSampleMidiChannel = 2;

/** Data structure for each sample player
 */
class SamplePlayer
{ 
public:
  SamplePlayer(const char *data, size_t size)
    : sample(data)
    , N(size)
    , i(0)
    , pitch(127)
    , play(0)
  { }
  /** Pointer to sample data */
  const char* sample;
  /** Size of sample data */
  unsigned int N;
  /** Playback position.
   * Can be manipulated as well and leads to cool effects!
   */
  unsigned int i;
  /** Playback pitch. 
   * 127 is normal speed. Smaller slower, greater faster :)
   */
  uint8_t pitch;
  /** Play trigger and sample's volume.
   * Playback will start with values greater than zero.
   * The value itself determines the volume.
   */
  uint8_t play;      
};

/** The sample table */
SamplePlayer samples[] =
{
  {sample_bd, sizeof(sample_bd)},
  {sample_sd, sizeof(sample_sd)},
  {sample_oh, sizeof(sample_oh)},
  {sample_cp, sizeof(sample_cp)},
  
  /* Add your additional samples here */
};

/* Total number of samples - automagically deduced from the above array */
const uint8_t NumSamples = sizeof(samples)/sizeof(samples[0]);

void midiNoteOn(byte channel,
                byte note,
                byte velocity)
{
  /* Ignore non-relevant channels */
  if (channel <  FirstSampleMidiChannel or
      channel >= FirstSampleMidiChannel + NumSamples) {
    return;
  }
  
  /* Fetch sample associated with the note's channel */
  auto &sample = samples[channel - FirstSampleMidiChannel];

  /* Reset sample's playback position */
  sample.i = 0;
  /* Center note pitch around MIDI center, so 127 / 2 = 63 is normal speed */
  sample.pitch = map(note, 0, 127, 0, 255);
  sample.play  = map(velocity, 0, 127, 0, 255);
}

void midiNoteOff(byte /*channel*/,
                 byte /*note*/,
                 byte /*velocity*/)
{
  /* currently no note off implemented. Should set sample's play to 0 */
}

void setup()
{
  MIDI.setHandleNoteOn(midiNoteOn);
  MIDI.setHandleNoteOff(midiNoteOff);
  MIDI.begin(MIDI_CHANNEL_OMNI);
  MIDI.turnThruOff();
  
  EwAudioSynth::begin();
}

void loop()
{
  MIDI.read();
}

EwAudioSynthLoop()
{
  int16_t output = 0;

  for (int8_t i = NumSamples - 1; i >= 0; i--) {
    SamplePlayer* s = samples + i;
    if (s->play) {

      unsigned long k = s->i;
      k *= s->pitch;
      k >>= 7;  // pitch: normal speed at 127
//    k >>= 8;  // pitch: normal speed at 255
      
      if (k >= s->N) {
        s->play = 0;
        continue;
      }
      int16_t v = (char)pgm_read_byte_near(s->sample + (unsigned int)k);
      v *= s->play;
      v >>= 8;
      output += v;

      s->i++;
    }
  }
  /* Scale output to the available range (dividing by four) */
  output >>= 2;

  /* Write to PWM - this function clips and adds the offset
   * for us.
   */  
  EwAudioSynth::audioWrite(output);
}
