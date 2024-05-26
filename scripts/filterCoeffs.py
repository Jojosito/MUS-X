#!/usr/bin/python3

import numpy as np
import scipy
import matplotlib.pyplot as plt
import sys

originalSampleRate = 48000 #Hz
#originalSampleRate = 44100 #Hz
oversampling = 2**3

targetStopBandAttenuation = -60 #dB

doPlot = False


passband = int(originalSampleRate / 2)

print('pass band range: 0 - ' + str(passband))

totalCostEstimation = 0.

while oversampling > 1:
	if oversampling == 2 :
		passband = 20000
		print('pass band range: 0 - ' + str(passband))

	sampleRate = int(originalSampleRate * oversampling)
	stopBandStart = int(sampleRate/2 - passband)
	stopBandEnd = int(sampleRate/2)
	
	transBand = (sampleRate/2 - 2 * passband) / sampleRate
	
	print(str(oversampling) + "x -> " + str(int(oversampling/2)) + "x; sample rate = " + str(sampleRate) + ' stop band range: ' + str(stopBandStart) + ' - ' + str(stopBandEnd) + ' transition band: ' + str(transBand))
	
	# calculate filter coeffs
	order = 1
	while True:
	
		filterLength = int(order*4 - 1)
		cutoff = 0.25 # halfband filter
		
		fPass = cutoff - (transBand/2)
		fStop = cutoff + (transBand/2)
		fVec = [0, fPass, fStop, 0.5]
		aVec = [1, 0]
		HBFWeights = scipy.signal.remez(filterLength, fVec, aVec, weight=None, Hz=None, type='bandpass', maxiter=250, grid_density=oversampling*1024, fs=None)
		
		cleanedUpWeights = []
		for i in range(order) :
			cleanedUpWeights.append(HBFWeights[2*i])
		
		# get filter response
		w, h = scipy.signal.freqz(HBFWeights)
		x = w * sampleRate * 1.0 / (2 * np.pi)
		y = 20 * np.log10(abs(h))
		
		# attenuation
		maxAtt = -1e32
		for f,a in zip(x,y) :
			if f>stopBandStart :
				maxAtt = max(maxAtt, a)
				
		if maxAtt > targetStopBandAttenuation :
			order += 1
			continue	
			
		print('order: ', order)	
		#print('filter coefficients: ', HBFWeights)
		print('filter coefficients: ', cleanedUpWeights)
				
		print('stop band attenuation: ' + str(maxAtt))
		
		# plot filter response
		if doPlot : 
			plt.figure(figsize=(10,5))
			plt.semilogx(x, y)
			plt.ylabel('Amplitude [dB]')
			plt.xlabel('Frequency [Hz]')
			plt.title('Frequency response ' + str(oversampling) + "x -> " + str(int(oversampling/2)))
			plt.grid(which='both', linestyle='-', color='grey')
			plt.xticks([20000, 24000, 48000, 96000, 192000, 348000, 696000, 1392000, 2784000, 5568000, 11136000], ["20k", "24k", "48k", "96k", "192k", "348k", "696k", "1392k", "2784k", "5568k", "11136k"])
			#plt.xticks([24000, 48000], ["24k", "48k"])
			plt.show()
			
		
		break
		
	
	oversampling = int(oversampling / 2)
	totalCostEstimation += oversampling * (order+1)
	print()
	
print('totalCostEstimation: ' + str(totalCostEstimation))



