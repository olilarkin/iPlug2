#pragma once

#include "IPlug_include_in_plug_hdr.h"

const int kNumPresets = 1;

enum EParams
{
  kGain = 0,
  kNumParams
};

using namespace iplug;
using namespace igraphics;

class IPlugCPPSketch final : public Plugin
{
public:
  IPlugCPPSketch(const InstanceInfo& info);
  void OnIdle() override;

#if IPLUG_EDITOR
  void OnParentWindowResize(int width, int height) override;
#endif
  
#if IPLUG_DSP
  void ProcessBlock(sample** inputs, sample** outputs, int nFrames) override;
#endif
};
