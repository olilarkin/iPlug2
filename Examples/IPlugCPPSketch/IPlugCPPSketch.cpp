#include "IPlugCPPSketch.h"
#include "IPlug_include_in_plug_src.h"
#include "LiveGUI.h"

LiveGUI* app = nullptr;

#ifndef HOT_RELOAD
#define HOT_RELOAD 0
#endif

#if HOT_RELOAD
#include "ReloadableClass.h"
#include "imgui_markdown.h"
ReloadableClass<LiveGUI> dylib;
#elif IPLUG_EDITOR
#include "IControls.h"
#include "IGraphicsLiveEdit.h"
#include "IPlugCPPSketchLiveGUI.h"
#endif

IPlugCPPSketch::IPlugCPPSketch(const InstanceInfo& info)
: Plugin(info, MakeConfig(kNumParams, kNumPresets))
{
  #if HOT_RELOAD
  dylib.willCloseDylib = [&] {
    if(GetUI()) {
      GetUI()->RemoveAllControls();
    }
  };
  
  dylib.reloaded = [&](LiveGUI* ptr) {
    app = ptr;
    
    if(GetUI()) {
      LayoutUI(GetUI());
      SendCurrentParamValuesFromDelegate();
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
#elif IPLUG_EDITOR
  app = new IPlugCPPSketchLiveGUI();
#endif
  
  GetParam(kGain)->InitDouble("Gain", 0., 0., 100.0, 0.01, "%");

  
#if IPLUG_EDITOR
  mMakeGraphicsFunc = [&]() {
#ifdef OS_WEB
    int w, h;
    GetScreenDimensions(w, h);
    return MakeGraphics(*this, w, h, 1.f);
#else
    return MakeGraphics(*this, PLUG_WIDTH, PLUG_HEIGHT, PLUG_FPS, GetScaleForScreen(PLUG_HEIGHT));
#endif
  };

  mLayoutFunc = [&](IGraphics* pGraphics) {
    if(app)
      app->Layout(pGraphics);
    #if !HOT_RELOAD
      pGraphics->AttachTextEntryControl();
      pGraphics->AttachPopupMenuControl();
      // pGraphics->EnableLiveEdit(true);
     pGraphics->AttachImGui([](IGraphics* pGraphics){
       static bool liveEdit = pGraphics->LiveEditEnabled();
       static bool showFPS = false;
       static bool showDrawnArea = false;
       static bool showControlBounds = false;
       static float bgColor [3] = {0.5f, 0.5f, 0.5f};
       if(ImGui::Checkbox("Live Edit", &liveEdit)) pGraphics->EnableLiveEdit(liveEdit);
       if(ImGui::Checkbox("Show FPS", &showFPS)) pGraphics->ShowFPSDisplay(showFPS);
       if(ImGui::Checkbox("Show Drawn Area", &showDrawnArea)) pGraphics->ShowAreaDrawn(showDrawnArea);
       if(ImGui::Checkbox("Show Ctrl Bounds", &showControlBounds)) pGraphics->ShowControlBounds(showControlBounds);
       ImGui::SetColorEditOptions(ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoInputs);
       // if(ImGui::ColorEdit3("BG Color", bgColor)) pGraphics->GetBackgroundControl()->As<IPanelControl>()->SetPattern(IColor::FromRGBf(bgColor));
     });
    #endif
  };
#endif
}

#if IPLUG_EDITOR
void IPlugCPPSketch::OnParentWindowResize(int width, int height)
{
  if(GetUI())
    GetUI()->Resize(width, height, 1.f, false);
}
#endif

#if IPLUG_DSP
void IPlugCPPSketch::ProcessBlock(iplug::sample** inputs, iplug::sample** outputs, int nFrames)
{
  const double gain = GetParam(kGain)->Value() / 100.;
  const int nChans = NOutChansConnected();
  
  for (int s = 0; s < nFrames; s++) {
    for (int c = 0; c < nChans; c++) {
      outputs[c][s] = inputs[c][s] * gain;
    }
  }
}
#endif

void IPlugCPPSketch::OnIdle()
{
#if HOT_RELOAD
  IGraphics* pGraphics = GetUI();
  
  if(pGraphics)
  {
    if(!pGraphics->LiveEditInProgress())
      dylib.checkForChanges();
  }
#endif
}
