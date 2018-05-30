#pragma once

#ifndef NO_IGRAPHICS

#include "IGraphics_select.h"

/** IGraphics platform class for linux
*   @ingroup PlatformClasses
*/
class IGraphicsLinux final : public IGRAPHICS_DRAW_CLASS
{
public:
  IGraphicsLinux(IGraphicsEditorDelegate& dlg, int w, int h, int fps)  final;
  virtual ~IGraphicsLinux);

  void* OpenWindow(void* pWindow) override;
  void CloseWindow() override;
  bool WindowIsOpen() override;
  void Resize(int w, int h, float scale) override;

  void HideMouseCursor() override;
  void ShowMouseCursor() override;

  int ShowMessageBox(const char* str, const char* caption, int type) override;
  void ForceEndUserEdit() override;

  const char* GetPlatformAPIStr() override;

  void UpdateTooltips() override;

  bool RevealPathInExplorerOrFinder(WDL_String& path, bool select) override;
  void PromptForFile(WDL_String& fileName, WDL_String& path, EFileAction action,  const char* ext) override;
  bool PromptForColor(IColor& color, const char* str) override;

  IPopupMenu* CreatePopupMenu(const IPopupMenu& menu, IRECT& bounds) override;
  void CreateTextEntry(IControl* pControl, const IText& text, const IRECT& bounds, const char* str) override;

  bool OpenURL(const char* url, const char* msgWindowTitle, const char* confirmMsg, const char* errMsgOnFailure) override;

  static int GetUserOSVersion();
  bool GetTextFromClipboard(WDL_String& str) override;

protected:
  bool OSFindResource(const char* name, const char* type, WDL_String& result) override;
}

#endif // NO_IGRAPHICS