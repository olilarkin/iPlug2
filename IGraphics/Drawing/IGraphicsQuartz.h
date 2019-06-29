/*
 ==============================================================================

 This file is part of the iPlug 2 library. Copyright (C) the iPlug 2 developers.

 See LICENSE.txt for  more info.

 ==============================================================================
*/

#pragma once

#include "IPlugPlatform.h"

#include "IGraphicsPathBase.h"

#include <CoreGraphics/CoreGraphics.h>

/** A Quartz API bitmap
 * @ingroup APIBitmaps */
class QuartzBitmap : public APIBitmap
{
public:
  QuartzBitmap(int scale, float drawScale);
  QuartzBitmap(int width, int height, int scale, float drawScale);
  virtual ~QuartzBitmap();
};

/** IGraphics draw class using Quartz
*   @ingroup DrawClasses */
class IGraphicsQuartz : public IGraphicsPathBase
{
public:
  const char* GetDrawingAPIStr() override { return "Quartz"; }

  IGraphicsQuartz(IGEditorDelegate& dlg, int w, int h, int fps, float scale);
  ~IGraphicsQuartz();

  void DrawBitmap(const IBitmap& bitmap, const IRECT& dest, int srcX, int srcY, const IBlend* pBlend) override;
      
  void PathClear() override;
  void PathClose() override;
  void PathArc(float cx, float cy, float r, float aMin, float aMax, EWinding winding) override;
  void PathMoveTo(float x, float y) override;
  void PathLineTo(float x, float y) override;
  void PathCubicBezierTo(float x1, float y1, float x2, float y2, float x3, float y3) override;
  void PathQuadraticBezierTo(float cx, float cy, float x2, float y2) override;
  void PathStroke(const IPattern& pattern, float thickness, const IStrokeOptions& options, const IBlend* pBlend) override;
  void PathFill(const IPattern& pattern, const IFillOptions& options, const IBlend* pBlend) override;
  
  IColor GetPoint(int x, int y) override;
  void* GetDrawContext() override { return (void*) GetPlatformContext(); } // same on quartz?
  void SetPlatformContext(void* pContext) override { mContext = (CGContextRef) pContext; IGraphics::SetPlatformContext(pContext); }

  void EndFrame() override;
  void DrawResize() override;

  bool BitmapExtSupported(const char* ext) override;

protected:
  APIBitmap* LoadAPIBitmap(const char* fileNameOrResID, int scale, EResourceLocation location, const char* ext) override;
  APIBitmap* CreateAPIBitmap(int width, int height, int scale, double drawScale) override;

  bool LoadAPIFont(const char* fontID, const PlatformFontPtr& font) override;
    
  int AlphaChannel() const override { return 3; }
  bool FlippedBitmap() const override { return false; }

  void GetLayerBitmapData(const ILayerPtr& layer, RawBitmapData& data) override;
  void ApplyShadowMask(ILayerPtr& layer, RawBitmapData& mask, const IShadow& shadow) override;
    
  void DoMeasureText(const IText& text, const char* str, IRECT& bounds) const override;
  void DoDrawText(const IText& text, const char* str, const IRECT& bounds, const IBlend* pBlend) override;
  
private:
  void PathTransformSetMatrix(const IMatrix& m) override;
  void SetClipRegion(const IRECT& r) override;
  void UpdateLayer() override { /*UpdateGraphicsContext();*/ }
  
  CGContextRef mContext;
};
