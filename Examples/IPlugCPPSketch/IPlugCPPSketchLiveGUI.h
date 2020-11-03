using namespace iplug;
using namespace igraphics;
//    pGraphics->AttachPanelBackground(COLOR_WHITE);

class IPlugCPPSketchLiveGUI : public LiveGUI
{
public:
  void Layout(void* pG) override
  {
    IGraphics* pGraphics = (IGraphics*) pG;
    auto smileyBitmap = pGraphics->LoadBitmap(SMILEY_FN);
    pGraphics->LoadFont("Roboto-Regular", ROBOTO_FN);
    pGraphics->AttachCornerResizer();
    pGraphics->EnableMouseOver(true);
    IRECT b = pGraphics->GetBounds().GetPadded(-10);

    LIVE_EDIT_INIT(__FILE__)

    LIVE_EDIT_CONTROL_START
    pGraphics->AttachPanelBackground(COLOR_ORANGE);
    LIVE_EDIT_CONTROL_END

    LIVE_EDIT_END
  }
};

