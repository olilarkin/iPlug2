#include <cstdio>
#include <algorithm>

#include "IPlugParameter.h"

#pragma mark - Shape

// Linear
double IParam::ShapeLinear::NormalizedToValue(double value, const IParam& param) const
{
  return param.mMin + value * (param.mMax - param.mMin);
}

double IParam::ShapeLinear::ValueToNormalized(double value, const IParam& param) const
{
  return (value - param.mMin) / (param.mMax - param.mMin);
}

// Power curve shape
IParam::ShapePowCurve::ShapePowCurve(double shape)
: mShape(shape)
{
}

IParam::EDisplayType IParam::ShapePowCurve::GetDisplayType() const
{
  if (mShape > 2.5) return kDisplayCubeRoot;
  if (mShape > 1.5) return kDisplaySquareRoot;
  if (mShape < (2.0 / 5.0)) return kDisplayCubed;
  if (mShape < (2.0 / 3.0)) return kDisplaySquared;
  
  return IParam::kDisplayLinear;
}

double IParam::ShapePowCurve::NormalizedToValue(double value, const IParam& param) const
{
  return param.GetMin() + std::pow(value, mShape) * (param.GetMax() - param.GetMin());
}

double IParam::ShapePowCurve::ValueToNormalized(double value, const IParam& param) const
{
  return std::pow((value - param.GetMin()) / (param.GetMax() - param.GetMin()), 1.0 / mShape);
}

// Exponential shape
void IParam::ShapeExp::Init(const IParam& param)
{
  mAdd = std::log(param.GetMin());
  mMul = std::log(param.GetMax() / param.GetMin());
}

double IParam::ShapeExp::NormalizedToValue(double value, const IParam& param) const
{
  return std::exp(mAdd + value * mMul);
}

double IParam::ShapeExp::ValueToNormalized(double value, const IParam& param) const
{
  return (std::log(value) - mAdd) / mMul;
}

#pragma mark -

IParam::IParam()
{
  mShape = new ShapeLinear;
  memset(mName, 0, MAX_PARAM_NAME_LEN * sizeof(char));
  memset(mLabel, 0, MAX_PARAM_LABEL_LEN * sizeof(char));
  memset(mParamGroup, 0, MAX_PARAM_LABEL_LEN * sizeof(char));
};

void IParam::InitBool(const char* name, bool defaultVal, const char* label, int flags, const char* group, const char* offText, const char* onText)
{
  if (mType == kTypeNone) mType = kTypeBool;
  
  InitEnum(name, (defaultVal ? 1 : 0), 2, label, flags | kFlagStepped, group);

  SetDisplayText(0, offText);
  SetDisplayText(1, onText);
}

void IParam::InitEnum(const char* name, int defaultVal, int nEnums, const char* label, int flags, const char* group, const char* listItems, ...)
{
  if (mType == kTypeNone) mType = kTypeEnum;
  
  InitInt(name, defaultVal, 0, nEnums - 1, label, flags | kFlagStepped, group);
  
  if(listItems)
  {
    SetDisplayText(0, listItems);

    va_list args;
    va_start(args, listItems);
    for (auto i = 1; i < nEnums; ++i)
      SetDisplayText(i, va_arg(args, const char*));
    va_end(args);
  }
}

void IParam::InitInt(const char* name, int defaultVal, int minVal, int maxVal, const char* label, int flags, const char* group)
{
  if (mType == kTypeNone) mType = kTypeInt;
  
  InitDouble(name, (double) defaultVal, (double) minVal, (double) maxVal, 1.0, label, flags | kFlagStepped, group);
}

void IParam::InitDouble(const char* name, double defaultVal, double minVal, double maxVal, double step, const char* label, int flags, const char* group, Shape* shape, EParamUnit unit, DisplayFunc displayFunc)
{
  if (mType == kTypeNone) mType = kTypeDouble;
  
//  assert(CStringHasContents(mName) && "Parameter already initialised!");
//  assert(CStringHasContents(name) && "Parameter must be given a name!");

  strcpy(mName, name);
  strcpy(mLabel, label);
  strcpy(mParamGroup, group);
  
  // N.B. apply stepping and constraints to the default value (and store the result)
  mMin = minVal;
  mMax = std::max(maxVal, minVal + step);
  mStep = step;
  mDefault = mValue;
  mUnit = unit;
  mFlags = flags;
  mDisplayFunction = displayFunc;

  Set(defaultVal);
  
  for (mDisplayPrecision = 0;
       mDisplayPrecision < MAX_PARAM_DISPLAY_PRECISION && step != floor(step);
       ++mDisplayPrecision, step *= 10.0)
  {
    ;
  }
    
  if (shape)
  {
    delete mShape;
    mShape = shape;
  }
  
  mShape->Init(*this);
}

void IParam::InitFrequency(const char *name, double defaultVal, double minVal, double maxVal, double step, int flags, const char *group)
{
  InitDouble(name, defaultVal, minVal, maxVal, step, "Hz", flags, group, new ShapeExp, kUnitFrequency);
}

void IParam::InitSeconds(const char *name, double defaultVal, double minVal, double maxVal, double step, int flags, const char *group)
{
  InitDouble(name, defaultVal, minVal, maxVal, step, "Seconds", flags, group, nullptr, kUnitSeconds);
}

void IParam::InitPitch(const char *name, int defaultVal, int minVal, int maxVal, int flags, const char *group)
{
  InitEnum(name, defaultVal, (maxVal - minVal) + 1, "", flags, group);
  WDL_String displayText;
  for (auto i = minVal; i <= maxVal; i++)
  {
    MidiNoteName(i, displayText);
    SetDisplayText(i - minVal, displayText.Get());
  }
}

void IParam::InitGain(const char *name, double defaultVal, double minVal, double maxVal, double step, int flags, const char *group)
{
  InitDouble(name, defaultVal, minVal, maxVal, step, "dB", flags, group, nullptr, kUnitDB);
}

void IParam::InitPercentage(const char *name, double defaultVal, double minVal, double maxVal, int flags, const char *group)
{
  InitDouble(name, defaultVal, minVal, maxVal, 1, "%", flags, group, nullptr, kUnitPercentage);
}

void IParam::SetDisplayText(double value, const char* str)
{
  int n = mDisplayTexts.GetSize();
  mDisplayTexts.Resize(n + 1);
  DisplayText* pDT = mDisplayTexts.Get() + n;
  pDT->mValue = value;
  strcpy(pDT->mText, str);
}

void IParam::GetDisplayForHost(double value, bool normalized, WDL_String& str, bool withDisplayText) const
{
  if (normalized) value = FromNormalized(value);

  if (mDisplayFunction != nullptr)
  {
    mDisplayFunction(value, str);
    return;
  }

  if (withDisplayText)
  {
    const char* displayText = GetDisplayText(value);

    if (CStringHasContents(displayText))
    {
      str.Set(displayText, MAX_PARAM_DISPLAY_LEN);
      return;
    }
  }

  double displayValue = value;

  if (mFlags & kFlagNegateDisplay)
    displayValue = -displayValue;

  // Squash all zeros to positive
  if (!displayValue) displayValue = 0.0;

  if (mDisplayPrecision == 0)
  {
    str.SetFormatted(MAX_PARAM_DISPLAY_LEN, "%d", int(round(displayValue)));
  }
  else if ((mFlags & kFlagSignDisplay) && displayValue)
  {
    char fmt[16];
    sprintf(fmt, "%%+.%df", mDisplayPrecision);
    str.SetFormatted(MAX_PARAM_DISPLAY_LEN, fmt, displayValue);
  }
  else
  {
    str.SetFormatted(MAX_PARAM_DISPLAY_LEN, "%.*f", mDisplayPrecision, displayValue);
  }
}

const char* IParam::GetNameForHost() const
{
  return mName;
}

const char* IParam::GetLabelForHost() const
{
  return (CStringHasContents(GetDisplayText((int) mValue))) ? "" : mLabel;
}

const char* IParam::GetGroupForHost() const
{
  return mParamGroup;
}

int IParam::NDisplayTexts() const
{
  return mDisplayTexts.GetSize();
}

const char* IParam::GetDisplayText(double value) const
{
  int n = mDisplayTexts.GetSize();
  for (DisplayText* pDT = mDisplayTexts.Get(); n; --n, ++pDT)
  {
    if (value == pDT->mValue) return pDT->mText;
  }
  return "";
}

const char* IParam::GetDisplayTextAtIdx(int idx, double* pValue) const
{
  DisplayText* pDT = mDisplayTexts.Get()+idx;
  if (pValue) *pValue = pDT->mValue;
  return pDT->mText;
}

bool IParam::MapDisplayText(const char* str, double* pValue) const
{
  int n = mDisplayTexts.GetSize();
  for (DisplayText* pDT = mDisplayTexts.Get(); n; --n, ++pDT)
  {
    if (!strcmp(str, pDT->mText))
    {
      *pValue = pDT->mValue;
      return true;
    }
  }
  return false;
}

double IParam::StringToValue(const char* str) const
{
  double v = 0.;
  bool mapped = (bool) NDisplayTexts();

  if (mapped)
    mapped = MapDisplayText(str, &v);

  if (!mapped && Type() != kTypeEnum && Type() != kTypeBool)
  {
    v = atof(str);

    if (mFlags & kFlagNegateDisplay)
      v = -v;

    v = Constrain(v);
    mapped = true;
  }

  return v;
}

void IParam::GetBounds(double& lo, double& hi) const
{
  lo = mMin;
  hi = mMax;
}

void IParam::GetJSON(WDL_String& json, int idx) const
{
  json.AppendFormatted(8192, "{");
  json.AppendFormatted(8192, "\"id\":%i, ", idx);
  json.AppendFormatted(8192, "\"name\":\"%s\", ", GetNameForHost());
  switch (Type())
  {
    case IParam::kTypeNone:
      break;
    case IParam::kTypeBool:
      json.AppendFormatted(8192, "\"type\":\"%s\", ", "bool");
      break;
    case IParam::kTypeInt:
      json.AppendFormatted(8192, "\"type\":\"%s\", ", "int");
      break;
    case IParam::kTypeEnum:
      json.AppendFormatted(8192, "\"type\":\"%s\", ", "enum");
      break;
    case IParam::kTypeDouble:
      json.AppendFormatted(8192, "\"type\":\"%s\", ", "float");
      break;
    default:
      break;
  }
  json.AppendFormatted(8192, "\"min\":%f, ", GetMin());
  json.AppendFormatted(8192, "\"max\":%f, ", GetMax());
  json.AppendFormatted(8192, "\"default\":%f, ", GetDefault());
  json.AppendFormatted(8192, "\"rate\":\"audio\"");
  json.AppendFormatted(8192, "}");
}
