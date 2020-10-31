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
 * LIVE_EDIT_RECT should be used for the IRECT, LIVE_EDIT_COLOR for an IColor
 *
 * All macros should be placed on the new line. */

#define LIVE_EDIT_INIT(p) pGraphics->SetLiveEditSourcePath(p);
#define LIVE_EDIT_END

#define LIVE_EDIT_CONTROL_START
#define LIVE_EDIT_CONTROL_END

#define LIVE_EDIT_RECT(L, T, R, B) IRECT(L, T, R, B)

#define LIVE_EDIT_COLOR(A, R, G, B) IColor(A, R, G, B)

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

  void UpdateControlRectSource(int controlIndex, const IRECT& r)
  {
    int sourceControlIndexStart = FindSourceIndex(controlIndex, "LIVE_EDIT_CONTROL_START");
    int sourceControlIndexEnd = FindSourceIndex(controlIndex, "LIVE_EDIT_CONTROL_END");

    if (controlIndex == -1 || sourceControlIndexStart == -1 || sourceControlIndexEnd == -1) return;

    for (int i = sourceControlIndexStart + 1; i < sourceControlIndexEnd; i++)
    {
      WDL_String rectSrc;
      rectSrc.SetFormatted(128, "LIVE_EDIT_RECT(%0.1f, %0.1f, %0.1f, %0.1f)", r.L, r.T, r.R, r.B);

      ReplaceSourceText(mSourceFile[i], "LIVE_EDIT_RECT", ")", rectSrc.Get());
    }

    WriteSourceFile();
  }
  
  void UpdateControlColorSource(int controlIndex, const IColor& c)
  {
    int sourceControlIndexStart = FindSourceIndex(controlIndex, "LIVE_EDIT_CONTROL_START");
    int sourceControlIndexEnd = FindSourceIndex(controlIndex, "LIVE_EDIT_CONTROL_END");

    if (controlIndex == -1 || sourceControlIndexStart == -1 || sourceControlIndexEnd == -1) return;

    for (int i = sourceControlIndexStart + 1; i < sourceControlIndexEnd; i++)
    {
      WDL_String rectSrc;
      rectSrc.SetFormatted(128, "LIVE_EDIT_COLOR(%i, %i, %i, %i)", c.A, c.R, c.G, c.B);

      ReplaceSourceText(mSourceFile[i], "LIVE_EDIT_COLOR", ")", rectSrc.Get());
    }

    WriteSourceFile();
  }

  void AddControlToSource(const char* ctorText, const IRECT& r)
  {
    int numberOfEnds = FindNumberOf("LIVE_EDIT_CONTROL_END");
    int appendToSourceIndex = 0;

    if (numberOfEnds == 0)
      appendToSourceIndex = FindSourceIndex(0, "LIVE_EDIT_INIT");
    else
      appendToSourceIndex = FindSourceIndex(numberOfEnds - 1, "LIVE_EDIT_CONTROL_END");
    
    std::vector<std::string> ctrlSrc;

    ctrlSrc.push_back("");
    ctrlSrc.push_back("    LIVE_EDIT_CONTROL_START");
    ctrlSrc.push_back("    pGraphics->AttachControl(new ");
    ctrlSrc.back().append(ctorText);
    ctrlSrc.back().append(");");
    
    // Add live edit rect values
    WDL_String rectSrc;
    rectSrc.SetFormatted(128, "LIVE_EDIT_RECT(%0.1f, %0.1f, %0.1f, %0.1f)", r.L, r.T, r.R, r.B);
    ReplaceSourceText(ctrlSrc.back(), "LIVE_EDIT_RECT", ")", rectSrc.Get());

    ctrlSrc.push_back("    LIVE_EDIT_CONTROL_END");

    // Add to source
    mSourceFile.insert(mSourceFile.begin() + appendToSourceIndex + 1, ctrlSrc.begin(), ctrlSrc.end());

    WriteSourceFile();
  }

  void RemoveControlFromSource(int controlIndexToRemove)
  {
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
          
          pNewControl = CreateNewControlBasedOnClassName(*className, mMouseDownRECT, label);
          
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
        WDL_String src;
        CreateSrcBasedOnClassName(*className, src);
        
        IPropMap map = pControl->GetProperties();
        
        if(!map.empty()) {
          src.AppendFormatted(1024, ")\n    ->SetProperties({\n    ");
        }
        
        for (auto&& prop : map) {
          src.AppendFormatted(1024, "{\"%s\", ", prop.first.c_str());
          switch (prop.second.index())
          {
            case kColor:
            {
              IColor val = *pControl->GetProp<IColor>(prop.first);
              src.AppendFormatted(1024, "IColor(%i, %i, %i, %i)", val.A, val.R, val.G, val.B);
              break;
            }
            case kBool:
            {
              bool val = *pControl->GetProp<bool>(prop.first);
              src.AppendFormatted(1024, val ? "true" : "false");
              break;
            }
            case kFloat:
            {
              float val = *pControl->GetProp<float>(prop.first);
              src.AppendFormatted(1024, "%0.6ff", val);
              break;
            }
            case kInt:
            {
              int val = *pControl->GetProp<int>(prop.first);
              src.AppendFormatted(1024, "%i", val);
              break;
            }
            case kRect:
            {
              IRECT val = *pControl->GetProp<IRECT>(prop.first);
              src.AppendFormatted(1024, "IRECT(%f,%f,%f,%f)", val.L, val.T, val.R, val.B);
              break;
            }
            case kText:
            {
              IText val = *pControl->GetProp<IText>(prop.first);
              src.AppendFormatted(1024, "DEFAULT_TEXT");
              break;
            }
            case kStr:
            {
              const char* val = *pControl->GetProp<const char*>(prop.first);
              src.AppendFormatted(1024, "\"%s\"", val);
              break;
            }
            default:
            {
//              bool val = *pControl->GetProp<bool>(prop.first);
              src.AppendFormatted(1024, "TODO");
              break;
            }
          }
          src.AppendFormatted(1024, "},\n    ");
        }
        src.AppendFormatted(1024, "}");
        
        mSourceEditor.AddControlToSource(src.Get(), mMouseDownRECT);
      }
//      else if (mod.R)
//      {
//        mClickedOnControl = c;
//        GetUI()->CreatePopupMenu(*this, mRightClickOnControlMenu, x, y);
//      }
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
    
    mMouseX = x;
    mMouseY = y;
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
    IGraphics* pGraphics = GetUI();
    
    if(pSelectedMenu)
    {
      IControl* pControl = pGraphics->GetControl(mClickedOnControl);

      if(pControl)
      {
        auto& props = pControl->GetProperties(); // TODO: does this copy?
        
        if(strcmp(pSelectedMenu->GetRootTitle(), "Add control") == 0)
        {
          auto idx = pSelectedMenu->GetChosenItemIdx();
          float x, y;
          pGraphics->GetMouseDownPoint(x, y);
          IRECT b = IRECT(x, y, x + 100.f, y + 100.f);
          WDL_String label;
          const char* className = pSelectedMenu->GetChosenItem()->GetText();
          label.SetFormatted(128, "%s", className);
          IControl* pNewControl = CreateNewControlBasedOnClassName(className, b, label.Get());

          if(pNewControl)
          {
            pGraphics->AttachControl(pNewControl);
            WDL_String str;
            CreateSrcBasedOnClassName(className, str);
            mSourceEditor.AddControlToSource(str.Get(), b);
          }
          else
            pGraphics->ShowMessageBox("Not implemented yet!", "", EMsgBoxType::kMB_OK, nullptr);
        }
        else
        {
          if(strcmp(pSelectedMenu->GetChosenItem()->GetText(), "Delete Control") == 0)
          {
            mSelectedControls.Empty();
            mSourceEditor.RemoveControlFromSource(pGraphics->GetControlIdx(pControl));
            pGraphics->RemoveControl(mClickedOnControl);
            mClickedOnControl = -1;
          }
          else if(pSelectedMenu->GetChosenItemIdx() < props.size())
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
        }
      }
    }
    
    mClickedOnControl = -1;
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
  
  void OnDrop(const char* str) override
  {
    IGraphics* pGraphics = GetUI();

    IBitmap bmp = pGraphics->LoadBitmap(str);
  
    pGraphics->AttachControl(new IBitmapControl(IRECT(mMouseX, mMouseY, bmp), bmp));

    WDL_String ctrlStr;
    ctrlStr.SetFormatted(128, "IBitmapControl(LIVE_EDIT_RECT(), pGraphics->LoadBitmap(\"%s\"))", str);
    mSourceEditor.AddControlToSource(ctrlStr.Get(), GetUI()->GetBounds());
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
  IControl* CreateNewControlBasedOnClassName(const char* cname, const IRECT& r, const char* label)
  {
    IControl* pNewControl = nullptr;
    if     (strcmp(cname, "IVLabelControl")       == 0) pNewControl = new IVLabelControl(r, label);
    else if(strcmp(cname, "IVButtonControl")      == 0) pNewControl = new IVButtonControl(r, SplashClickActionFunc, label);
    else if(strcmp(cname, "IVSwitchControl")      == 0) pNewControl = new IVSwitchControl(r, kNoParameter, label);
    else if(strcmp(cname, "IVToggleControl")      == 0) pNewControl = new IVToggleControl(r, kNoParameter, label);
    else if(strcmp(cname, "IVSlideSwitchControl") == 0) pNewControl = new IVSlideSwitchControl(r, kNoParameter, label);
    else if(strcmp(cname, "IVTabSwitchControl")   == 0) pNewControl = new IVTabSwitchControl(r, kNoParameter, {"One", "Two", "Three"}, label);
    else if(strcmp(cname, "IVRadioButtonControl") == 0) pNewControl = new IVRadioButtonControl(r, kNoParameter, {"One", "Two", "Three"}, label);
    else if(strcmp(cname, "IVKnobControl")        == 0) pNewControl = new IVKnobControl(r, kNoParameter, label);
    else if(strcmp(cname, "IVSliderControl")      == 0) pNewControl = new IVSliderControl(r, kNoParameter, label);
    else if(strcmp(cname, "IVRangeSliderControl") == 0) pNewControl = new IVRangeSliderControl(r, {kNoParameter, kNoParameter}, label);
    else if(strcmp(cname, "IVXYPadControl")       == 0) pNewControl = new IVXYPadControl(r, {kNoParameter, kNoParameter}, label);
//    else if(strcmp(cname, "IVPlotControl")        == 0) pNewControl = new IVPlotControl(r);
    else if(strcmp(cname, "IVGroupControl")       == 0) pNewControl = new IVGroupControl(r);
    else if(strcmp(cname, "IVPanelControl")       == 0) pNewControl = new IVPanelControl(r);
    else if(strcmp(cname, "IVColorSwatchControl") == 0) pNewControl = new IVColorSwatchControl(r);
//    else if(strcmp(cname, "ISVGKnobControl")      == 0) pNewControl = new ISVGKnobControl(r);
//    else if(strcmp(cname, "ISVGButtonControl")    == 0) pNewControl = new ISVGButtonControl(r);
//    else if(strcmp(cname, "ISVGSwitchControl")    == 0) pNewControl = new ISVGSwitchControl(r);
//    else if(strcmp(cname, "ISVGSliderControl")    == 0) pNewControl = new ISVGSliderControl(r);
//    else if(strcmp(cname, "ISVGControl")          == 0) pNewControl = new ISVGControl(r);
//    else if(strcmp(cname, "IBButtonControl")      == 0) pNewControl = new IBButtonControl(r);
//    else if(strcmp(cname, "IBSwitchControl")      == 0) pNewControl = new IBSwitchControl(r);
//    else if(strcmp(cname, "IBKnobControl")        == 0) pNewControl = new IBKnobControl(r);
//    else if(strcmp(cname, "IBKnobRotaterControl") == 0) pNewControl = new IBKnobRotaterControl(r);
//    else if(strcmp(cname, "IBSliderControl")      == 0) pNewControl = new IBSliderControl(r);
//    else if(strcmp(cname, "IBTextControl")        == 0) pNewControl = new IBTextControl(r);
//    else if(strcmp(cname, "IBitmapControl")       == 0) pNewControl = new IBitmapControl(r);
    else if(strcmp(cname, "IPanelControl")        == 0) pNewControl = new IPanelControl(r);
    else if(strcmp(cname, "ILambdaControl")       == 0) pNewControl = new ILambdaControl(r, nullptr);
    else if(strcmp(cname, "ITextControl")         == 0) pNewControl = new ITextControl(r);
    else if(strcmp(cname, "IURLControl")          == 0) pNewControl = new IURLControl(r, label, "https://iplug2.github.io");
    else if(strcmp(cname, "ITextToggleControl")   == 0) pNewControl = new ITextToggleControl(r, kNoParameter, "OFF", "ON");
    else if(strcmp(cname, "ICaptionControl")      == 0) pNewControl = new ICaptionControl(r, kNoParameter);
    else if(strcmp(cname, "IPlaceHolderControl")  == 0) pNewControl = new IPlaceHolderControl(r);
    
    return pNewControl;
  }
  
  void CreateSrcBasedOnClassName(const char* cname, WDL_String& str)
  {
    const char* LER = "LIVE_EDIT_RECT()";
    if     (strcmp(cname, "IVLabelControl")       == 0) str.SetFormatted(1024, "IVLabelControl(%s, \"%s\")", LER, cname);
    else if(strcmp(cname, "IVButtonControl")      == 0) str.SetFormatted(1024, "IVButtonControl(%s, SplashClickActionFunc, \"%s\")", LER, cname);
    else if(strcmp(cname, "IVSwitchControl")      == 0) str.SetFormatted(1024, "IVSwitchControl(%s, SplashClickActionFunc, \"%s\")", LER, cname);
    else if(strcmp(cname, "IVToggleControl")      == 0) str.SetFormatted(1024, "IVToggleControl(%s, kNoParameter, \"%s\")", LER, cname);
    else if(strcmp(cname, "IVSlideSwitchControl") == 0) str.SetFormatted(1024, "IVSlideSwitchControl(%s, kNoParameter, \"%s\")", LER, cname);
    else if(strcmp(cname, "IVTabSwitchControl")   == 0) str.SetFormatted(1024, "IVTabSwitchControl(%s, kNoParameter, {\"One\", \"Two\", \"Three\"}, \"%s\")", LER, cname);
    else if(strcmp(cname, "IVRadioButtonControl") == 0) str.SetFormatted(1024, "IVRadioButtonControl(%s, kNoParameter, {\"One\", \"Two\", \"Three\"}", LER);
    else if(strcmp(cname, "IVKnobControl")        == 0) str.SetFormatted(1024, "IVKnobControl(%s, kNoParameter, \"%s\")", LER, cname);
    else if(strcmp(cname, "IVSliderControl")      == 0) str.SetFormatted(1024, "IVSliderControl(%s, kNoParameter, \"%s\")", LER, cname);
    else if(strcmp(cname, "IVRangeSliderControl") == 0) str.SetFormatted(1024, "IVRangeSliderControl(%s, {kNoParameter, kNoParameter}, \"%s\")", LER, cname);
    else if(strcmp(cname, "IVXYPadControl")       == 0) str.SetFormatted(1024, "IVXYPadControl(%s, {kNoParameter, kNoParameter}, \"%s\")", LER, cname);
    else if(strcmp(cname, "IVPlotControl")        == 0) str.SetFormatted(1024, "IVPlotControl(%s, \"%s\")", LER, cname);
    else if(strcmp(cname, "IVGroupControl")       == 0) str.SetFormatted(1024, "IVGroupControl(%s, \"%s\")", LER, cname);
    else if(strcmp(cname, "IVPanelControl")       == 0) str.SetFormatted(1024, "IVPanelControl(%s, \"%s\")", LER, cname);
    else if(strcmp(cname, "IVColorSwatchControl") == 0) str.SetFormatted(1024, "IVColorSwatchControl(%s, \"%s\")", LER, cname);
    else if(strcmp(cname, "ISVGKnobControl")      == 0) str.SetFormatted(1024, "ISVGKnobControl(%s, kNoParameter, \"%s\")", LER, cname);
    else if(strcmp(cname, "ISVGButtonControl")    == 0) str.SetFormatted(1024, "ISVGButtonControl(%s, kNoParameter, \"%s\")", LER, cname);
    else if(strcmp(cname, "ISVGSwitchControl")    == 0) str.SetFormatted(1024, "ISVGSwitchControl(%s, kNoParameter, \"%s\")", LER, cname);
    else if(strcmp(cname, "ISVGSliderControl")    == 0) str.SetFormatted(1024, "ISVGSliderControl(%s, kNoParameter, \"%s\")", LER, cname);
    else if(strcmp(cname, "IBButtonControl")      == 0) str.SetFormatted(1024, "IBButtonControl(%s, kNoParameter, \"%s\")", LER, cname);
    else if(strcmp(cname, "IBSwitchControl")      == 0) str.SetFormatted(1024, "IBSwitchControl(%s, kNoParameter, \"%s\")", LER, cname);
    else if(strcmp(cname, "IBKnobControl")        == 0) str.SetFormatted(1024, "IBKnobControl(%s, kNoParameter, \"%s\")", LER, cname);
    else if(strcmp(cname, "IBKnobRotaterControl") == 0) str.SetFormatted(1024, "IBKnobRotaterControl(%s, kNoParameter, \"%s\")", LER, cname);
    else if(strcmp(cname, "IBSliderControl")      == 0) str.SetFormatted(1024, "IBSliderControl(%s, kNoParameter, \"%s\")", LER, cname);
    else if(strcmp(cname, "IBTextControl")        == 0) str.SetFormatted(1024, "IBTextControl(%s, kNoParameter, \"%s\")", LER, cname);
    else if(strcmp(cname, "IPanelControl")        == 0) str.SetFormatted(1024, "IPanelControl(%s, \"%s\")", LER, cname);
    else if(strcmp(cname, "ILambdaControl")       == 0) str.SetFormatted(1024, "ILambdaControl(%s, nullptr, \"%s\")", LER, cname);
    else if(strcmp(cname, "IBitmapControl")       == 0) str.SetFormatted(1024, "IBitmapControl(%s, kNoParameter, \"%s\")", LER, cname);
    else if(strcmp(cname, "ISVGControl")          == 0) str.SetFormatted(1024, "ISVGControl(%s, kNoParameter, \"%s\")", LER, cname);
    else if(strcmp(cname, "ITextControl")         == 0) str.SetFormatted(1024, "ITextControl(%s, \"%s\")", LER, cname);
    else if(strcmp(cname, "IURLControl")          == 0) str.SetFormatted(1024, "IURLControl(%s, \"URL\", \"https://iPlug2.github.io\", \"%s\")", LER, cname);
    else if(strcmp(cname, "ITextToggleControl")   == 0) str.SetFormatted(1024, "ITextToggleControl(%s, kNoParameter, \"OFF\", \"ON\", \"%s\")", LER, cname);
    else if(strcmp(cname, "ICaptionControl")      == 0) str.SetFormatted(1024, "ICaptionControl(%s, kNoParameter)", LER, cname);
    else                                                str.SetFormatted(1024, "IPlaceHolderControl(LER)",               LER);
  }
  
  IPopupMenu mRightClickOutsideControlMenu {"Outside Control", {""}};
  IPopupMenu mRightClickOnControlMenu {"On Control", {"Delete Control"}};

  bool mMouseOversEnabled;
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
  float mMouseX = 0.f;
  float mMouseY = 0.f;

  float mGridSize = 10;
  int mClickedOnControl = -1;

  IGraphicsLiveEditSourceEditor mSourceEditor;
};

END_IGRAPHICS_NAMESPACE
END_IPLUG_NAMESPACE

#endif // !NDEBUG
