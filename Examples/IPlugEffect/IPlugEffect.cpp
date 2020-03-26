#include "IPlugEffect.h"
#include "IPlug_include_in_plug_src.h"
#include "IControls.h"
#include "SVF.h"

IPlugEffect::IPlugEffect(const InstanceInfo& info)
: iplug::Plugin(info, MakeConfig(kNumParams, kNumPresets))
{
  GetParam(kFreq)->InitFrequency("freq", 1000., 100., 15000.);
  GetParam(kQ)->InitPercentage("Q", 3., 0.1, 10.);
  GetParam(kType)->InitEnum("Type", SVF<>::EMode::kLowPass, { SVFMODES_VALIST });
  GetParam(kGain)->InitGain("Gain", 0., -30, 30);

#if IPLUG_EDITOR // http://bit.ly/2S64BDd
  mMakeGraphicsFunc = [&]() {
    return MakeGraphics(*this, PLUG_WIDTH, PLUG_HEIGHT, PLUG_FPS, GetScaleForScreen(PLUG_WIDTH, PLUG_HEIGHT));
  };
  
  mLayoutFunc = [&](IGraphics* pGraphics) {
    pGraphics->AttachCornerResizer(EUIResizerMode::Scale, false);
    pGraphics->AttachPanelBackground(COLOR_GRAY);
    pGraphics->LoadFont("Roboto-Regular", ROBOTO_FN);
    const IRECT b = pGraphics->GetBounds().GetPadded(-20.f);
    pGraphics->AttachControl(new IVPlotControl(b.GetFromTLHC(256, 256), { {COLOR_RED, [this](double x) {
        return SVF<>::PlotResponse(GetParam(kFreq)->Value(), GetParam(kQ)->Value(), (SVF<>::EMode) GetParam(kType)->Int(), x, GetParam(kGain)->Value());
      }} }, 256, "", DEFAULT_STYLE, -100.f, 100.));
    //pGraphics->AttachControl(new ITextControl(b.GetMidVPadded(50), "Hello iPlug 2!", IText(50)));
    pGraphics->AttachControl(new IVSliderControl(b.GetFromBottom(256).GetGridCell(0, 1, 3), kFreq, "", DEFAULT_STYLE, true, EDirection::Vertical))->SetActionFunction([](IControl* pCaller) {pCaller->GetUI()->SetAllControlsDirty(); });
    pGraphics->AttachControl(new IVSliderControl(b.GetFromBottom(256).GetGridCell(1, 1, 3), kQ, "", DEFAULT_STYLE, true, EDirection::Vertical))->SetActionFunction([](IControl* pCaller) {pCaller->GetUI()->SetAllControlsDirty(); });
    pGraphics->AttachControl(new IVSliderControl(b.GetFromBottom(256).GetGridCell(2, 1, 3), kGain, "", DEFAULT_STYLE, true, EDirection::Vertical))->SetActionFunction([](IControl* pCaller) {pCaller->GetUI()->SetAllControlsDirty(); });
    pGraphics->AttachControl(new IVRadioButtonControl(b.GetFromTRHC(256, 256), kType))->SetActionFunction([](IControl* pCaller) {pCaller->GetUI()->SetAllControlsDirty(); });
  };
#endif
}

#if IPLUG_DSP
void IPlugEffect::ProcessBlock(sample** inputs, sample** outputs, int nFrames)
{
  const double gain = 0.f;// GetParam(kGain)->Value() / 100.;
  const int nChans = NOutChansConnected();
  
  for (int s = 0; s < nFrames; s++) {
    for (int c = 0; c < nChans; c++) {
      outputs[c][s] = inputs[c][s] * gain;
    }
  }
}
#endif
