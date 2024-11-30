#include "IPlugEffect.h"
#include "IPlug_include_in_plug_src.h"
#include "IControls.h"

IPlugEffect::IPlugEffect(const InstanceInfo& info)
: iplug::Plugin(info, MakeConfig(kNumParams, kNumPresets))
{
  GetParam(kGain)->InitDouble("Gain", 0., 0., 100.0, 0.01, "%");

#if IPLUG_EDITOR // http://bit.ly/2S64BDd
  mMakeGraphicsFunc = [&]() {
    return MakeGraphics(*this, PLUG_WIDTH, PLUG_HEIGHT, PLUG_FPS, GetScaleForScreen(PLUG_WIDTH, PLUG_HEIGHT));
  };
  
  mLayoutFunc = [&](IGraphics* pGraphics) {
    pGraphics->AttachCornerResizer();
    pGraphics->AttachPanelBackground(COLOR_GRAY);
    pGraphics->LoadFont("Roboto-Regular", ROBOTO_FN);
    pGraphics->AttachControl(new ILambdaControl(pGraphics->GetBounds(),
                                                [&](ILambdaControl* pCaller, IGraphics& g, IRECT& r) {
                                                  if(mCount != mPrevCount) {
                                                    mStr.SetFormatted(32, "%i", mCount);
                                                    dynamic_cast<IPanelControl*>(g.GetBackgroundControl())->SetPattern(IColor::GetRandomColor());
                                                  }
                                                  
                                                  g.DrawText(IText(100), mStr.Get(), r);
                                                  mPrevCount = mCount;
                                                }, 100, true, true));
  };
  
  mSyncThingy.setBeatScalar(4.);
  mSyncThingy.setFunc([&]() {
    mCount++;
  });
#endif
}

#if IPLUG_DSP
void IPlugEffect::OnReset()
{
  mSyncThingy.setSampleRate(GetSampleRate());
}

void IPlugEffect::ProcessBlock(sample** inputs, sample** outputs, int nFrames)
{
  const double gain = GetParam(kGain)->Value() / 100.;
  const int nChans = NOutChansConnected();
 
  mSyncThingy.process(mTimeInfo.mPPQPos);
  
  for (int s = 0; s < nFrames; s++) {
    for (int c = 0; c < nChans; c++) {
      outputs[c][s] = inputs[c][s] * gain;
    }
  }
}
#endif
