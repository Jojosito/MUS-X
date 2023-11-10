//#define DBG

/** Downsamples by a factor 2.
  * MAXINPUTLENGTH must be power of 2 and > 2*ORDER
  *
  * A half band FIR filter has coefficients e.g. [a, 0, b, 0.5, b, 0, a]
  * i.e. every second coefficient is 0, except the middle one, which is 0.5.
  * The other coefficients are symmetric.
  * For this example, ORDER is 2, and setCoeffs takes [a, b] as an argument
  * */
template <int MAXINPUTLENGTH, int ORDER, typename T = float>
struct HalfBandDecimator {
	static_assert(MAXINPUTLENGTH>0 && ((MAXINPUTLENGTH & (MAXINPUTLENGTH-1)) == 0), "MAXINPUTLENGTH must be power of 2");
	static_assert(MAXINPUTLENGTH > 4*ORDER, "MAXINPUTLENGTH too small for ORDER");

	T inBuffer[2*MAXINPUTLENGTH] = {0};
	float coeffs[ORDER] = {0};

	int inIndex = 0;

	HalfBandDecimator() {
		reset();
	}

	void reset() {
		inIndex = 0;
		std::memset(inBuffer, 0, sizeof(inBuffer));
	}

	void setCoeffs(const float* arg)
	{
		std::memcpy(&coeffs[0], arg, ORDER * sizeof(float));

#ifdef DBG
		{
			for (int i=0; i<ORDER; ++i)
			{
				assert(coeffs[i] != 0);
				assert(coeffs[i] < 0.5f);
				assert(coeffs[i] > -0.5f);
			}
		}
#endif

	}

	/** inputlength must be power of 2
	  * `out` will be filled u p to inputlength/2 */
	void process(T* out, const int inputlength) {
		// Perform convolution
		for (int o = 0; o < inputlength/2; o++) { // loop over output samples to be calculated

			out[o] = 0.5f * inBuffer[(inIndex + 2*o + 2*MAXINPUTLENGTH - (2*ORDER - 1)) & (2*MAXINPUTLENGTH-1)];
			for (int k = 0; k < ORDER; k++) { // loop over kernel
				out[o] += coeffs[k] *
						(inBuffer[(inIndex + 2*o + 2*MAXINPUTLENGTH                 - 2*k) & (2*MAXINPUTLENGTH-1)]  +
						 inBuffer[(inIndex + 2*o + 2*MAXINPUTLENGTH - (4*ORDER - 2) + 2*k) & (2*MAXINPUTLENGTH-1)]);
			}
		}

		// advance index
		inIndex = (inIndex + inputlength) & (2*MAXINPUTLENGTH-1);

#ifdef DBG
		assert(inputlength>0 && ((inputlength & (inputlength-1)) == 0));
		assert(inIndex + inputlength <= 2*MAXINPUTLENGTH);
#endif
	}

	T* getInputArray()
	{
		return &inBuffer[inIndex];
	}
};

template <typename T = float>
struct HalfBandDecimatorCascade {
	HalfBandDecimator<1024, 1, T> decimator1024; // decimate down to 512x
	HalfBandDecimator< 512, 1, T> decimator512; // decimate down to 256x
	HalfBandDecimator< 256, 1, T> decimator256; // decimate down to 128x

	HalfBandDecimator<128, 2, T> decimator128; // decimate down to 64
	HalfBandDecimator< 64, 2, T> decimator64; // decimate down to 32x
	HalfBandDecimator< 32, 2, T> decimator32; // decimate down to 16x

	HalfBandDecimator<16, 3, T> decimator16; // decimate down to 8x
	HalfBandDecimator<16, 3, T> decimator8; // decimate down to 4x

	HalfBandDecimator<32, 6, T> decimator4; // decimate down to 2x

	HalfBandDecimator<128, 22, T> decimator2; // decimate down to 1x

	T outBuffer[1];

	HalfBandDecimatorCascade() {
		// transition band: 0.49609375; stop band attenuation: -100 dB
		float coeffs256[1] = {0.2500094126245982};
		decimator1024.setCoeffs(coeffs256);
		decimator512.setCoeffs(coeffs256);
		decimator256.setCoeffs(coeffs256);

		// transition band: 0.46875; stop band attenuation: -113 dB
		float coeffs32[2] = {-0.03147685303471284, 0.2814757608962846};
		decimator128.setCoeffs(coeffs32);
		decimator64.setCoeffs(coeffs32);
		decimator32.setCoeffs(coeffs32);

		// transition band: 0.375; stop band attenuation: -94 dB
		float coeffs8[3] = {0.0071174511083665504, -0.05231169587073679, 0.2952039278517196};
		decimator16.setCoeffs(coeffs8);
		decimator8.setCoeffs(coeffs8);

		// transition band: 0.25; stop band attenuation: -104 dB
		float coeffs4[6] = {-0.00034696754642661164, 0.0025132384125000433, -0.010158904818112154, 0.030552323579531526, -0.08198144832535462, 0.3094188471351897};
		decimator4.setCoeffs(coeffs4);

		// transition band: 0.0625; stop band attenuation: -91 dB
		float coeffs2[22] = {-3.78984774783134e-05, 7.863590263960349e-05, -0.00015644609673577748, 0.000280705554967871, -0.00046880653545378965, 0.0007418705114406702, -0.0011250733106933466, 0.0016480557249014232, -0.0023455500288496497, 0.003258444430478961, -0.004435660674522054, 0.005937492977997016, -0.007841575438896764, 0.010253701642225216, -0.013328034730955753, 0.01730676011216719, -0.0226037708095706, 0.030000600584212778, -0.04117865039985172, 0.06052862461039207, -0.10419628766252699, 0.3176696599347577};
		decimator2.setCoeffs(coeffs2);
	}

	void reset() {
		decimator1024.reset();
		decimator512.reset();
		decimator256.reset();
		decimator128.reset();
		decimator64.reset();
		decimator32.reset();
		decimator16.reset();
		decimator8.reset();
		decimator4.reset();
		decimator2.reset();
	}

	T* getInputArray(int inputlength)
	{
		switch (inputlength)
		{
			case 1024:
				return decimator1024.getInputArray();
			case 512:
				return decimator512.getInputArray();
			case 256:
				return decimator256.getInputArray();
			case 128:
				return decimator128.getInputArray();
			case 64:
				return decimator64.getInputArray();
			case 32:
				return decimator32.getInputArray();
			case 16:
				return decimator16.getInputArray();
			case 8:
				return decimator8.getInputArray();
			case 4:
				return decimator4.getInputArray();
			case 2:
				return decimator2.getInputArray();
			case 1:
			default:
				return &outBuffer[0];
		}
	}

	T process(int inputlength) {
		switch (inputlength)
		{
			case 1024:
				decimator1024.process(decimator512.getInputArray(), 1024);
				[[fallthrough]];
			case 512:
				decimator512.process(decimator256.getInputArray(), 512);
				[[fallthrough]];
			case 256:
				decimator256.process(decimator128.getInputArray(), 256);
				[[fallthrough]];
			case 128:
				decimator128.process(decimator64.getInputArray(), 128);
				[[fallthrough]];
			case 64:
				decimator64.process(decimator32.getInputArray(), 64);
				[[fallthrough]];
			case 32:
				decimator32.process(decimator16.getInputArray(), 32);
				[[fallthrough]];
			case 16:
				decimator16.process(decimator8.getInputArray(), 16);
				[[fallthrough]];
			case 8:
				decimator8.process(decimator4.getInputArray(), 8);
				[[fallthrough]];
			case 4:
				decimator4.process(decimator2.getInputArray(), 4);
				[[fallthrough]];
			case 2:
				decimator2.process(outBuffer, 2);
		}

		return outBuffer[0];
	}
};
