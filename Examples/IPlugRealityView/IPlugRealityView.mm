#include "IPlugRealityView.h"
#include "IPlug_include_in_plug_src.h"

#ifdef FRAMEWORK_BUILD
#import <AUv3Framework/IPlugRealityView-Swift.h>
#import <AUv3Framework/IPlugRealityView-Shared.h>
#else
#import <IPlugRealityView-Swift.h>
#endif

IPlugRealityView::IPlugRealityView(const InstanceInfo& info)
: Plugin(info, MakeConfig(kNumParams, kNumPresets))
{
  GetParam(kParamGain)->InitDouble("Gain", 50., 0., 100.0, 0.01, "%");
  GetParam(kParamFreq)->InitDouble("Freq", 0., 0., 100.0, 0.01, "%");
  
  MakeDefaultPreset();
}

void IPlugRealityView::OnIdle()
{
  mSender.TransmitData(*this);
}

void* IPlugRealityView::OpenWindow(void* pParent)
{
  PLATFORM_VIEW* platformParent = (PLATFORM_VIEW*) pParent;
  IPlugRealityViewViewController* vc = [[IPlugRealityViewViewController alloc] init];
  [vc setEditorDelegate: this];
  mViewController = vc;
  vc.view.frame = MAKERECT(0.f, 0.f, (float) PLUG_WIDTH, (float) PLUG_HEIGHT);
  [platformParent addSubview:vc.view];
  
  OnUIOpen();
  
  return vc.view;
}

void IPlugRealityView::OnParentWindowResize(int width, int height)
{
  auto* vc = (IPlugRealityViewViewController*) mViewController;
  vc.view.frame = MAKERECT(0.f, 0.f, (float) width, (float) height);
}

// AUv3 only
bool IPlugRealityView::OnHostRequestingSupportedViewConfiguration(int width, int height)
{
#ifdef OS_MAC
  // Logic/GB offer one option with 0w, 0h, and if we allow that, our AUv3 has "our" size as its 100% setting
  return ((width + height) == 0);
#else
  return true;
#endif
}

void IPlugRealityView::ProcessBlock(sample** inputs, sample** outputs, int nFrames)
{
  const double gain = GetParam(kParamGain)->Value() / 100.;
  const int nChans = NOutChansConnected();
    
  for (int s = 0; s < nFrames; s++) {
    for (int c = 0; c < nChans; c++) {
      outputs[c][s] = inputs[0][s] * gain;
    }
  }
  
  mSender.ProcessBlock(outputs, nFrames, kCtrlTagVUMeter);
}

bool IPlugRealityView::OnMessage(int msgTag, int ctrlTag, int dataSize, const void* pData)
{
  if(msgTag == kMsgTagHello)
  {
    DBGMSG("MsgTagHello received on C++ side\n");
    return true;
  }
  else if(msgTag == kMsgTagRestorePreset)
  {
    RestorePreset(ctrlTag);
  }
  
  return CocoaEditorDelegate::OnMessage(msgTag, ctrlTag, dataSize, pData);
}
