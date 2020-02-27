class IVPopupSelectionControl : public IVButtonControl
{
public:
  IVPopupSelectionControl(const IRECT& bounds, IPopupMenu* pMenu, const char* label = "", const IVStyle& style = DEFAULT_STYLE)
  : IVButtonControl(bounds, nullptr, label, style, false, true)
  , mMenuPtr(pMenu)
  {
    mDisablePrompt = false;
    
    SetActionFunction([&](IControl* pCaller){
      if(!mInPrompt) {
        SplashClickActionFunc(pCaller);
      }
    });
    
    SetAnimationEndActionFunction([&](IControl* pCaller) {
      if(!mInPrompt) {
        SetAnimation([&](IControl* pCaller) {
          if(pCaller->GetAnimationProgress() > 1.) {
            mInPrompt = true;
            pCaller->GetUI()->CreatePopupMenu(*this, *mMenuPtr, mRECT);
            //Don't call OnEndAnimationFunction()
            SetAnimation(nullptr);
            SetDirty(false);
            return;
          }
        }, DEFAULT_ANIMATION_DURATION);
      }
      else
        mInPrompt = false;
    });
  }
  
  void OnPopupMenuSelection(IPopupMenu* pSelectedMenu, int valIdx) override
  {
    mInPrompt = false;
  }
private:
  bool mInPrompt = false;
  IPopupMenu* mMenuPtr;
};