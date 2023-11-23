# MUS-X

## Modules

### ADSR
ADSR envelope generator with exponential decay/release, built in velocity scaling and sustain CV.
During the decay and release phase, a gate signal is output. This can e.g. be used to trigger another envelope generator, that shapes the sustain.

### Delay
A delay inspired by analog bucket-brigade delay pedals.

'Time' adjusts the delay time. The range is determined by the 'BBD size' parameter.
The delay time can also be tapped with the 'Tap' button. The delay time is also visualized by the 'Tap'-button LED.
'Feedback' adjusts the delay feedback. WARNING: this goes well beyond 100% and can cause self oscillation. The output is limited to +-10V.
Both 'Time' and 'Feedback' can be CV controlled.

'Cutoff' adjusts the cutoff frequency of the anti-aliasing and reconstruction filters. It makes the delay brighter or darker. The aliasing of the BBD can be audible at higher cutoff frequencies.
'Resonance' adjusts the resonance of the anti-aliasing and reconstruction filters. This alters the character of the delay, especially at high feedback amounts.
'Noise' adjusts the noise of the BBD line. More noise will result in self oscillation at high feedback levels.
'BBD size' adjusts the number of stages of the BBD line. A smaller size can be used for very short delays, or a chorus effect. Many famous hardware units use 2048 or 4096 stages.
A larger BBD size can give longer delay times, and a brighter delay without aliasing.

'Input' adjusts the input level to the delay. It does not affect the dry signal output. A small LED indicates an overload of the delay circuit and will result in saturation.
Turning up the 'Stereo' parameter gives a ping-pong delay. It has no effect when 'Inv R' is enabled.
If 'Inv R' is enabled, the right wet signal will be the inverted left wet signal. You can use this with a small BBD size (e.g. 512), no feedback, and delay time modulation to create a chorus effect.
'Mix' adjusts the dry-wet balance.

### Drift
Drift generates subtle constant offset and drift.
The 'Poly' input determines the polyphony channels of the output.
'Const' adjusts the amount of a random (per voice) constant offset.
'RNG' generates a new set of random offsets.
'Drift' adjusts the amount of a random (per voice) drift.
'Drift Rate' adjusts the frequency of the random drift.

### OnePole
A simple CV controllable 1-pole highpass and lowpass filter.

### Oscillators
A pair of analog-style oscillators.
'Shape' blends from triangle to sawtooth to pulse wave.
'PW' adjusts the phase of the triangle, and the pulse wave from 0% to 100% duty cycle.
'Vol' adjusts the oscillator volume.
'Sync' hard syncs oscillator 2 to oscillator 1.
'FM' adjusts the (Linear-Through-Zero) frequency modulation from oscillator 1 to oscillator 2.
Oscillator 2 stays in tune when applying FM.
'RM' adjusts the volume of the ring modulator (multiplication of oscillator 1 and 2).

Every parameter is CV controllable.
There are two 'V/Oct' inputs, one for each oscillator. You can use the 'Tune' module to add tuning controls.
'Out' outputs the mix of oscillator 1, oscillator 2 and the ring modulator.

The oscillators use a naive implementation, which is quite CPU friendly, and can therefore be massively oversampled to reduce aliasing.
This is especially useful for FM and sync sounds.
With no oversampling, the oscillators alias a lot.
Right click on the module to adjust the oversampling factor.

### Tune
Tune a V/Oct signal by +-5 octaves, plus coarse (1 octave) and fine (1 semitone) tuning.
The 'Fine' input is added and can be used for subtle modulations in the semitone range.
The output is limited to +-12V, so a huge frequency range can be covered.