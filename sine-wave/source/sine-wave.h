#pragma once

#include "public.sdk/source/vst2.x/audioeffectx.h"

class SineWave : public AudioEffectX {
public:
  SineWave(audioMasterCallback);
  ~SineWave() {};

  /* AudioEffect */
  virtual void processReplacing (float** inputs, float** outputs, VstInt32 sampleFrames);

private:
  long size;
  long cursor;
  double frequency;
};
