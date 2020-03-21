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

BEGIN_IPLUG_NAMESPACE
BEGIN_IGRAPHICS_NAMESPACE

/** A control to enable live modification of control layout in an IGraphics context in debug builds
 * This is based on the work of Youlean, who first included it in iPlug-Youlean
 * The lives outside the main IGraphics control stack and it can be added with IGraphics::EnableLiveEdit().
 * It should not be used in the main control stack.
 * @ingroup SpecialControls */
class IGraphicsLiveEdit : public IControl
{
public:
  IGraphicsLiveEdit(bool mouseOversEnabled, const char* pathToSourceFile = 0, float gridSize = 10)
  : IControl(IRECT())
  , mPathToSourceFile(pathToSourceFile)
  , mGridSize(gridSize)
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
      
      if(mod.A)
      {
        GetUI()->AttachControl(new IPlaceHolderControl(mMouseDownRECT));
        mClickedOnControl = GetUI()->NControls() - 1;
        mMouseClickedOnResizeHandle = false;
      }
      else
      {
        mClickedOnControl = c;
        
        if(GetHandleRect(mMouseDownRECT).Contains(x, y))
        {
          mMouseClickedOnResizeHandle = true;
        }
//        else
//        {
//          mRightClickMenu.Clear();
//          
//          mRightClickMenu.AddItem("Replace with control...");
//          mRightClickMenu.AddSeparator();
//          
//          for(auto& prop : pControl->GetProperties())
//          {
//            mRightClickMenu.AddItem(prop.first.c_str());
//          }
//          
//          GetUI()->CreatePopupMenu(*this, mRightClickMenu, x, y);
//        }
      }
    }
    else if(mod.R)
    {
      mClickedOnControl = 0;
      
      mRightClickMenu.Clear();
            
      for(auto& prop : GetUI()->GetBackgroundControl()->GetProperties())
      {
        mRightClickMenu.AddItem(prop.first.c_str());
      }
      
      mRightClickMenu.AddItem("Add control ...", new IPopupMenu("Add control",
      {
        "IVLabelControl",
        "IVButtonControl",
        "IVSwitchControl",
        "IVToggleControl",
        "IVSlideSwitchControl",
        "IVTabSwitchControl",
        "IVRadioButtonControl",
        "IVKnobControl",
        "IVSliderControl",
        "IVRangeSliderControl",
        "IVXYPadControl",
        "IVPlotControl",
        "IVGroupControl",
        "IVPanelControl",
        "IVColorSwatchControl",
        "ISVGKnobControl",
        "ISVGButtonControl",
        "ISVGSwitchControl",
        "ISVGSliderControl",
        "IBButtonControl",
        "IBSwitchControl",
        "IBKnobControl",
        "IBKnobRotaterControl",
        "IBSliderControl",
        "IBTextControl",
        "IPanelControl",
        "ILambdaControl",
        "IBitmapControl",
        "ISVGControl",
        "ITextControl",
        "IURLControl",
        "ITextToggleControl",
        "ICaptionControl",
        "IPlaceHolderControl"
      } ));
      
      
      mRightClickMenu.AddSeparator();
      
      GetUI()->CreatePopupMenu(*this, mRightClickMenu, x, y);
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
          
        pControl->SetSize(r.W(), r.H());
      }
      else
      {
        const float x1 = SnapToGrid(mMouseDownRECT.L + (x - mouseDownX));
        const float y1 = SnapToGrid(mMouseDownRECT.T + (y - mouseDownY));
          
        pControl->SetPosition(x1, y1);
      }
        
      GetUI()->SetAllControlsDirty();
    }
  }
  
  void OnPopupMenuSelection(IPopupMenu* pSelectedMenu, int valIdx) override
  {
    IGraphics* pGraphics = GetUI();
    
    if(pSelectedMenu)
    {
      IControl* pControl = pGraphics->GetControl(mClickedOnControl);

      if(pControl)
      {
        auto& props = pControl->GetProperties();
        
        if(strcmp(pSelectedMenu->GetRootTitle(), "Add control") == 0)
        {
          auto idx = pSelectedMenu->GetChosenItemIdx();
          float x, y;
          pGraphics->GetMouseDownPoint(x, y);
          IRECT b = IRECT(x, y, x + 100.f, y + 100.f);
    
          IControl* pNewControl = nullptr;
          
          switch(idx)
          {
            case 0 : pNewControl = new IVLabelControl(b, "New IVLabelControl"); break;
//            case 1 : pNewControl = new IVButtonControl(b); break;
//            case 2 : pNewControl = new IVSwitchControl(b); break;
//            case 3 : pNewControl = new IVToggleControl(b); break;
//            case 4 : pNewControl = new IVSlideSwitchControl(b); break;
//            case 5 : pNewControl = new IVTabSwitchControl(b); break;
//            case 6 : pNewControl = new IVRadioButtonControl(b); break;
            case 7 : pNewControl = new IVKnobControl(b, kNoParameter); break;
            case 8 : pNewControl = new IVSliderControl(b); break;
//            case 9 : pNewControl = new IVRangeSliderControl(b); break;
//            case 10 : pNewControl = new IVXYPadControl(b); break;
//            case 11 : pNewControl = new IVPlotControl(b); break;
//            case 12 : pNewControl = new IVGroupControl(b); break;
//            case 13 : pNewControl = new IVPanelControl(b); break;
//            case 14 : pNewControl = new IVColorSwatchControl(b); break;
//            case 15 : pNewControl = new ISVGKnobControl(b); break;
//            case 16 : pNewControl = new ISVGButtonControl(b); break;
//            case 17 : pNewControl = new ISVGSwitchControl(b); break;
//            case 18 : pNewControl = new ISVGSliderControl(b); break;
//            case 19 : pNewControl = new IBButtonControl(b); break;
//            case 20 : pNewControl = new IBSwitchControl(b); break;
//            case 21 : pNewControl = new IBKnobControl(b); break;
//            case 22 : pNewControl = new IBKnobRotaterControl(b); break;
//            case 23 : pNewControl = new IBSliderControl(b); break;
//            case 24 : pNewControl = new IBTextControl(b); break;
//            case 25 : pNewControl = new IPanelControl(b); break;
//            case 26 : pNewControl = new ILambdaControl(b); break;
//            case 27 : pNewControl = new IBitmapControl(b); break;
//            case 28 : pNewControl = new ISVGControl(b); break;
//            case 29 : pNewControl = new ITextControl(b); break;
//            case 30 : pNewControl = new IURLControl(b); break;
//            case 31 : pNewControl = new ITextToggleControl(b); break;
//            case 32 : pNewControl = new ICaptionControl(b); break;
            case 33 : pNewControl = new IPlaceHolderControl(b); break;
            default: break;
          }
          
          pGraphics->AttachControl(pNewControl);
        }
        else
        {
          if(pSelectedMenu->GetChosenItemIdx() < props.size())
          {
            auto prop = *(props.find(pSelectedMenu->GetChosenItem()->GetText()));
            auto& propName = prop.first;
            auto& propVal = prop.second;
            
            switch (prop.second.index()) {
              case kColor:
              {
                IColor startColor = *(pControl->GetProp<IColor>(propName));
                GetUI()->PromptForColor(startColor,
                                        propName.c_str(),
                                        [pControl, propName](const IColor& color) {
                                          pControl->SetProp(propName, color, true);
                                        });
                break;
              }
              default:
                break;
            }
          }
//          else
//          {
//          }
        }
      }
    }
    
    mClickedOnControl = -1;
  }
  
  void Draw(IGraphics& g) override
  {
    IBlend b {EBlend::Add, 0.25f};
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
  }
  
  void OnResize() override
  {
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

private:
  IPopupMenu mRightClickMenu {"Add an item", {}};
  bool mMouseOversEnabled;
//  bool mEditModeActive = false;
//  bool mLiveEditingEnabled = false;
  bool mMouseClickedOnResizeHandle = false;
  bool mMouseIsDragging = false;
  WDL_String mPathToSourceFile;
  WDL_String mErrorMessage;

  IColor mGridColor = COLOR_WHITE;
  IColor mRectColor = COLOR_WHITE;
  static const int RESIZE_HANDLE_SIZE = 10;

  IRECT mMouseDownRECT;
  IRECT mMouseDownTargetRECT;

  float mGridSize = 10;
  int mClickedOnControl = -1;
};

END_IGRAPHICS_NAMESPACE
END_IPLUG_NAMESPACE

#endif // !NDEBUG
