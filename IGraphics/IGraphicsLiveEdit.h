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
class IGraphicsLiveEdit : public IControl
{
public:
  IGraphicsLiveEdit(bool mouseOversEnabled)
  : IControl(IRECT())
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
    IGraphics* pGraphics = GetUI();
    int c = pGraphics->GetMouseControlIdx(x, y, true);
    
    if (c > 0)
    {
      IControl* pControl = pGraphics->GetControl(c);
      mMouseDownRECT = pControl->GetRECT();
      mMouseDownTargetRECT = pControl->GetTargetRECT();

      if(!mod.S)
        mSelectedControls.Empty();
      
      mSelectedControls.Add(pControl);

      if(mod.A)
      {
        auto className = pControl->GetProp<const char*>("class_name");
        
        IControl* pNewControl = nullptr;
        
        if(className.has_value())
        {
          IVectorBase* pVectorBase = dynamic_cast<IVectorBase*>(pControl);
          
          const char* label = pVectorBase ? pVectorBase->GetLabelStr() : "";
          
          if(strcmp(*className, "IVLabelControl")            == 0) pNewControl = new IVLabelControl(mMouseDownRECT, label);
          else if(strcmp(*className, "IVButtonControl")      == 0) pNewControl = new IVButtonControl(mMouseDownRECT);
          else if(strcmp(*className, "IVSwitchControl")      == 0) pNewControl = new IVSwitchControl(mMouseDownRECT, kNoParameter, label);
          else if(strcmp(*className, "IVToggleControl")      == 0) pNewControl = new IVToggleControl(mMouseDownRECT, kNoParameter, label);
          else if(strcmp(*className, "IVSlideSwitchControl") == 0) pNewControl = new IVSlideSwitchControl(mMouseDownRECT, kNoParameter, label);
          else if(strcmp(*className, "IVTabSwitchControl")   == 0) pNewControl = new IVTabSwitchControl(mMouseDownRECT, kNoParameter, {"One", "Two", "Three"}, label);
          else if(strcmp(*className, "IVRadioButtonControl") == 0) pNewControl = new IVRadioButtonControl(mMouseDownRECT, kNoParameter, {"One", "Two", "Three"}, label);
          else if(strcmp(*className, "IVKnobControl")        == 0) pNewControl = new IVKnobControl(mMouseDownRECT, kNoParameter, label);
          else if(strcmp(*className, "IVSliderControl")      == 0) pNewControl = new IVSliderControl(mMouseDownRECT, kNoParameter, label);
//          else if(strcmp(*className, "IVRangeSliderControl") == 0) pNewControl = new IVRangeSliderControl(mMouseDownRECT);
//          else if(strcmp(*className, "IVXYPadControl")       == 0) pNewControl = new IVXYPadControl(mMouseDownRECT);
//          else if(strcmp(*className, "IVPlotControl")        == 0) pNewControl = new IVPlotControl(mMouseDownRECT);
//          else if(strcmp(*className, "IVGroupControl")       == 0) pNewControl = new IVGroupControl(mMouseDownRECT);
//          else if(strcmp(*className, "IVPanelControl")       == 0) pNewControl = new IVPanelControl(mMouseDownRECT);
//          else if(strcmp(*className, "IVColorSwatchControl") == 0) pNewControl = new IVColorSwatchControl(mMouseDownRECT);
//          else if(strcmp(*className, "ISVGKnobControl")      == 0) pNewControl = new ISVGKnobControl(mMouseDownRECT);
//          else if(strcmp(*className, "ISVGButtonControl")    == 0) pNewControl = new ISVGButtonControl(mMouseDownRECT);
//          else if(strcmp(*className, "ISVGSwitchControl")    == 0) pNewControl = new ISVGSwitchControl(mMouseDownRECT);
//          else if(strcmp(*className, "ISVGSliderControl")    == 0) pNewControl = new ISVGSliderControl(mMouseDownRECT);
//          else if(strcmp(*className, "IBButtonControl")      == 0) pNewControl = new IBButtonControl(mMouseDownRECT);
//          else if(strcmp(*className, "IBSwitchControl")      == 0) pNewControl = new IBSwitchControl(mMouseDownRECT);
//          else if(strcmp(*className, "IBKnobControl")        == 0) pNewControl = new IBKnobControl(mMouseDownRECT);
//          else if(strcmp(*className, "IBKnobRotaterControl") == 0) pNewControl = new IBKnobRotaterControl(mMouseDownRECT);
//          else if(strcmp(*className, "IBSliderControl")      == 0) pNewControl = new IBSliderControl(mMouseDownRECT);
//          else if(strcmp(*className, "IBTextControl")        == 0) pNewControl = new IBTextControl(mMouseDownRECT);
//          else if(strcmp(*className, "IPanelControl")        == 0) pNewControl = new IPanelControl(mMouseDownRECT);
//          else if(strcmp(*className, "ILambdaControl")       == 0) pNewControl = new ILambdaControl(mMouseDownRECT);
//          else if(strcmp(*className, "IBitmapControl")       == 0) pNewControl = new IBitmapControl(mMouseDownRECT);
//          else if(strcmp(*className, "ISVGControl")          == 0) pNewControl = new ISVGControl(mMouseDownRECT);
//          else if(strcmp(*className, "ITextControl")         == 0) pNewControl = new ITextControl(mMouseDownRECT);
//          else if(strcmp(*className, "IURLControl")          == 0) pNewControl = new IURLControl(mMouseDownRECT);
//          else if(strcmp(*className, "ITextToggleControl")   == 0) pNewControl = new ITextToggleControl(mMouseDownRECT);
//          else if(strcmp(*className, "ICaptionControl")      == 0) pNewControl = new ICaptionControl(mMouseDownRECT);
//          else if(strcmp(*className, "IPlaceHolderControl")  == 0) pNewControl = new IPlaceHolderControl(mMouseDownRECT);
          
          if(pNewControl)
            pNewControl->SetProperties(pControl->GetProperties());
        }
        else
        {
          pNewControl = new IPlaceHolderControl(mMouseDownRECT);
        }
        
        pGraphics->AttachControl(pNewControl);
        
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
        else if(mod.R)
        {
          mRightClickOnControlMenu.Clear();
          
//          mRightClickOnControlMenu.AddItem("Replace with control...");
//          mRightClickOnControlMenu.AddSeparator();
          
          for(auto& prop : pControl->GetProperties())
          {
            if(prop.first != "class_name")
            {
              int flags = 0;
              
              if(prop.second.index() == kBool)
              {
                if(*std::get_if<bool>(&prop.second))
                  flags |= IPopupMenu::Item::Flags::kChecked;
              }
                
              auto* pItem = new IPopupMenu::Item(prop.first.c_str(), flags);
              
              mRightClickOnControlMenu.AddItem(pItem);
            }
          }
          
          GetUI()->CreatePopupMenu(*this, mRightClickOnControlMenu, x, y);
        }
      }
    }
    else if(mod.R) // right click on background
    {
      mClickedOnControl = 0;
      
      mRightClickOutsideControlMenu.Clear();
            
      for(auto& prop : GetUI()->GetBackgroundControl()->GetProperties())
      {
        if(prop.first != "class_name")
          mRightClickOutsideControlMenu.AddItem(prop.first.c_str());
      }
      
      mRightClickOutsideControlMenu.AddItem("Add control ...", new IPopupMenu("Add control",
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
      
      
      mRightClickOutsideControlMenu.AddSeparator();
      
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
            case 1 : pNewControl = new IVButtonControl(b, SplashClickActionFunc, "New IVButtonControl"); break;
            case 2 : pNewControl = new IVSwitchControl(b, SplashClickActionFunc, "New IVSwitchControl"); break;
            case 3 : pNewControl = new IVToggleControl(b, kNoParameter, "New IVToggleControl"); break;
            case 4 : pNewControl = new IVSlideSwitchControl(b, kNoParameter, "New IVSlideSwitchControl"); break;
            case 5 : pNewControl = new IVTabSwitchControl(b, kNoParameter, {"One", "Two", "Three"}, "New IVTabSwitchControl"); break;
            case 6 : pNewControl = new IVRadioButtonControl(b, kNoParameter, {"One", "Two", "Three"}, "New IVRadioButtonControl"); break;
            case 7 : pNewControl = new IVKnobControl(b, kNoParameter, "New IVKnobControl"); break;
            case 8 : pNewControl = new IVSliderControl(b, kNoParameter, "New IVSliderControl"); break;
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
            case 25 : pNewControl = new IPanelControl(b); break;
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
          
          if(pNewControl)
            pGraphics->AttachControl(pNewControl);
          else
            pGraphics->ShowMessageBox("Not implemented yet!", "", EMsgBoxType::kMB_OK, nullptr);
        }
        else
        {
          if(pSelectedMenu->GetChosenItemIdx() < props.size())
          {
            auto prop = *(props.find(pSelectedMenu->GetChosenItem()->GetText()));
            auto& propName = prop.first;
            auto& propVal = prop.second;
            
            switch (prop.second.index())
            {
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
              case kBool:
              {
                bool currentState = *std::get_if<bool>(&propVal);
                pControl->SetProp(propName, !currentState, true);
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
      
//      if (pSelectedMenu == &mRightClickOnControlMenu)
//      {
//        auto idx = pSelectedMenu->GetChosenItemIdx();
//
//        switch (idx)
//        {
//          case 0:
//            mSelectedControls.Empty();
//            GetUI()->RemoveControl(mClickedOnControl);
//            mClickedOnControl = -1;
//            break;
//          default:
//            break;
//        }
//
//      }
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

private:
  IPopupMenu mRightClickOutsideControlMenu {"Outside Control", {"Add Place Holder"}};
  IPopupMenu mRightClickOnControlMenu{ "On Control", {"Delete Control"} };

  bool mMouseOversEnabled = false;
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
};

END_IGRAPHICS_NAMESPACE
END_IPLUG_NAMESPACE

#endif // !NDEBUG
