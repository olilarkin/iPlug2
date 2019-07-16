#include "IPlugEffect.h"
#include "IPlug_include_in_plug_src.h"
#include "IControls.h"
#include "IPlugPaths.h"

IPlugEffect::IPlugEffect(IPlugInstanceInfo instanceInfo)
: IPLUG_CTOR(kNumParams, kNumPrograms, instanceInfo)
{
  GetParam(kGain)->InitDouble("Gain", 0., 0., 100.0, 0.01, "%");

#ifdef WEBSOCKET_SERVER
  WDL_String str;
  BundleResourcePath(str, GetBundleID());
  str.Append("/build-web");
  CreateServer(str.Get());
#endif
  
#if IPLUG_EDITOR // All UI methods and member variables should be within an IPLUG_EDITOR guard, should you want distributed UI
  mMakeGraphicsFunc = [&]() {
    return MakeGraphics(*this, PLUG_WIDTH, PLUG_HEIGHT, PLUG_FPS, 1.);
  };
  
  mLayoutFunc = [&](IGraphics* pGraphics) {
    pGraphics->AttachCornerResizer(EUIResizerMode::Scale, false);
    pGraphics->AttachPanelBackground(COLOR_RED);
    pGraphics->LoadFont("Roboto-Regular", ROBOTO_FN);
    pGraphics->HandleMouseOver(true);
    const IRECT b = pGraphics->GetBounds();
    IURLControl* pCtrl;
    pGraphics->AttachControl(pCtrl = new IURLControl(b, "Open Remote Editor", "http://localhost:8001", IText(12)));
    pCtrl->SetBoundsBasedOnTextDimensions();
    pGraphics->AttachControl(new IVKnobControl(b.GetCentredInside(100).GetVShifted(-100), kGain));
  };
#endif
}

#if IPLUG_DSP
void IPlugEffect::ProcessBlock(sample** inputs, sample** outputs, int nFrames)
{
  const double gain = GetParam(kGain)->Value() / 100.;
  const int nChans = NOutChansConnected();
  
  for (int s = 0; s < nFrames; s++) {
    for (int c = 0; c < nChans; c++) {
      outputs[c][s] = inputs[c][s] * gain;
    }
  }
}

void IPlugEffect::OnIdle()
{
  #ifdef WEBSOCKET_SERVER
  ProcessWebsocketQueue();
  #endif
}
#endif
