using namespace iplug;
using namespace igraphics;

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

    LIVE_EDIT_CONTROL_START
    pGraphics->AttachControl(new IVButtonControl(LIVE_EDIT_RECT(136.50,230.73,236.50,330.73), SplashClickActionFunc, LIVE_EDIT_LABEL("IVButtonControl"))
    LIVE_EDIT_PROPS_START
    {"shadow_offset", 3.00f},
    {"colors", IVColorSpec{{0,0,0,0}, {255,200,200,200}, {255,240,240,240}, {255,70,70,70}, {10,0,0,0}, {60,0,0,0}, {255,0,0,0}, {255,0,255,0}, {255,0,0,255}, }},
    {"label_text", IText(14.00f, IColor(255,0,0,0), "Roboto-Regular", EAlign::Center, EVAlign::Top, 0.00f, IColor(255,255,255,255), IColor(255,0,0,0))},
    {"frame_thickness", 1.00f},
    {"widget_frac", 1.00f},
    {"draw_frame", true},
    {"emboss", false},
    {"draw_shadows", true},
    {"angle", 0.00f},
    {"value_text", IText(14.00f, IColor(255,0,0,0), "Roboto-Regular", EAlign::Center, EVAlign::Bottom, 0.00f, IColor(255,255,255,255), IColor(255,0,0,0))},
    {"roundness", 0.00f},
    {"show_value", true},
    {"show_label", true},
    LIVE_EDIT_PROPS_END
    LIVE_EDIT_CONTROL_END

    LIVE_EDIT_END
  }
};

