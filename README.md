# MUS-X

Modules for VCV Rack, with a focus on MIDI-controllable, analog poly-synths, and per-voice variance.

## Modules

### ADSR
ADSR envelope generator with exponential decay/release, built in velocity scaling and sustain CV.

'Rnd' randomly scales the A, D, S and R parameters per channel. This simulates the behavior of old polysynths with analog envelopes, where each voice/envelope behaves slightly different due to component tolerances. The offsets are individually generated for each ADSR module, and stored with the patch.

During the decay and release phase, a gate signal is output. This can e.g. be used to trigger another envelope generator, that shapes the sustain.

### Delay
A delay inspired by analog bucket-brigade delay pedals.

* 'Time' adjusts the delay time. The range is determined by the 'BBD size' parameter.
The delay time can also be tapped with the 'Tap' button. The delay time is also visualized by the 'Tap'-button LED.
  'Time' can be CV controlled.
* 'Feedback' adjusts the delay feedback. WARNING: this goes well beyond 100% and can cause self oscillation. The output is limited to ±10V.
'Feedback' can be CV controlled.
* 'Cutoff' adjusts the cutoff frequency of the anti-aliasing and reconstruction filters. It makes the delay brighter or darker. The aliasing of the BBD can be audible at higher cutoff frequencies.
* 'Resonance' adjusts the resonance of the anti-aliasing and reconstruction filters. This alters the character of the delay, especially at high feedback amounts.
* 'Noise' adjusts the noise of the BBD line. More noise will result in self oscillation at high feedback levels.
* 'BBD size' adjusts the number of stages of the BBD line. A smaller size can be used for very short delays, or a chorus effect. Many famous hardware units use 2048 or 4096 stages.
A larger BBD size can give longer delay times, and a brighter delay without aliasing.
* 'Poles' adjusts the slope of the anti-aliasing and reconstruction filters from 6 dB/Oct to 24 dB/Oct. A steeper slope gives a 'cleaner', 'duller' sound, a more shallow slope sounds 'dirtier' (with more audible aliasing), but brighter.
* 'Comp' adjusts the reaction time of the compander. Lower values will give the repeats a slight 'fade in' and 'fade out', and can give the delay a 'dreamy' character. This value also slightly affects the low-frequency response of the delay.

* 'Input' adjusts the input level to the delay. It does not affect the dry signal output. A small red LED indicates an overload of the delay circuit and will result in saturation.
* Turning up the 'Stereo' parameter gives a ping-pong delay. It has no effect when 'Inv R' is enabled.
* If 'Inv R' is enabled, the right wet signal will be the inverted left wet signal. You can use this with a small BBD size (e.g. 512), no feedback, and delay time modulation to create a chorus effect.
* 'Mix' adjusts the dry-wet balance.

### Drift
Drift generates subtle constant offset and drift.
The 'Poly' input determines the polyphony channels of the output.
* 'Const' adjusts the amount of a random (per voice) constant offset.
* 'RNG' generates a new set of random offsets. The offsets are stored with the patch.
* 'Drift' adjusts the amount of a random (per voice) drift.
* 'Drift Rate' adjusts the frequency of the random drift.

### Mod Matrix
A polyphonic modulation matrix with 13 inputs and 16 outputs.
It is fully MIDI-controllable with 16 knobs and 12 buttons.

The outputs are clamped to ±12V.

#### Usage
The recommended usage is to assign up to 16 knobs of your MIDI controller to the 'Control' knobs in the first row, and up to 12 buttons of your MIDI controller to the 'Select' buttons in the right column.
I recommend [stoermelder's 'MIDI-CAT'](https://library.vcvrack.com/Stoermelder-P1/MidiCat). 

Connect modulation sources to the 'Signal' inputs (leave input 0 disconnected), and modulation destinations to the 'Mix' outputs. 

When no buttons are pressed, the 'Control' knobs control the 'base' value from -5V to 5V (in bipolar mode), or 0V to 10V (in unipolar mode). The range of the base value can be changed by connecting the 'Signal 0' input.

Selecting a row by holding a button immediately changes the values of the 'Control' knobs to the values of the selected row. You can then control the selected row of the modulation matrix with your MIDI controller knobs.

Once you release the button, the values of the 'Control' knobs revert back to their previous 'base' values.

#### Context menu options
* 'Reduce internal sample rate': The internal sample rate can be reduced. Since modulation signals are usually not audio rate, this can be used to save CPU time.
* 'Latch buttons': The behavior of the buttons can be switched from momentary to latched (this is useful if you want to select the active row with a mouse click). Regardless of the mode, only one row can be selected for editing. The active row is indicated by a light.
* 'Bipolar': The behavior of the knobs can be switched between bipolar (-100% to 100% range) and unipolar (0 to 100% range).
* 'Relative MIDI control mode': If this is not checked, the controls work in absolute mode. This is ideal if you have a controller with encoders, and MIDI feedback.
    With a normal MIDI controller, you will get parameter jumps whenever the positions of your physical knobs don't match the control knob values in Rack.
    With 'Relative MIDI control mode' enabled, your physical knobs control the relative position, so you get no parameter jumps.
    Instead, it can happen that your physical knobs are at the end of their travel, but the on screen knob is not. In that case, you have to turn the physical knob back the whole way, and forth again. 

### OnePole
A simple CV controllable 1-pole highpass and lowpass filter.

### Oscillators
A pair of analog-style oscillators.
* 'Shape' blends from triangle to sawtooth to pulse wave.
* 'PW' adjusts the phase of the triangle, and the pulse wave from 0% to 100% duty cycle.
* 'Vol' adjusts the oscillator volume.
* 'Sub' adjusts the sub-oscillator volume, a square wave one octave below oscillator 1.
* 'Sync' hard syncs oscillator 2 to oscillator 1.
* 'FM' adjusts the (linear-through-zero) frequency modulation from oscillator 1 to oscillator 2.
Oscillator 2 stays in tune when applying FM.
* 'RM' adjusts the volume of the ring modulator (multiplication of oscillator 1 and 2).

Every parameter is CV controllable.

There are two 'V/Oct' inputs, one for each oscillator. You can use the 'Tune' module to add tuning controls.

'Out' outputs the mix of oscillator 1, the sub-oscillator, oscillator 2 and the ring modulator. The output is soft-clipped to ±10V.

#### Context menu options
* 'Oversampling rate': The oscillators use a naive implementation, which is quite CPU friendly, and can therefore be massively oversampled to reduce aliasing.
This is especially useful for FM and sync sounds.
With no oversampling, the oscillators alias a lot.
* 'DC blocker': FM and the ring modulator can create a DC offset. Therefore, a DC blocker is enabled by default, but can be disabled in the context menu.
* 'LFO mode' lets you use the module as an LFO. It lowers the frequencies of the oscillators by 7 octaves, and internally disables oversampling and the DC blocker.
An input of 0V gives a frequency of about 2 Hz.

### Tune
Tune by octaves, plus coarse and fine (1 semitone) tuning.

The ranges can be adjusted in the context menu.

The two 'V/Oct' and the 'Fine' input are added and tuned. The 'Fine' input can be used for subtle modulations in the semitone range.

The output is limited to ±12V, so a huge frequency range can be covered.
