/*
 ==============================================================================

 This file is part of the iPlug 2 library. Copyright (C) the iPlug 2 developers.

 See LICENSE.txt for  more info.

 ==============================================================================
*/

#include <cmath>

#include "IGraphicsQuartz.h"
#include "ITextEntryControl.h"


QuartzBitmap::QuartzBitmap(int scale, float drawScale)
{
}

QuartzBitmap::QuartzBitmap(int width, int height, int scale, float drawScale)
{
}
  
QuartzBitmap::~QuartzBitmap()
{
}

#pragma mark -

inline CGBlendMode QuartzBlendMode(const IBlend* pBlend)
{
  if (!pBlend)
  {
    return kCGBlendModeNormal;
  }
  switch (pBlend->mMethod)
  {
    case EBlend::Default:         // fall through
    case EBlend::Clobber:         // fall through
    case EBlend::SourceOver:      return kCGBlendModeNormal;
    case EBlend::SourceIn:        return kCGBlendModeSourceIn;
    case EBlend::SourceOut:       return kCGBlendModeSourceOut;
    case EBlend::SourceAtop:      return kCGBlendModeSourceAtop;
    case EBlend::DestOver:        return kCGBlendModeDestinationOver;
    case EBlend::DestIn:          return kCGBlendModeDestinationIn;
    case EBlend::DestOut:         return kCGBlendModeDestinationOut;
    case EBlend::DestAtop:        return kCGBlendModeDestinationAtop;
    case EBlend::Add:             return kCGBlendModePlusLighter; // ???
    case EBlend::XOR:             return kCGBlendModeXOR;
  }
}

#pragma mark -

IGraphicsQuartz::IGraphicsQuartz(IGEditorDelegate& dlg, int w, int h, int fps, float scale)
: IGraphicsPathBase(dlg, w, h, fps, scale)
{
  DBGMSG("IGraphics Quartz @ %i FPS\n", fps);
  
//  StaticStorage<QuartzFont>::Accessor storage(sFontCache);
//  storage.Retain();
}

IGraphicsQuartz::~IGraphicsQuartz()
{
//  StaticStorage<QuartzFont>::Accessor storage(sFontCache);
//  storage.Release();
}

void IGraphicsQuartz::DrawResize()
{
}

APIBitmap* IGraphicsQuartz::LoadAPIBitmap(const char* fileNameOrResID, int scale, EResourceLocation location, const char* ext)
{
  //
  return new QuartzBitmap(scale, 1.f);
}

APIBitmap* IGraphicsQuartz::CreateAPIBitmap(int width, int height, int scale, double drawScale)
{
  return new QuartzBitmap(width, height, scale, drawScale);
}

bool IGraphicsQuartz::BitmapExtSupported(const char* ext)
{
  char extLower[32];
  ToLower(extLower, ext);
  return (strstr(extLower, "png") != nullptr) /*|| (strstr(extLower, "jpg") != nullptr) || (strstr(extLower, "jpeg") != nullptr)*/;
}

void IGraphicsQuartz::GetLayerBitmapData(const ILayerPtr& layer, RawBitmapData& data)
{
}

void IGraphicsQuartz::ApplyShadowMask(ILayerPtr& layer, RawBitmapData& mask, const IShadow& shadow)
{
}

void IGraphicsQuartz::DrawBitmap(const IBitmap& bitmap, const IRECT& dest, int srcX, int srcY, const IBlend* pBlend)
{
}

void IGraphicsQuartz::PathClear()
{
}

void IGraphicsQuartz::PathClose()
{
}

void IGraphicsQuartz::PathArc(float cx, float cy, float r, float aMin, float aMax, EWinding winding)
{
}

void IGraphicsQuartz::PathMoveTo(float x, float y)
{
}

void IGraphicsQuartz::PathLineTo(float x, float y)
{
}

void IGraphicsQuartz::PathCubicBezierTo(float c1x, float c1y, float c2x, float c2y, float x2, float y2)
{
}

void IGraphicsQuartz::PathQuadraticBezierTo(float cx, float cy, float x2, float y2)
{
}

void IGraphicsQuartz::PathStroke(const IPattern& pattern, float thickness, const IStrokeOptions& options, const IBlend* pBlend)
{
}

void IGraphicsQuartz::PathFill(const IPattern& pattern, const IFillOptions& options, const IBlend* pBlend)
{
}

IColor IGraphicsQuartz::GetPoint(int x, int y)
{
  return COLOR_BLACK;
}

void IGraphicsQuartz::DoMeasureText(const IText& text, const char* str, IRECT& bounds) const
{
}

void IGraphicsQuartz::DoDrawText(const IText& text, const char* str, const IRECT& bounds, const IBlend* pBlend)
{
}

void IGraphicsQuartz::EndFrame()
{
  CGContextSetRGBFillColor (mContext, 1, 0, 0, 1);// 3
  CGContextFillRect (mContext, CGRectMake (0, 0, 200, 100 ));// 4
  CGContextSetRGBFillColor (mContext, 0, 0, 1, .5);// 5
  CGContextFillRect (mContext, CGRectMake (0, 0, 100, 200));
}

bool IGraphicsQuartz::LoadAPIFont(const char* fontID, const PlatformFontPtr& font)
{
  return false;
}

void IGraphicsQuartz::PathTransformSetMatrix(const IMatrix& m)
{
}

void IGraphicsQuartz::SetClipRegion(const IRECT& r)
{
}
