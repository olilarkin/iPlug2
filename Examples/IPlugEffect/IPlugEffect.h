#pragma once

#include "IPlug_include_in_plug_hdr.h"

const int kNumPresets = 1;

enum EParams
{
  kGain = 0,
  kNumParams
};

class SyncThingy
{
private:
  double mPhaseStep; // (1./sr);
  double mPhase = 0.; // float phase (goes between 0. and 1.)
  double mPrevPhase = 1.; // for delta calc
  double mBeatScalar = 1.;
  std::function<void()> mFunc = nullptr;
  
public:
  void setSampleRate(double sr)  { mPhaseStep = 1./sr; }
  
  inline void setBeatScalar(double scalar) { mBeatScalar = scalar; }
  
  void setFunc(std::function<void()> func) { mFunc = func; }
  
  void process(double ppqPos) {
    if(mFunc) {
      auto wrap = [](double x, double lo = 0., double hi = 1.) {
        while (x >= hi) x -= hi;
        while (x < lo)  x += hi - lo;
        
        return x;
      };
      
      double oneOverBS = 1./mBeatScalar;
      
      // set phase
      double beatPhase = std::fmod(ppqPos, oneOverBS) / oneOverBS;
      mPhase = wrap(beatPhase);
      
      if ((mPhase - mPrevPhase) < -0.5)
          mFunc();

      mPrevPhase = mPhase;
    }
  }
};


using namespace iplug;
using namespace igraphics;

class IPlugEffect final : public Plugin
{
public:
  IPlugEffect(const InstanceInfo& info);

  void ProcessBlock(sample** inputs, sample** outputs, int nFrames) override;
  void OnReset() override;
  
private:
  SyncThingy mSyncThingy;
  int mCount = 0;
  int mPrevCount = 0;
  WDL_String mStr;
};
