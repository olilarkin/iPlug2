#include "IPlugEffect.h"
#include "IPlug_include_in_plug_src.h"
#include "IControls.h"

class PropControl : public IControl
{
public:
  PropControl(const IRECT& bounds, const IPropMap& properties)
  : IControl(bounds)
  {
    SetProperties(properties);
  }

  void Draw(IGraphics& g) override
  {
    g.FillRect(*GetProp<IColor>("bgcolor"), mRECT);
    g.PathRect(mRECT);
    g.PathFill(*GetProp<IPattern>("pattern"));
    g.FillRect(*GetProp<IColor>("fgcolor"), mRECT.GetCentredInside(*GetProp<IRECT>("rect")));
//    g.DrawText(*GetProp<IText>("text"), *GetProp<const char*>("str"), mRECT);
    g.DrawBitmap(*GetProp<IBitmap>("bmp"), mRECT, 0, 0);
  }
  
private:
};

class MyRoundPanelControl : public IPanelControl
{
public:
  MyRoundPanelControl(const IRECT& bounds, const IPropMap& props)
  : IPanelControl(bounds, props)
  {
  }
  
  void Draw(IGraphics& g) override
  {
    g.FillRoundRect(*GetProp<IColor>("color"), mRECT, *GetProp<float>("roundness"), nullptr);
  }
};

IPlugEffect::IPlugEffect(const InstanceInfo& info)
: Plugin(info, MakeConfig(kNumParams, kNumPrograms))
{
  GetParam(kGain)->InitDouble("Gain", 0., 0., 100.0, 0.01, "%");

#if IPLUG_EDITOR // http://bit.ly/2S64BDd
  mMakeGraphicsFunc = [&]() {
    return MakeGraphics(*this, PLUG_WIDTH, PLUG_HEIGHT, PLUG_FPS, 1.);
  };
  
  mLayoutFunc = [&](IGraphics* pGraphics) {
    pGraphics->AttachCornerResizer(EUIResizerMode::Scale, false);
    pGraphics->AttachPanelBackground(COLOR_GRAY);
    pGraphics->LoadFont("Roboto-Regular", ROBOTO_FN);
    const IRECT b = pGraphics->GetBounds();
    
    const IPropMap props =
    {
      {"bgcolor", COLOR_RED},
      {"fgcolor", COLOR_BLUE},
      {"text", DEFAULT_TEXT.WithSize(10.f).WithVAlign(EVAlign::Top)},
      {"str", "Hello World"},
      {"rect", IRECT::MakeXYWH(0, 0, 10, 10)},
      {"pattern", IPattern::CreateRadialGradient(b.GetCentredInside(100).MW(), b.GetCentredInside(100).MH(), 100.,
        {
          {COLOR_WHITE, 0.f},
          {COLOR_TRANSPARENT, 1.f}
        })},
      {"bmp", pGraphics->LoadBitmap("/Users/oli/Dev/iPlug2/Examples/IPlugControls/resources/img/button@2x.png")}
    };
    
    MyRoundPanelControl* pPanel;
    pGraphics->AttachControl(pPanel = new MyRoundPanelControl(b.GetCentredInside(300), {{"roundness", 30.f}}));
    pGraphics->AttachControl(new PropControl(b.GetCentredInside(100), props));
    
    pGraphics->AttachControl(new IVSliderControl(b.GetFromBottom(100.f), [pPanel](IControl* pSlider){
      pPanel->SetProp("roundness", (float) (pSlider->GetValue() * 100.f), true);
    }, "", DEFAULT_STYLE, false, EDirection::Horizontal));
    
//    pGraphics->AttachControl(new IVKnobControl(b.GetCentredInside(100).GetVShifted(-100), kGain));
//    pGraphics->AttachControl(new IPanelControl(b, COLOR_RED));

//    pGraphics->AttachControl(new IVColorSwatchControl(b.GetCentredInside(200)));
    
//    pGraphics->AttachControl(new IVSliderControl(b.GetCentredInside(30,100), [](IControl* pCaller){
//      dynamic_cast<IVSliderControl*>(pCaller)->SetColor(kX1, IColor::LinearInterpolateBetween(COLOR_RED, COLOR_ORANGE, pCaller->GetValue())); }
//      , "", DEFAULT_STYLE, false, EDirection::Vertical, DEFAULT_GEARING, 0.f, 15.f));
//
//    pGraphics->AttachControl(new IVSliderControl(b.GetCentredInside(30,100).GetHShifted(30), [](IControl* pCaller){
//      dynamic_cast<IVSliderControl*>(pCaller)->SetColor(kX1, IColor::LinearInterpolateBetween(COLOR_RED, COLOR_WHITE, pCaller->GetValue())); },
//      "", DEFAULT_STYLE, false, EDirection::Vertical, DEFAULT_GEARING, 0.f, 15.f));
//    pGraphics->AttachControl(new ILambdaControl(b.GetCentredInside(100),
//      [](ILambdaControl* pCaller, IGraphics& g, IRECT& rect) {
//
//        g.FillRect(COLOR_RED, rect.GetScaledAboutCentre(2.f));
//
//    }, DEFAULT_ANIMATION_DURATION, false /*loop*/, false /*start immediately*/));
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
