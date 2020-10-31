using namespace iplug;
using namespace igraphics;
//    pGraphics->AttachPanelBackground(COLOR_WHITE);

class IPlugCPPSketchLiveGUI : public LiveGUI
{
public:
  void layout(void* pG) override
  {
    IGraphics* pGraphics = (IGraphics*) pG;
    auto smileyBitmap = pGraphics->LoadBitmap(SMILEY_FN);
    pGraphics->LoadFont("Roboto-Regular", ROBOTO_FN);
    pGraphics->AttachCornerResizer();
    pGraphics->EnableMouseOver(true);
    IRECT b = pGraphics->GetBounds().GetPadded(-10);

    LIVE_EDIT_INIT(__FILE__)

    LIVE_EDIT_CONTROL_START
    pGraphics->AttachPanelBackground(COLOR_WHITE);
    LIVE_EDIT_CONTROL_END

    LIVE_EDIT_CONTROL_START
    pGraphics->AttachControl(new ICaptionControl(LIVE_EDIT_RECT(90.0, 140.0, 440.0, 430.0), kNoParameter));
    LIVE_EDIT_CONTROL_END

    LIVE_EDIT_CONTROL_START
    pGraphics->AttachControl(new IVButtonControl(LIVE_EDIT_RECT(60.0, 90.0, 230.0, 120.0), SplashClickActionFunc, "IVButtonControl"));
    LIVE_EDIT_CONTROL_END

    LIVE_EDIT_CONTROL_START
    pGraphics->AttachControl(new IVLabelControl(LIVE_EDIT_RECT(235.8, 141.9, 420.0, 190.0), "IVLabelControl"));
    LIVE_EDIT_CONTROL_END

    LIVE_EDIT_CONTROL_START
    pGraphics->AttachControl(new IBitmapControl(0.,0., pGraphics->LoadBitmap("/Users/oli/Dev/iPlug2/Projects/VirtualCZ/resources/img/Env_Loop@2x.png")));
    LIVE_EDIT_CONTROL_END

    LIVE_EDIT_CONTROL_START
    pGraphics->AttachControl(new IBitmapControl(LIVE_EDIT_RECT(320.0, 50.0, 920.0, 650.0), pGraphics->LoadBitmap("/Users/oli/Dev/iPlug2/Projects/VirtualCZ/resources/img/Env_EndPoint@2x.png")));
    LIVE_EDIT_CONTROL_END

    LIVE_EDIT_END
  }
};

