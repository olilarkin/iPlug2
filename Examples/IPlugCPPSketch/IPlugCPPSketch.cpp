#include "IPlugCPPSketch.h"
#include "IPlug_include_in_plug_src.h"
#include "IControls.h"

#include "ReloadableClass.h"
#include "LiveGUI.h"

LiveGUI* app = nullptr;

ReloadableClass<LiveGUI> dylib;

IPlugCPPSketch::IPlugCPPSketch(const InstanceInfo& info)
: Plugin(info, MakeConfig(kNumParams, kNumPresets))
{
  dylib.willCloseDylib = [&] {
    if(GetUI()) {
      GetUI()->RemoveAllControls();
    }
  };
  
  dylib.reloaded = [&](LiveGUI* ptr) {
    app = ptr;
    
    if(GetUI()) {
      LayoutUI(GetUI());
    }
  };
  
  dylib.init("/Users/oli/Dev/iPlug2/Examples/IPlugCPPSketch/IPlugCPPSketchLiveGUI.h",
  {
   "Examples/IPlugCPPSketch/",
   "IPlug",
   "IPlug/Extras",
   "IGraphics",
   "IGraphics/Controls",
   "IGraphics/Platforms",
   "IGraphics/Drawing",
   "IGraphics/Extras",
   "Dependencies/IGraphics/NanoSVG/src",
   "Dependencies/IGraphics/NanoVG/src",
   "Dependencies/IGraphics/MetalNanoVG/src",
   "Dependencies/IGraphics/yoga",
   "Dependencies/IGraphics/yoga/yoga",
   "WDL",
   "WDL/lice",
   "WDL/swell",
   "Dependencies/Build/mac/include",
   "Dependencies/Extras/nlohmann"
  },
  "/Users/oli/Dev/iPlug2/Examples/IPlugCPPSketch/precompiled.h");
  
  GetParam(kGain)->InitDouble("Gain", 0., 0., 100.0, 0.01, "%");

  mMakeGraphicsFunc = [&]() {
    return MakeGraphics(*this, PLUG_WIDTH, PLUG_HEIGHT, PLUG_FPS, GetScaleForScreen(PLUG_HEIGHT));
  };

  mLayoutFunc = [&](IGraphics* pGraphics) {
    if(app)
      app->layout(pGraphics);
  };
}

void IPlugCPPSketch::ProcessBlock(double** inputs, double** outputs, int nFrames)
{
  const double gain = GetParam(kGain)->Value() / 100.;
  const int nChans = NOutChansConnected();
  
  for (int s = 0; s < nFrames; s++) {
    for (int c = 0; c < nChans; c++) {
      outputs[c][s] = inputs[c][s] * gain;
    }
  }
}

void IPlugCPPSketch::OnIdle()
{
  IGraphics* pGraphics = GetUI();
  
  if(pGraphics)
  {
    if(!pGraphics->LiveEditInProgress())
      dylib.checkForChanges();
  }
}
