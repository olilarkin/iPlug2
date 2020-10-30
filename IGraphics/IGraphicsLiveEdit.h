/*
 ==============================================================================

 This file is part of the iPlug 2 library. Copyright (C) the iPlug 2 developers.

 See LICENSE.txt for  more info.

 ==============================================================================
*/

#pragma once

/**
 * @file
 * @copydoc IGraphicsLiveEdit
 */

#ifndef NDEBUG

#include "IControl.h"
#include <fstream>

BEGIN_IPLUG_NAMESPACE
BEGIN_IGRAPHICS_NAMESPACE

/** A control to enable live modification of control layout in an IGraphics context in debug builds
 * This is based on the work of Youlean, who first included it in iPlug-Youlean
 * The lives outside the main IGraphics control stack and it can be added with IGraphics::EnableLiveEdit().
 * It should not be used in the main control stack.
 * @ingroup SpecialControls */

/** All attached controls should be inside LIVE_EDIT_INIT and LIVE_EDIT_END
 * All controls should be wrapped with LIVE_EDIT_CONTROL_START and LIVE_EDIT_CONTROL_END
 * LIVE_EDIT_RECT should be used for the IRECT
 *
 * All macros should be placed on the new line. */

#define LIVE_EDIT_INIT(p) pGraphics->SetLiveEditSourcePath(p);
#define LIVE_EDIT_END

#define LIVE_EDIT_CONTROL_START
#define LIVE_EDIT_CONTROL_END

#define LIVE_EDIT_RECT(L, T, R, B) IRECT(L, T, R, B)


class IGraphicsLiveEditSourceEditor
{
public:
  IGraphicsLiveEditSourceEditor(const char* liveEditSourcePath)
  {
    mLiveEditSourcePath = liveEditSourcePath;
   
    ReadSourceFile();

    //int startsNumber = FindNumberOf("LIVE_EDIT_CONTROL_START");
    //int endsNumber = FindNumberOf("LIVE_EDIT_CONTROL_END");
  }

  void UpdateControlRectSource(int controlIndex, IRECT r)
  {
    int sourceControlIndexStart = FindSourceIndex(controlIndex, "LIVE_EDIT_CONTROL_START");
    int sourceControlIndexEnd = FindSourceIndex(controlIndex, "LIVE_EDIT_CONTROL_END");

    if (controlIndex == -1 || sourceControlIndexStart == -1 || sourceControlIndexEnd == -1) return;

    for (int i = sourceControlIndexStart + 1; i < sourceControlIndexEnd; i++)
    {
      WDL_String rectSource;
      rectSource.SetFormatted(128, "LIVE_EDIT_RECT(%i, %i, %i, %i)", (int)r.L, (int)r.T, (int)r.R, (int)r.B);

      ReplaceSourceText(mSourceFile[i], "LIVE_EDIT_RECT", ")", rectSource.Get());
    }

    WriteSourceFile();
  }

  void AddControlToSource(const char* controlConstructor, IRECT r)
  {
    int numberOfEnds = FindNumberOf("LIVE_EDIT_CONTROL_END");
    int appendToSourceIndex = 0;

    if (numberOfEnds == 0)
      appendToSourceIndex = FindSourceIndex(0, "LIVE_EDIT_INIT");
    
    else
      appendToSourceIndex = FindSourceIndex(numberOfEnds - 1, "LIVE_EDIT_CONTROL_END");
    
    std::vector<std::string> controlSource;

    controlSource.push_back("");
    controlSource.push_back("    LIVE_EDIT_CONTROL_START;");
    controlSource.push_back("    pGraphics->AttachControl(new ");
    controlSource.back().append(controlConstructor);
    controlSource.back().append(");");

    // Add live edit rect values
    WDL_String rectSource;
    rectSource.SetFormatted(128, "LIVE_EDIT_RECT(%i, %i, %i, %i)", (int)r.L, (int)r.T, (int)r.R, (int)r.B);
    ReplaceSourceText(controlSource.back(), "LIVE_EDIT_RECT", ")", rectSource.Get());

    controlSource.push_back("    LIVE_EDIT_CONTROL_END;");

    // Add to source
    mSourceFile.insert(mSourceFile.begin() + appendToSourceIndex + 1, controlSource.begin(), controlSource.end());

    WriteSourceFile();
  }

  void RemoveControlFromSource(int controlIndexToRemove)
  {
    // To compensate for the background control
    controlIndexToRemove -= 1;

    int sourceControlIndexStart = FindSourceIndex(controlIndexToRemove, "LIVE_EDIT_CONTROL_START");
    int sourceControlIndexEnd = FindSourceIndex(controlIndexToRemove, "LIVE_EDIT_CONTROL_END");

    if (sourceControlIndexStart == -1 || sourceControlIndexEnd == -1) return;

    mSourceFile.erase(mSourceFile.begin() + sourceControlIndexStart - 1, mSourceFile.begin() + sourceControlIndexEnd + 1);

    WriteSourceFile();
  }
  
private:
  void ReplaceSourceText(std::string &textToBeReplaced, std::string start, std::string end, std::string replaceWith)
  {
    size_t startPos = textToBeReplaced.find(start);
    size_t endPos = textToBeReplaced.find(end);

    if (startPos == std::string::npos || endPos == std::string::npos) return;

    textToBeReplaced.replace(startPos, endPos - startPos + 1, replaceWith);
  }

  int FindNumberOf(const char* stringToFind)
  {
    int foundNumber = 0;

    for (size_t i = 0; i < mSourceFile.size(); i++)
    {
      size_t pos = mSourceFile[i].find(stringToFind);

      if (pos != std::string::npos)
        if (!IsLineCommented(i, pos))
          foundNumber++;
    }

    return foundNumber;
  }

  int FindSourceIndex(int index, const char* stringToFind)
  {
    int foundNumber = 0;

    for (size_t i = 0; i < mSourceFile.size(); i++)
    {
      size_t pos = mSourceFile[i].find(stringToFind);

      if (pos != std::string::npos)
        if (!IsLineCommented(i, pos))
          foundNumber++;

      if (foundNumber - 1 == index)
        return (int) i;
    }

    return -1;
  }

  bool IsLineCommented(size_t lineIndex, size_t endCharIndex)
  {
    size_t commentPosition = mSourceFile[lineIndex].find("//");

    if (commentPosition != std::string::npos && commentPosition < endCharIndex)
      return true;

    return false;
  }

  void ReadSourceFile()
  {
    mSourceFile.resize(0);

    std::string line;
    std::ifstream file(mLiveEditSourcePath);

    if (file.is_open())
    {
      while (getline(file, line))
      {
        mSourceFile.push_back(line);
      }
      file.close();
    }
  }

  void WriteSourceFile()
  {
    std::string data;

    for (size_t i = 0; i < mSourceFile.size(); i++)
    {
      data.append(mSourceFile[i]);
      data.append("\n");
    }

    std::ofstream file(mLiveEditSourcePath);

    if (file.is_open())
    {
      file << data.c_str();
      file.close();
    }
  }

  std::vector<std::string> mSourceFile;
  std::string mLiveEditSourcePath;
};

class IGraphicsLiveEdit : public IControl
{
public:
  IGraphicsLiveEdit(bool mouseOversEnabled, const char* liveEditSourcePath)
  : IControl(IRECT())
  , mSourceEditor(liveEditSourcePath)
  , mGridSize(10)
  , mMouseOversEnabled(mouseOversEnabled) 
  {
    mTargetRECT = mRECT;
  }
  
  ~IGraphicsLiveEdit()
  {
    GetUI()->EnableMouseOver(mMouseOversEnabled); // Set it back to what it was
  }
  
  void OnInit() override
  {
    GetUI()->EnableMouseOver(true);
  }

  void OnMouseDown(float x, float y, const IMouseMod& mod) override
  {
    int c = GetUI()->GetMouseControlIdx(x, y, true);
    
    if (c > 0)
    {
      IControl* pControl = GetUI()->GetControl(c);
      mMouseDownRECT = pControl->GetRECT();
      mMouseDownTargetRECT = pControl->GetTargetRECT();

      if(!mod.S)
        mSelectedControls.Empty();
      
      mSelectedControls.Add(pControl);

      if(mod.A)
      {
        GetUI()->AttachControl(new PlaceHolder(mMouseDownRECT));
        mClickedOnControl = GetUI()->NControls() - 1;
        mMouseClickedOnResizeHandle = false;

        mSourceEditor.AddControlToSource("PlaceHolder(LIVE_EDIT_RECT())", mMouseDownRECT);
      }
      else if (mod.R)
      {
        mClickedOnControl = c;
        GetUI()->CreatePopupMenu(*this, mRightClickOnControlMenu, x, y);
      }
      else if (mod.R)
      {
        mClickedOnControl = c;
        GetUI()->CreatePopupMenu(*this, mRightClickOnControlMenu, x, y);
      }
      else
      {
        mClickedOnControl = c;
        
        if(GetHandleRect(mMouseDownRECT).Contains(x, y))
        {
          mMouseClickedOnResizeHandle = true;
        }
      }
    }
    else if(mod.R)
    {
      GetUI()->CreatePopupMenu(*this, mRightClickOutsideControlMenu, x, y);
    }
    else
    {
      mSelectedControls.Empty();
      mDragRegion.L = mDragRegion.R = x;
      mDragRegion.T = mDragRegion.B = y;
    }
  }
  
  void OnMouseUp(float x, float y, const IMouseMod& mod) override
  {
    if(mMouseClickedOnResizeHandle)
    {
      IControl* pControl = GetUI()->GetControl(mClickedOnControl);
      IRECT r = pControl->GetRECT();
      float w = r.R - r.L;
      float h = r.B - r.T;
      
      if(w < 0.f || h < 0.f)
      {
        pControl->SetRECT(mMouseDownRECT);
        pControl->SetTargetRECT(mMouseDownTargetRECT);
      }
    }
    mClickedOnControl = -1;
    mMouseClickedOnResizeHandle = false;
    GetUI()->SetAllControlsDirty();
    
    mDragRegion = IRECT();
  }
  
  void OnMouseDblClick(float x, float y, const IMouseMod& mod) override
  {
  }
  
  void OnMouseOver(float x, float y, const IMouseMod& mod) override
  {
    int c = GetUI()->GetMouseControlIdx(x, y, true);
    if (c > 0)
    {
      IRECT cr = GetUI()->GetControl(c)->GetRECT();
      IRECT h = GetHandleRect(cr);
      
      if(h.Contains(x, y))
      {
        GetUI()->SetMouseCursor(ECursor::SIZENWSE);
        return;
      }
      else
        GetUI()->SetMouseCursor(ECursor::HAND);
    }
    else
      GetUI()->SetMouseCursor(ECursor::ARROW);
  }
  
  void OnMouseDrag(float x, float y, float dX, float dY, const IMouseMod& mod) override
  {
    float mouseDownX, mouseDownY;
    GetUI()->GetMouseDownPoint(mouseDownX, mouseDownY);
    
    if(mClickedOnControl > 0)
    {
      IControl* pControl = GetUI()->GetControl(mClickedOnControl);
      
      if(mMouseClickedOnResizeHandle)
      {
        IRECT r = pControl->GetRECT();
        r.R = SnapToGrid(mMouseDownRECT.R + (x - mouseDownX));
        r.B = SnapToGrid(mMouseDownRECT.B + (y - mouseDownY));
        
        if(r.R < mMouseDownRECT.L +mGridSize) r.R = mMouseDownRECT.L+mGridSize;
        if(r.B < mMouseDownRECT.T +mGridSize) r.B = mMouseDownRECT.T+mGridSize;
          
        GetUI()->SetControlSize(mClickedOnControl, r.W(), r.H());
      }
      else
      {
        const float x1 = SnapToGrid(mMouseDownRECT.L + (x - mouseDownX));
        const float y1 = SnapToGrid(mMouseDownRECT.T + (y - mouseDownY));
          
        GetUI()->SetControlPosition(mClickedOnControl, x1, y1);
      }
      
      mSourceEditor.UpdateControlRectSource(GetUI()->GetControlIdx(pControl), pControl->GetRECT());
      
      GetUI()->SetAllControlsDirty();
    }
    else
    {
      float mouseDownX, mouseDownY;
      GetUI()->GetMouseDownPoint(mouseDownX, mouseDownY);
      mDragRegion.L = x < mouseDownX ? x : mouseDownX;
      mDragRegion.R = x < mouseDownX ? mouseDownX : x;
      mDragRegion.T = y < mouseDownY ? y : mouseDownY;
      mDragRegion.B = y < mouseDownY ? mouseDownY : y;
      
      GetUI()->ForStandardControlsFunc([&](IControl& c) {
                                         if(mDragRegion.Contains(c.GetRECT())) {
                                           if(mSelectedControls.FindR(&c) == -1)
                                             mSelectedControls.Add(&c);
                                         }
                                         else {
                                           int idx = mSelectedControls.FindR(&c);
                                           if(idx > -1)
                                             mSelectedControls.Delete(idx);
                                         }
        
        
                                       });
    }
  }
  
  bool OnKeyDown(float x, float y, const IKeyPress& key) override
  {
    GetUI()->ReleaseMouseCapture();
    
    if(key.VK == kVK_BACK || key.VK == kVK_DELETE)
    {
      if(mSelectedControls.GetSize())
      {
        for(int i = 0; i < mSelectedControls.GetSize(); i++)
        {
          IControl* pControl = mSelectedControls.Get(i);
          mSourceEditor.RemoveControlFromSource(GetUI()->GetControlIdx(pControl));
          GetUI()->RemoveControl(pControl);
        }
        
        mSelectedControls.Empty();
        GetUI()->SetAllControlsDirty();
        
        return true;
      }
    }
    
    return false;
  }
  
  void OnPopupMenuSelection(IPopupMenu* pSelectedMenu, int valIdx) override
  {
    if(pSelectedMenu && pSelectedMenu == &mRightClickOutsideControlMenu)
    {
      auto idx = pSelectedMenu->GetChosenItemIdx();
      float x, y;
      GetUI()->GetMouseDownPoint(x, y);
      IRECT b = IRECT(x, y, x + 100.f, y + 100.f);
      const char* controlConstructor = "";

      switch(idx)
      {
        case 0:
          GetUI()->AttachControl(new PlaceHolder(b));
          controlConstructor = "PlaceHolder(LIVE_EDIT_RECT())";
          mSourceEditor.AddControlToSource(controlConstructor, b);
          break;
        case 1:
          GetUI()->AttachControl(new IVKnobControl(b, nullptr));
          controlConstructor = "IVKnobControl(LIVE_EDIT_RECT(), nullptr)";
          mSourceEditor.AddControlToSource(controlConstructor, b);
          break;
        case 2:
          GetUI()->AttachControl(new IVSliderControl(b, nullptr));
          controlConstructor = "IVSliderControl(LIVE_EDIT_RECT(), nullptr)";
          mSourceEditor.AddControlToSource(controlConstructor, b);
          break;
        default:
          break;
      }
    }

    if (pSelectedMenu && pSelectedMenu == &mRightClickOnControlMenu)
    {
      auto idx = pSelectedMenu->GetChosenItemIdx();

      switch (idx)
      {
      case 0:
          mSelectedControls.Empty();
          GetUI()->RemoveControl(mClickedOnControl);
          mSourceEditor.RemoveControlFromSource(mClickedOnControl);
          mClickedOnControl = -1;
        break;
      default:
        break;
      }

    }
  }
  
  void Draw(IGraphics& g) override
  {
    IBlend b {EBlend::Default, 0.25f};
    g.DrawGrid(mGridColor, g.GetBounds(), mGridSize, mGridSize, &b);
    
    for(int i = 1; i < g.NControls(); i++)
    {
      IControl* pControl = g.GetControl(i);
      IRECT cr = pControl->GetRECT();

      if(pControl->IsHidden())
        g.DrawDottedRect(COLOR_RED, cr);
      else if(pControl->IsDisabled())
        g.DrawDottedRect(COLOR_GREEN, cr);
      else
        g.DrawDottedRect(COLOR_BLUE, cr);
      
      IRECT h = GetHandleRect(cr);
      g.FillTriangle(mRectColor, h.L, h.B, h.R, h.B, h.R, h.T);
      g.DrawTriangle(COLOR_BLACK, h.L, h.B, h.R, h.B, h.R, h.T);
    }
    
    for(int i = 0; i< mSelectedControls.GetSize(); i++)
    {
      g.DrawDottedRect(COLOR_WHITE, mSelectedControls.Get(i)->GetRECT());
    }
    
    if(!mDragRegion.Empty())
    {
      g.DrawDottedRect(COLOR_RED, mDragRegion);
    }
  }
  
  void OnResize() override
  {
    mSelectedControls.Empty();
    mRECT = GetUI()->GetBounds();
    SetTargetRECT(mRECT);
  }
  
  bool IsDirty() override { return true; }

  inline IRECT GetHandleRect(const IRECT& r)
  {
    return IRECT(r.R - RESIZE_HANDLE_SIZE, r.B - RESIZE_HANDLE_SIZE, r.R, r.B);
  }

  inline float SnapToGrid(float input)
  {
    if (mGridSize > 1)
      return (float) std::round(input / (float) mGridSize) * mGridSize;
    else
      return input;
  }
  
  bool GetActive() const
  {
    return mClickedOnControl > 0;
  }

private:
  IPopupMenu mRightClickOutsideControlMenu {"Outside Control", {"Add Place Holder", "Add IVKnobControl", "Add IVButtonControl"}};
  IPopupMenu mRightClickOnControlMenu{ "On Control", {"Delete Control"} };

  bool mMouseOversEnabled;
//  bool mEditModeActive = false;
//  bool mLiveEditingEnabled = false;
  bool mMouseClickedOnResizeHandle = false;
  bool mMouseIsDragging = false;
  WDL_String mErrorMessage;
  WDL_PtrList<IControl> mSelectedControls;

  IColor mGridColor = COLOR_WHITE;
  IColor mRectColor = COLOR_WHITE;
  static const int RESIZE_HANDLE_SIZE = 10;

  IRECT mMouseDownRECT;
  IRECT mMouseDownTargetRECT;
  IRECT mDragRegion;

  float mGridSize = 10;
  int mClickedOnControl = -1;

  IGraphicsLiveEditSourceEditor mSourceEditor;
};

END_IGRAPHICS_NAMESPACE
END_IPLUG_NAMESPACE

#endif // !NDEBUG
