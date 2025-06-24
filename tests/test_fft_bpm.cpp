#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include <vector>
#include <cmath>
#include <fftw3.h>
#include <aubio/aubio.h>

// --- FFT test: pure sine wave →
TEST_CASE("FFT peak bin matches sine frequency") {
    const double sampleRate = 1024.0;
    const int N = 1024;
    const double freq = 50.0;  // Hz
    std::vector<double> in(N), out(N);
    // generate sine wave
    for(int i = 0; i < N; ++i)
        in[i] = std::sin(2*M_PI*freq*(i/sampleRate));
    // perform FFT
    fftw_plan p = fftw_plan_r2r_1d(N, in.data(), out.data(),
                                   FFTW_R2HC, FFTW_ESTIMATE);
    fftw_execute(p);
    fftw_destroy_plan(p);
    // find peak magnitude bin
    int peakBin = 1;
    double maxMag = 0;
    for(int k = 1; k < N/2; ++k) {
        double re = out[k];
        double im = (k == N/2 ? 0 : out[N-k]);
        double mag = std::hypot(re, im);
        if(mag > maxMag) { maxMag = mag; peakBin = k; }
    }
    double peakFreq = peakBin * (sampleRate/N);
    REQUIRE(std::fabs(peakFreq - freq) < 1.0);  // within 1 Hz
}

// --- BPM test: two impulses one second apart → ~60 BPM
TEST_CASE("BPM detection on two impulses") {
    const uint_t winSize = 1024, hopSize = 512, sr = 44100;
    aubio_tempo_t* tempo = new_aubio_tempo("default", winSize, hopSize, sr);
    REQUIRE(tempo != nullptr);
    fvec_t* in = new_fvec(hopSize);
    fvec_t* out = new_fvec(1);

    // feed silence for a bit
    for(int i=0;i<10;++i) {
        for(uint_t j=0;j<hopSize;++j) fvec_set_sample(in, 0, j);
        aubio_tempo_do(tempo, in, out);
    }
    // first impulse
    for(uint_t j=0;j<hopSize;++j)
        fvec_set_sample(in, (j==0 ? 1.0f : 0.0f), j);
    aubio_tempo_do(tempo, in, out);
    // feed silence for 1 second in hops
    int hopsPerSec = sr/hopSize;
    for(int i=0;i<hopsPerSec;++i) {
        for(uint_t j=0;j<hopSize;++j) fvec_set_sample(in, 0, j);
        aubio_tempo_do(tempo, in, out);
    }
    // second impulse
    for(uint_t j=0;j<hopSize;++j)
        fvec_set_sample(in, (j==0 ? 1.0f : 0.0f), j);
    aubio_tempo_do(tempo, in, out);

    double detectedBPM = aubio_tempo_get_bpm(tempo);
    // expect ~60 BPM (1 beat per second)
    REQUIRE(std::fabs(detectedBPM - 60.0) < 5.0);

    del_aubio_tempo(tempo);
    del_fvec(in);
    del_fvec(out);
}
