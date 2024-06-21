#!/usr/bin/env python3

filters = [
	'filter1Pole',
	'filter1Pole',	
	'ladderFilter2Pole',
	'ladderFilter2Pole',
	'ladderFilter4Pole',
	'ladderFilter4Pole',
	'ladderFilter4Pole',
	'ladderFilter4Pole',
	'sallenKeyFilterLpBp',
	'sallenKeyFilterLpBp',
	'sallenKeyFilterHp',
	'sallenKeyFilterHp',
	'diodeClipper',
	'diodeClipperAsym',
]

functions = [
	'lowpass',
	'highpass',	
	'lowpass',
	'bandpass',
	'lowpass6',
	'lowpass12',
	'lowpass18',
	'lowpass24',
	'lowpass',
	'bandpass',
	'highpass6',
	'highpass12',
	'out',
	'out',
]

integratorTypes = [
	'_linear',
	'_ota_tanh',
	'_ota_alt',
	'_transistor_tanh',
	'_transistor_alt',
]

methods = [
	'processEuler',
	'processRK2',
	'processRK4',
]



print("void setCutoffFrequencyAndResonance(float_4 frequency, float_4 resonance)")
print("{")
print("\tswitch (switchValue)")
print("\t{")
for iFilter in range(len(filters)):
	for iIntegratorType in range(len(integratorTypes)):
		for iMethod in range(len(methods)):
			print("\tcase " + str(iFilter*100 + iIntegratorType*10 + iMethod) + ":")
		print("\t\t" + filters[iFilter] + integratorTypes[iIntegratorType] + ".setCutoffFreq(frequency);")
		print("\t\t" + filters[iFilter] + integratorTypes[iIntegratorType] + ".setResonance(resonance);")
		print("\t\tbreak;")
print("\t}")
print("}\n")



print("float_4 process(float_4 in, float_4 dt)")
print("{")
print("\tswitch (switchValue)")
print("\t{")
for iFilter in range(len(filters)):
	for iIntegratorType in range(len(integratorTypes)):
		for iMethod in range(len(methods)):
			print("\tcase " + str(iFilter*100 + iIntegratorType*10 + iMethod) + ":")
			print("\t\t" + filters[iFilter] + integratorTypes[iIntegratorType] + "." + methods[iMethod] + "(in, dt);")
			print("\t\treturn " + filters[iFilter] + integratorTypes[iIntegratorType] + "." + functions[iFilter] + "();")
print("\tdefault:")
print("\t\treturn in;")
print("\t}")
print("}\n")



print("void processBlock(float_4* in, float_4 dt, int oversamplingRate)")
print("{")
print("\tswitch (switchValue)")
print("\t{")
for iFilter in range(len(filters)):
	for iIntegratorType in range(len(integratorTypes)):
		for iMethod in range(len(methods)):
			print("\tcase " + str(iFilter*100 + iIntegratorType*10 + iMethod) + ":")
			
			print("\t\tfor (int i = 0; i < oversamplingRate; ++i)")
			print("\t\t{")
			
			print("\t\t\t" + filters[iFilter] + integratorTypes[iIntegratorType] + "." + methods[iMethod] + "(in[i], dt);")
			print("\t\t\tin[i] = " + filters[iFilter] + integratorTypes[iIntegratorType] + "." + functions[iFilter] + "();")
			
			print("\t\t}")
			print("\t\tbreak;")
print("\tdefault:")
print("\t\treturn;")
print("\t}")

print("}")








