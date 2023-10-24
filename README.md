# MUS-X

## Modules

### ADSR
ADSR envelope generator with exponential decay/release, built in velocity scaling and sustain CV.
During the decay and release phase, a gate signal is output. This can e.g. be used to trigger another envelope generator, that shapes the sustain.

### Oscillators
A pair of analog-style oscillators.
'Shape' blends from sawtooth to pulse wave.
'PW' adjusts the pulse wave from 0% to 100% duty cycle.
'Vol' adjusts the oscillator volume.
'Sync' hard syncs oscillator 2 to oscillator 1.
'FM' adjusts the (exponential) frequency modulation (also known as crossmod) from oscillator 1 to oscillator 2.
'RM' adjusts the volume of the ring modulator (multiplication of oscillator 1 and 2).

Every parameter (except sync) is CV controllable.
There are two 'V/Oct' inputs, one for each oscillator. You can use the 'Tune' module to add tuning controls.
'Out' outputs the mix of oscillator 1, oscillator 2 and the ring modulator.

The oscillators use a naive implementation, which is quite CPU friendly, and can therefore be massively oversampled.
This is especially useful if you want to emulate analog crossmod without aliasing.
Right click on the module to adjust the oversampling factor.

### Tune
Tune a V/Oct signal by +-4 octaves, plus coarse (1 octave) and fine (1 semitone) tuning.
The 'Fine' input is added and can be used for subtle modulations in the semitone range.
The output is limited to +-10V, so a huge frequency range can be covered.