#include "MyControl.h"

#import <MetalKit/MetalKit.h>
#import <QuartzCore/QuartzCore.h>

#include "IGraphicsSkia.h"

id<MTLCommandQueue> fQueue;

void MyControl::OnInit()
{
  mCALayerPtr = GetUI()->AttachPlatformLayer(GetRECT(), true);
  fQueue = [((CAMetalLayer*) mCALayerPtr).device newCommandQueue];
  
  mTimer = Timer::Create([this](Timer& t)
                         {
                           DrawToCALayer();
                         }, 100);
}


void draw(SkCanvas* canvas) {
  SkPaint p;
  
  static bool test = false;

  p.setColor(test ? SK_ColorGREEN : SK_ColorRED);
  
  test = !test;
  
  p.setAntiAlias(true);
  p.setStyle(SkPaint::kFill_Style);

//  canvas->drawCircle({50,50}, 10, p);
  canvas->drawRect({0,0,100,100}, p);

  canvas->flush();
}

void MyControl::DrawToCALayer()
{
  @autoreleasepool {
    id<CAMetalDrawable> currentDrawable = [(CAMetalLayer*) mCALayerPtr nextDrawable];
    mDrawable = (__bridge GrMTLHandle) currentDrawable;
    sk_sp<SkSurface> surface = SkSurface::MakeFromCAMetalLayer(static_cast<IGraphicsSkia*>(GetUI())->GetGrContext(), mCALayerPtr, kTopLeft_GrSurfaceOrigin, fSampleCount, kBGRA_8888_SkColorType,
                                                               fDisplayParams.fColorSpace,
                                                               &fDisplayParams.fSurfaceProps,
                                                               &mDrawable);
    
    if(mDrawable) {
      draw(surface->getCanvas());
      
      id<CAMetalDrawable> currentDrawable = (__bridge id<CAMetalDrawable>) mDrawable;
      
      id<MTLCommandBuffer> commandBuffer = [fQueue commandBuffer];
      commandBuffer.label = @"Present";
      
      [commandBuffer presentDrawable:currentDrawable];
      [commandBuffer commit];
    }
    
    [(CAMetalLayer*) mCALayerPtr setNeedsDisplay];
  }
}

void MyControl::Draw(IGraphics& g)
{
}

void MyControl::OnMouseDown(float x, float y, const IMouseMod& mod)
{
  DrawToCALayer();
}

void MyControl::OnResize()
{
  CAMetalLayer* layer = static_cast<CAMetalLayer*>(mCALayerPtr);
  
  [CATransaction begin];
  [CATransaction setValue: (id) kCFBooleanTrue forKey: kCATransactionDisableActions];
//  layer.position = CGPointMake(mRECT.L, mRECT.T);
  layer.frame = CGRectMake(mRECT.L, mRECT.T, mRECT.W(), mRECT.H());
  [CATransaction commit];
}
