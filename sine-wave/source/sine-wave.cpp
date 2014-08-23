#include <stdio.h>
#include <cmath>

#include "sine-wave.h"

void LogS(const char* szString) {
  FILE* pFile = fopen("/tmp/logFile.txt", "a");
  fprintf(pFile, "%s\n",szString);
  fclose(pFile);
}

void LogD(const char* str, double val) {
  FILE* pFile = fopen("/tmp/logFile.txt", "a");
  fprintf(pFile, "%s: %f\n", str, val);
  fclose(pFile);
}

SineWave::SineWave(audioMasterCallback audioMaster) : AudioEffectX(audioMaster, 0, 3) {
  size = 44100;
  cursor = 0;
  frequency = 200.0;

  setNumInputs(1); // mono input
  setNumOutputs(2); // stereo output
  setUniqueID('????'); // this should be unique, use the Steinberg web page for plugin Id registration
  resume();    // flush buffer
}

void SineWave::processReplacing (float** inputs, float** outputs, VstInt32 sampleFrames) {
  auto *in = inputs[0];
  auto *out1 = outputs[0];
  auto *out2 = outputs[1];

  static const double step = pow(2.0, 1.0 / 12.0);
  
  double freq = frequency * pow(step, 0);
  double samples_per_second = size;
  double samples_per_cycle = samples_per_second / freq;

  for (auto i = 0; i < sampleFrames; ++i) {
    double theta = 3.14159 * 2.0 / samples_per_cycle * static_cast<double>(cursor);
    float d = (*in++ + sin(theta)) / 2;
    *out1++ = d;
    
    if (out2) {
        *out2++ = d;
    }
    
    cursor = (cursor + 1) % size;
  }
}

AudioEffect* createEffectInstance (audioMasterCallback audioMaster) {
	return new SineWave (audioMaster);
}

