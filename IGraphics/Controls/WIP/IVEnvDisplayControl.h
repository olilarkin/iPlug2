#include "IControl.h"

BEGIN_IPLUG_NAMESPACE
BEGIN_IGRAPHICS_NAMESPACE

class IVEnvDisplayControl : public IControl
                          , public IVectorBase
                          , public IMultiTouchControlBase
{  
public:
//  enum EADSRPts { START, ATTACK, DECAY, SUSTAIN, RELEASE };
    
  IVEnvDisplayControl(const IRECT& bounds, const std::initializer_list<int>& params, const char* label = "")
  : IControl(bounds, params)
  , IVectorBase(DEFAULT_STYLE)
  , IMultiTouchControlBase()
  {
    AttachIControl(this, label);
  }
  
  void Draw(IGraphics& g) override
  {
    DrawBackGround(g, mRECT);
    DrawWidget(g);
    DrawLabel(g);
    
    if(mStyle.drawFrame)
      g.DrawRect(GetColor(kFR), mWidgetBounds, nullptr, mStyle.frameThickness);
  }
  
  void DrawWidget(IGraphics& g) override
  {
    UpdatePositions();
    
    float x, y;
    float px = mPoints[0].x;
    float py = mPoints[0].y;

    g.fillRect(GetColor(kBG), mRECT);

    for (int i = 0; i < NUM_DRAWN_POINTS_ADSR; i++)
    {
      x = mPoints[i+1].x;
      y = mPoints[i+1].y;

      if (i == SUSTAIN_START_PT) // start of sustain segment
      {
        g.DrawDottedLine(GetColor(kFG), px, py, x, y, nullptr, mStyle.frameThickness);
      }
      else if (i == (SUSTAIN_START_PT+1)) // end of sustain segment
      {
        g.DrawLine(GetColor(kX1), px, py, x, mPointBounds.B, nullptr, mStyle.frameThickness);
        break;
      }
      else
      {
        g.DrawLine(GetColor(kFG), px, py, x, y, nullptr, mStyle.frameThickness);
      }
      
      px = x;
      py = y;
    }

    for (int i = 0; i < NUM_DRAWN_POINTS_ADSR; i++)
    {
      x = mPoints[i].x;
      y = mPoints[i].y;

      g.fillCircle(GetColor(kFG), x, y, POINT_RADIUS);

      if (i == SUSTAIN_START_PT)
        g.DrawCircle(GetColor(kFG), x, y, POINT_RADIUS + 2);
    }
  }

  void UpdatePositions()
  {
    // set x positions
    mPoints[1].x = mPointBounds.L + POINT_RADIUS + CalcXPosition(GetValue(0));
    mPoints[2].x = mPoints[1].x + CalcXPosition(GetValue(1));
    mPoints[3].x = mPoints[2].x + (mMaxWidthForTimeSeg * 0.75); // sustain
    mPoints[4].x = mPoints[3].x + CalcXPosition(GetValue(3));
    
    // set y position
    mPoints[2].y = mPoints[3].y = mPointBounds.B - 1 - GetValue(2) * (mPointBounds.H()-1); // sustain
  }

  void OnResize() override
  {
    SetTargetRECT(MakeRects(mRECT));
    
    mMaxWidthForTimeSeg = (float) mWidgetBounds.W() / (NUM_DRAWN_POINTS_ADSR - 1);

    mPointBounds.L = (float) mWidgetBounds.L + POINT_RADIUS + 1;
    mPointBounds.B = (float) mWidgetBounds.B - POINT_RADIUS - 2;
    mPointBounds.T = (float) mWidgetBounds.T + POINT_RADIUS + 1;
    mPointBounds.R = (float) mWidgetBounds.R - POINT_RADIUS - 2;

    for (int i = 0; i < NUM_DRAWN_POINTS_ADSR ; i++)
    {
      mPoints[i].x = mPointBounds.L + POINT_RADIUS + ((float)i/NUM_DRAWN_POINTS_ADSR * (mWidgetBounds.W() - POINT_DIAMETER));
      mPoints[i].y = mPointBounds.B;
    }

    mPoints[1].y = mPointBounds.T;
    
    
    SetDirty(false);
  }
private:
  inline float CalcXPosition(float normalisedValue) { return powf(normalisedValue, 0.5f) * mMaxWidthForTimeSeg; }

  inline float linearMap(float input, float outputMin, float outputMax) { return outputMin+(outputMax-outputMin) * input; }

  static constexpr int NUM_DRAWN_POINTS_ADSR = 5;
  static constexpr int POINT_RADIUS = 2;
  static constexpr int POINT_DIAMETER = 4;
  static constexpr int SUSTAIN_START_PT = 2;

  IRECT mPointBounds;
  float mMaxWidthForTimeSeg;
  IVec2 mPoints[NUM_DRAWN_POINTS_ADSR]; // because of start and end points
};

END_IPLUG_NAMESPACE
END_IGRAPHICS_NAMESPACE
