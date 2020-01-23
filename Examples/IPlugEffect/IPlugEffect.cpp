#include "IPlugEffect.h"
#include "IPlug_include_in_plug_src.h"
#include "IControls.h"
#include "IPopupMenuControl.h"


class BlehMenu : public IPopupMenuControl
{
public:
  BlehMenu(const IRECT& bounds, const IRECT& expanded)
  : IPopupMenuControl(kNoParameter, {50.f, EAlign::Center}, bounds, expanded)
  {
    mHide = false;
    SetPopupMenu(mThisMenu);
  }
  
  void OnMouseDown(float x, float y, const IMouseMod &mod) override
  {
    if(!GetExpanded())
      CreatePopupMenu(mThisMenu, mSpecifiedExpandedBounds.GetTranslated(0, -mSpecifiedExpandedBounds.H()));
    else
      IPopupMenuControl::OnMouseDown(x, y, mod);
  }
private:
  IPopupMenu mThisMenu {"Test", {"One", "Two", "Three"}};
};

IPlugEffect::IPlugEffect(const InstanceInfo& info)
: Plugin(info, MakeConfig(kNumParams, kNumPrograms))
{
  GetParam(kGain)->InitDouble("Gain", 0., 0., 100.0, 0.01, "%");

#if IPLUG_EDITOR // All UI methods and member variables should be within an IPLUG_EDITOR guard, should you want distributed UI
  mMakeGraphicsFunc = [&]() {
    return MakeGraphics(*this, PLUG_WIDTH, PLUG_HEIGHT, PLUG_FPS, 1.);
  };
  
  mLayoutFunc = [&](IGraphics* pGraphics) {
    pGraphics->AttachCornerResizer(EUIResizerMode::Scale, false);
    pGraphics->AttachPanelBackground(COLOR_GRAY);
    pGraphics->HandleMouseOver(true);
    pGraphics->LoadFont("Roboto-Regular", ROBOTO_FN);
    const IRECT b = pGraphics->GetBounds();
    // pGraphics->AttachControl(new ITextControl(b.GetMidVPadded(50), "Hello iPlug 2!", IText(50)));
//    pGraphics->AttachControl(new MyNewControl(b.GetCentredInside(300)), kNoTag, "Experiment");
    pGraphics->AttachControl(new BlehMenu(b.GetCentredInside(100), b.GetCentredInside(300)));
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
#endif
