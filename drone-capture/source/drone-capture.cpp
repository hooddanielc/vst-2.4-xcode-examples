#include <cmath>
#include <vector>
#include <string>
#include <memory>
#include <map>

#include <SDL2/SDL.h>

#include "public.sdk/source/vst2.x/audioeffectx.h"
#include "public.sdk/source/vst2.x/aeffeditor.h"

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

class DroneCaptureEditor : public AEffEditor {
public:
  ERect rect;

  DroneCaptureEditor(AudioEffect *effect) : AEffEditor(effect) {
    rect.left = rect.top = rect.right = rect.bottom = 0;
    effect->setEditor(this);

    if (SDL_Init(SDL_INIT_EVERYTHING) != 0){
      LogS("Error: SDL Could not initiate");
    }
  }

  virtual bool getRect(ERect** mRect) {
    LogS("Get Rect");
    *mRect = &rect;
    return true;
  }

  virtual bool open(void *window) {
    systemWindow = SDL_CreateWindow(
      "Hello World!",
      100,
      100,
      640,
      480,
      SDL_WINDOW_SHOWN
    );
    return true;
  }

  virtual void idle() {
    // check for SDL_Events
    SDL_Event e;

    while(SDL_PollEvent(&e)) {
      if (e.type == SDL_WINDOWEVENT) {
        switch (e.window.event) {
          case SDL_WINDOWEVENT_CLOSE:
            close();
        }
      }
    }
  }

  virtual void close() {
    SDL_DestroyWindow(static_cast<SDL_Window*>(systemWindow));
    systemWindow = 0;
  }

  ~DroneCaptureEditor () {
    SDL_Quit();
  }
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

    this->editor = new DroneCaptureEditor(this);
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
          channel2cache.push_back(channel2cache[i - 1]);
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

  virtual void process (float **inputs, float **outputs, long sampleFrames) {
    float *in1 = inputs[0];
    float *in2 = inputs[1];
    float *out1 = outputs[0];
    float *out2 = outputs[1];
    
    while (--sampleFrames >= 0) {
      (*out1++) += (*in1++);
      (*out2++) += (*in2++);
    }
  }

  virtual long dispatcher (long opCode, long index, long value, void *ptr, float opt) {
    return 0.0;
  }

  virtual void resume() {
    LogS("Resume");
  }

  virtual void suspend() {
    LogS("Suspend");
  }

  virtual void open() {
    LogS("Open");
  }

  virtual void close() {
    LogS("Close");
  }
};

AudioEffect* createEffectInstance(audioMasterCallback audioMaster) {
  return new DroneCapture(audioMaster);
}
