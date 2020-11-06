/*
 ==============================================================================

 This file is part of the iPlug 2 library. Copyright (C) the iPlug 2 developers.

 See LICENSE.txt for  more info.

 ==============================================================================
*/

#pragma once

/**
 * @file
 * @ingroup Controls
 * @copydoc IVSpectrumControl
 */

#include "IControl.h"
#include "IPlugStructs.h"
#include "IPlugQueue.h"

BEGIN_IPLUG_NAMESPACE
BEGIN_IGRAPHICS_NAMESPACE

/** Vectorial multichannel capable spectrum display control
 * @ingroup IControls */
template <int MAXNC = 1, int MAXBUF = 128, int QUEUE_SIZE = 1024>
class IVSpectrumControl : public IControl
                        , public IVectorBase
{
public:
  /** Constructs an IVSpectrumControl
   * @param bounds The rectangular area that the control occupies
   * @param label A CString to label the control
   * @param style, /see IVStyle */
  IVSpectrumControl(const IRECT& bounds, const char* label = "", const IVStyle& style = DEFAULT_STYLE)
  : IControl(bounds)
  , IVectorBase(style)
  {
    AttachIControl(this, label);
  }
  
  void Draw(IGraphics& g) override
  {
//    DrawBackGround(g, mRECT);
    DrawWidget(g);
    DrawLabel(g);
    
    if(mStyle.drawFrame)
      g.DrawRect(GetColor(kFR), mWidgetBounds, nullptr, mStyle.frameThickness);
  }

  void DrawWidget(IGraphics& g) override
  {
  }
  
  void OnResize() override
  {
    SetTargetRECT(MakeRects(mRECT));
    SetDirty(false);
  }

  void OnMsgFromDelegate(int messageTag, int dataSize, const void* pData) override
  {
//    IByteStream stream(pData, dataSize);
//
//    int pos = stream.Get(&mBuf.nchans, 0);
//
//    while(pos < stream.Size())
//    {
//      for (auto ch = 0; ch < mBuf.nchans; ch++) {
//        for (auto s = 0; s < MAXBUF; s++) {
//          pos = stream.Get(&mBuf.vals[ch][s], pos);
//        }
//      }
//    }

    SetDirty(false);
  }

private:
  float mPadding = 2.f;
};

END_IGRAPHICS_NAMESPACE
END_IPLUG_NAMESPACE

