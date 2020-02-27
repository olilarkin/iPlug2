class SliderTicks : public IControl {
public:
  SliderTicks(const IRECT& bounds)
  : IControl(bounds) {
  }

  void Draw(IGraphics& g) override
  {
    if(!g.CheckLayer(mLayer))
    {
      g.StartLayer(this, mRECT);

      float yincr = 1.f/10.f;
  //      auto text = IText(15.f, EVAlign::Middle).WithAlign(EAlign::Far);

      for (int i=0; i<=10; i++) {
        float ypos = (float)i/10.;

        g.DrawHorizontalLine(COLOR_BLACK, mRECT, ypos, &BLEND_50, 2.f);

        for (int j=0; j<4; j++) {
          g.DrawHorizontalLine(COLOR_BLACK, mRECT.GetHPadded(-10.f), ypos + ((yincr/5.f) * (j+1)), &BLEND_25, 1.f);
        }

        WDL_FastString str;
        str.SetFormatted(32, "%i", i);
        g.DrawRect(COLOR_RED, mRECT.GetFromRight(20.f).SubRectVertical(11, i));
  //        g.DrawText(text, str.Get(), b.GetFromRight(20.f).SubRectVertical(11, i));
      }
  //      g.PathClipRegion();

      g.FillRect(COLOR_GRAY, mRECT.GetVPadded(10.f).GetMidHPadded(10.f));

      mLayer = g.EndLayer();
    }
    else
      g.DrawLayer(mLayer);
  }

private:
  ILayerPtr mLayer;
};