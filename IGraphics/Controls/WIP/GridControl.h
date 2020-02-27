class GridControl : public IControl
{
public:
  GridControl(const IRECT& bounds, int rows, int columns, const IPattern& onColor = COLOR_WHITE, const IPattern& offColor = COLOR_BLACK)
  : IControl(bounds)
  , mRows(rows)
  , mColumns(columns)
  , mOnColor(onColor)
  , mOffColor(offColor)
  {
  }
  
  void Draw(IGraphics& g) override
  {
    int ncells = mRows * mColumns;

    g.PathRect(mRECT);
    g.PathFill(mOffColor);
    
    for (int i=0; i < ncells; i++)
    {
      if(mCells[i].state)
      {
        auto rect = mCells[i].r.GetPixelSnapped();
        
        if(mEllipses)
          g.PathEllipse(rect);
        else
          g.PathRect(rect);

        if(mCells[i].state == 1)
        {
          g.PathStroke(mOnColor, mStrokeThickness);
        }
        else
        {
          IFillOptions fillOptions;
          fillOptions.mPreserve = true;
          g.PathFill(mOnColor, fillOptions);
          g.PathStroke(mOnColor, mStrokeThickness);
        }
      }
    }
  }
  
  void OnMouseDown(float x, float y, const IMouseMod& mod) override
  {
    DoSet(x, y, mod, false);
  }
  
  void OnMouseDrag(float x, float y, float dx, float dy, const IMouseMod& mod) override
  {
    DoSet(x, y, mod, true);
  }
  
  void OnResize() override
  {
    MakeGrid(mRows, mColumns);
  }
  
  void SetRowBits(int rowIdx, int64_t bits)
  {
  }

protected:
  struct cell
  {
    IRECT r;
    int state = 0;
    std::vector<cell> mCells;
    int idx, row, col;
    
    cell(const IRECT& r, int idx, int row, int col) : r(r), idx(idx), row(row), col(col) {}
  };
  
  cell* GetCell(float x, float y)
  {
    int ncells = mRows * mColumns;

    for (int i=0; i < ncells; i++)
    {
      if(mCells[i].r.Contains(x, y))
      {
        return &mCells[i];
      }
    }
    return nullptr;
  }
  
  void DoSet(float x, float y, const IMouseMod& mod, bool isDrag)
  {
    auto* hitCell = GetCell(x, y);
    
    if(mod.S || mod.C)
    {
      
      if(hitCell)
        mod.S ? SetRow(hitCell->row, !mod.R) : SetCol(hitCell->col, !mod.R);
    }
    else {
      if(hitCell)
      {
        if(isDrag && hitCell->state == 1)
          return;
        
        SetCell(hitCell->idx, !mod.R);
      }
    }
    
    SetDirty();
  }
  
  void SetCell(int cell, bool filled)
  {
    
    if(filled)
      mCells[cell].state++;
    else
      mCells[cell].state--;

    mCells[cell].state = Clip(mCells[cell].state, 0, 2);
  }
  
  void SetRow(int row, bool filled)
  {
    int ncells = mRows * mColumns;
    
    for (int i=0; i < ncells; i++)
    {
      if(mCells[i].row == row)
      {
        SetCell(i, filled);
      }
    }
  }
  
  void SetCol(int col, bool filled)
  {
    int ncells = mRows * mColumns;
    
    for (int i=0; i < ncells; i++)
    {
      if(mCells[i].col == col)
      {
        SetCell(i, filled);
      }
    }
  }

  void MakeGrid(int rows, int columns)
  {
    
    if(mCells.empty() || rows != mRows || columns != mColumns )
    {
      mCells.clear();
      int idx = 0;
      for (int r = 0; r < mRows; r++)
      {
        for (int c = 0; c < mColumns; c++)
        {
          IRECT bounds = mRECT.GetGridCell(r, c, mRows, mColumns);
          mCells.push_back({bounds, idx++, r, c});
          bounds.DBGPrint();
        }
      }
    }
    else
    {
      int ncells = mRows * mColumns;
      for (int i=0; i < ncells; i++)
      {
        mCells[i].r = mRECT.GetGridCell(i, rows, columns);
      }
    }
    
    mRows = rows;
    mColumns = columns;
  }
  
  std::vector<cell> mCells;
  int mRows;
  int mColumns;
  IPattern mOnColor;
  IPattern mOffColor;
  bool mEllipses = false;
  float mStrokeThickness = 2.f;
};
