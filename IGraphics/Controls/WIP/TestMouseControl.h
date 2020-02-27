/*
 ==============================================================================

 This file is part of the iPlug 2 library. Copyright (C) the iPlug 2 developers.

 See LICENSE.txt for  more info.

 ==============================================================================
*/

#pragma once

/**
 * @file
 * @copydoc TestMouseControl
 */

#include "IControl.h"

/** Control to test mouse interaction
 *   @ingroup TestControls */
class TestMouseControl : public IControl
{
public:
  TestMouseControl(IRECT rect, int paramIdx = kNoParameter)
  : IControl(rect, paramIdx)
  {
    SetTooltip("TestMouseControl");
  }

  void Draw(IGraphics& g) override
  {
    g.DrawText(mText, GetCursorStr(mCursor), mRECT);
  }

  void OnMouseDown(float x, float y, const IMouseMod& mod)
  {
  }
  
  void OnMouseUp(float x, float y, const IMouseMod& mod)
  {
    
  }
  
  void OnMouseDrag(float x, float y, float dX, float dY, const IMouseMod& mod)
  {
    
  }
  
  void OnMouseDblClick(float x, float y, const IMouseMod& mod);
  {
  }
  
  void OnMouseWheel(float x, float y, const IMouseMod& mod, float d)
  {
  }

  void OnMouseOut() override
  {
  }
  
private:
  WDL_String mStr;
};
