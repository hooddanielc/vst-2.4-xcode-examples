#include <cmath>
#include <vector>
#include <string>
#include <memory>
#include <map>

#include "public.sdk/source/vst2.x/audioeffectx.h"

using namespace std;

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

class Parameter {
public:
  Parameter(string mName = "none", float mValue = 0) : name(mName), value(mValue) {}
  void setup(string mName, float mValue) {
    name = mName;
    value = mValue;
  };
  ~Parameter() {}
  float value;
  string name;
};

class DroneCapture : public AudioEffectX {
private:
  long size;
  long cursor;
  double frequency;
  map<int, Parameter> parameters;

  vector<float> channel1cache;
  vector<float> channel2cache;
  bool recording = false;

public:
  DroneCapture(audioMasterCallback) : AudioEffectX(audioMaster, 0, 2) {
    parameters[0].setup("capture", 0);
    parameters[1].setup("reset", 0);
    size = getSampleRate();
    cursor = 0;
    frequency = 200.0;
    setNumInputs(2);
    setNumOutputs(2);
    setUniqueID('1234');
    resume();
  }

  ~DroneCapture() {};

  /* AudioEffect */
  virtual void getParameterName (VstInt32 index, char *text) {
    strcpy(text, parameters[index].name.c_str());
  }

  virtual void setParameter(VstInt32 index, float value) {
    parameters[index].value = value;
  }

  virtual void processReplacing(float** inputs, float** outputs, VstInt32 sampleFrames) {
    auto *in1 = inputs[0];
    auto *in2 = inputs[1];
    auto *out1 = outputs[0];
    auto *out2 = outputs[1];

    if (parameters[1].value == 1.0) {
      channel1cache.clear();
      channel2cache.clear();
    }

    if (parameters[0].value > 0.5) {
      // recordAudio
      if (!recording) {
        channel1cache.clear();
        channel2cache.clear();
        recording = true;
      }

      for (auto i = 0; i < sampleFrames; ++i) {
        channel1cache.push_back(*in1++);
        channel2cache.push_back(*in2++);
      }
    } else {
      if (recording) {
        cursor = 0;
        recording = false;

        // append reversed version of cache
        for (auto i = channel1cache.size(); i != 0; --i) {
          channel1cache.push_back(channel1cache[i - 1]);
          channel2cache.push_back(channel1cache[i - 1]);
        }
      }

      if (channel1cache.size() > 0) {
        for (auto i = 0; i < sampleFrames; ++i) {
          if (++cursor == channel1cache.size()) {
            cursor = 0;
          }

          *out1++ = channel1cache[cursor];
          *out2++ = channel2cache[cursor];
        }
      }
    }
  }

  virtual void process (float **inputs, float **outputs, long sampleFrames) {}
};

AudioEffect* createEffectInstance(audioMasterCallback audioMaster) {
  return new DroneCapture(audioMaster);
}
