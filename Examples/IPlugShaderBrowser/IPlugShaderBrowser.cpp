#include "IPlugShaderBrowser.h"
#include "IPlug_include_in_plug_src.h"
#include "IControls.h"
#include <filesystem>
#include <fstream>
#include "IShaderControl.h"

IPlugShaderBrowser::IPlugShaderBrowser(const InstanceInfo& info)
: iplug::Plugin(info, MakeConfig(kNumParams, kNumPresets))
{
  GetParam(kGain)->InitDouble("Gain", 0., 0., 100.0, 0.01, "%");

#if IPLUG_EDITOR // http://bit.ly/2S64BDd
  mMakeGraphicsFunc = [&]() {
    return MakeGraphics(*this, PLUG_WIDTH, PLUG_HEIGHT, PLUG_FPS, GetScaleForScreen(PLUG_WIDTH, PLUG_HEIGHT));
  };
  
  mLayoutFunc = [&](IGraphics* pGraphics) {
    pGraphics->AttachCornerResizer(EUIResizerMode::Scale, false);
    pGraphics->AttachPanelBackground(COLOR_WHITE);
    pGraphics->LoadFont("Roboto-Regular", ROBOTO_FN);
    const IRECT b = pGraphics->GetBounds();
    
    WDL_String resourcePath;
    #ifdef OS_MAC
    BundleResourcePath(resourcePath, BUNDLE_ID);
    #else
    namespace fs = std::filesystem;
    fs::path mainPath(__FILE__);
    fs::path imgResourcesPath = mainPath.parent_path() / "Resources" / "shaders";
    resourcePath.Set(imgResourcesPath.string().c_str());
    #endif
    
    pGraphics->AttachControl(new IShaderControl(b, ""), 0);
  
    pGraphics->AttachControl(new IVDiskPresetManagerControl(b.GetFromTop(40).GetMidVPadded(15), resourcePath.Get(), "sksl", true, "", DEFAULT_STYLE, [pGraphics](const WDL_String& path) {
      WDL_String err;
      std::ifstream shaderFileStream(path.Get());

      if (!shaderFileStream.is_open())
      {
        DBGMSG("Failed to open shader file: %s\n", path.Get());
      }

      std::string shaderSrcStr((std::istreambuf_iterator<char>(shaderFileStream)), std::istreambuf_iterator<char>());
      pGraphics->GetControlWithTag(0)->As<IShaderControl>()->SetShaderStr(shaderSrcStr.c_str(), err);
      
      if (err.GetLength())
        DBGMSG("%s\n", err.Get());
      
      pGraphics->SetAllControlsDirty();
    }));
  };
#endif
}

#if IPLUG_DSP
void IPlugShaderBrowser::ProcessBlock(sample** inputs, sample** outputs, int nFrames)
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
