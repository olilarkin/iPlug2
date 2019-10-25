
#ifdef IGRAPHICS_SKIA
#define SK_METAL
#include "include/core/SkRefCnt.h"
#include "include/core/SkSurfaceProps.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkSurface.h"
#include "include/gpu/GrContext.h"
#include "include/gpu/GrTypes.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkSurfaceProps.h"
#include "include/gpu/GrContextOptions.h"

struct DisplayParams {
  DisplayParams()
  : fColorType(kN32_SkColorType)
  , fColorSpace(nullptr)
  , fMSAASampleCount(1)
  , fSurfaceProps(SkSurfaceProps::kLegacyFontHost_InitType)
  , fDisableVsync(false)
  {}
  
  SkColorType         fColorType;
  sk_sp<SkColorSpace> fColorSpace;
  int                 fMSAASampleCount;
  GrContextOptions    fGrContextOptions;
  SkSurfaceProps      fSurfaceProps;
  bool                fDisableVsync;
};

#endif

#ifdef IGRAPHICS_NANOVG
struct NVGcontext;
#endif

#include "IControl.h"
#include "IPlugTimer.h"

using namespace iplug;
using namespace igraphics;


class MyControl : public IControl
{
public:
  MyControl(const IRECT& bounds)
  : IControl(bounds) {
  }
  
  void OnInit() override;
  void OnMouseDown(float x, float y, const IMouseMod& mod) override;
  
  void Draw(IGraphics& g) override;
  
  void DrawToCALayer();
  
  void OnResize() override;
  
private:
#ifdef IGRAPHICS_SKIA
  GrMTLHandle mCALayerPtr;
  GrMTLHandle mDrawable;
  sk_sp<GrContext> fContext;
  int fWidth;
  int fHeight;
  DisplayParams fDisplayParams;
  int fSampleCount;
  int fStencilBits;
#endif
  
#ifdef IGRAPHICS_NANOVG
  NVGcontext* mVG = nullptr;
  CALayerPtr mCALayer;
#endif
  
  Timer* mTimer;
  

};
