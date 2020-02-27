class ISwipePanelControl : public IControl
{
public:
  ISwipePanelControl(const IRECT& bounds, const std::initializer_list<IPattern>& panels)
  : IControl(bounds, kNoParameter)
  , mPanels(panels)
  {
  }

  void Draw(IGraphics& g) override
  {
    g.PathTransformSave();

    float frac = 0.;
    if(mDirection==0)
      frac = GetAnimationProgress();
    else
      frac = 1. - GetAnimationProgress();

    g.PathTransformTranslate(mRECT.L - (frac * mRECT.W()), mRECT.T);

    float offset = 0.;

    g.PathRect(IRECT(offset, 0., offset + mRECT.W(), mRECT.H()));
    g.PathFill(mPanels[mDirection ? mNextPanel : mSelectedPanel]);
    offset += mRECT.W();

    g.PathRect(IRECT(offset, 0., offset + mRECT.W(), mRECT.H()));
    g.PathFill(mPanels[mDirection ? mSelectedPanel : mNextPanel]);

    g.PathTransformRestore();;
  }

  void OnEndAnimation() override
  {
    IControl::OnEndAnimation();
    mSelectedPanel = mNextPanel;
    GetUI()->SetAllControlsDirty();
  }

  void OnInit() override
  {
    auto gestureFunc = [&](IControl* pCaller, const IGestureInfo& info) {
      switch(info.type)
      {
        case EGestureType::SwipeLeft:
        {
          mNextPanel = (mSelectedPanel + 1) % mPanels.size();
          mDirection = 0;
          break;
        }
        case EGestureType::SwipeRight:
        {
          mNextPanel = (mSelectedPanel - 1);
          if(mNextPanel < 0)
            mNextPanel += mPanels.size();
          mDirection = 1;
          break;
        }
        default:
          break;
      }
      pCaller->SetAnimation(DefaultAnimationFunc, 300);
    };

    AttachGestureRecognizer(EGestureType::SwipeLeft, gestureFunc); // AttachGestureRecognizer calles GetUI(), not know till Init
    AttachGestureRecognizer(EGestureType::SwipeRight, gestureFunc);
  }

private:
  std::vector<IPattern> mPanels;
  int mSelectedPanel = 0;
  int mNextPanel = 0;
  int mDirection = 0;
};
