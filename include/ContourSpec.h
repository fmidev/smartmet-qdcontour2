// ----------------------------------------------------------------------
// Yksitt�isen parametrin piirto-ohjeet
// ----------------------------------------------------------------------

#ifndef CONTOURSPEC_H
#define CONTOURSPEC_H

#include "ContourFont.h"
#include "ContourLabel.h"
#include "ContourPattern.h"
#include "ContourRange.h"
#include "ContourSymbol.h"
#include "ContourValue.h"

#include "NFmiDataMatrix.h"
#include "NFmiPoint.h"

#include <list>
#include <string>
#include <vector>

class ContourSpec
{
 public:
#ifdef COMPILER_GENERATED
  ~ContourSpec();
  ContourSpec(const ContourSpec& theValue);
  ContourSpec& operator=(const ContourSpec& theValue);
#endif

  ContourSpec(const std::string& theParam,
              const std::string& theInterpolation,
              const std::string& theSmoother,
              int theLevel = -1,
              float theSmootherRadius = 1.0,
              int theSmootherFactor = 1,
              float theHiLimit = kFloatMissing);

  const std::list<ContourRange>& contourFills(void) const;
  const std::list<ContourPattern>& contourPatterns(void) const;
  const std::list<ContourValue>& contourValues(void) const;
  const std::list<ContourSymbol>& contourSymbols(void) const;
  const std::list<ContourLabel>& contourLabels(void) const;
  const std::list<ContourFont>& contourFonts(void) const;
  const std::map<float, std::string>& contourLabelTexts() const;

  const std::string& param(void) const;
  int level() const;
  const std::string& contourInterpolation(void) const;
  const std::string& smoother(void) const;
  float smootherRadius(void) const;
  int smootherFactor(void) const;
  float exactHiLimit(void) const;
  float dataHiLimit(void) const;
  float dataLoLimit(void) const;

  const std::string& overlay(void) const;

  void level(int theValue);
  void contourInterpolation(const std::string& theValue);
  void smoother(const std::string& theValue);
  void smootherRadius(float theRadius);
  void smootherFactor(int theFactor);
  void exactHiLimit(float theLimit);
  void overlay(const std::string& theOverlay);

  void dataLoLimit(float theLimit);
  void dataHiLimit(float theLimit);

  void add(const ContourRange& theRange);
  void add(const ContourValue& theValue);
  void add(const ContourPattern& theValue);
  void add(const ContourSymbol& theValue);
  void add(const ContourLabel& theValue);
  void add(const ContourFont& theValue);

  // This was done to replace 32700 with -1 in PrecipitationForm

  bool replace(void) const;
  float replaceSourceValue(void) const;
  float replaceTargetValue(void) const;

  void replace(float theSrcValue, float theDstValue);

  // Despeckling

  void despeckle(
      float theLoLimit, float theHiLimit, int theRadius, float theWeight, int theIterations);

  void despeckle(NFmiDataMatrix<float>& theValues) const;

  // Label specific methods

  const std::list<std::pair<NFmiPoint, NFmiPoint> >& labelPoints(void) const;

  void add(const NFmiPoint& thePoint,
           const NFmiPoint theXY = NFmiPoint(kFloatMissing, kFloatMissing));

  const std::vector<float>& labelValues(void) const;

  // labelsxy command methods
  const std::list<std::pair<NFmiPoint, float> >& pixelLabels(void) const;
  void addPixelLabel(const NFmiPoint& thePoint, float value);
  void clearPixelLabels(void);

  void addLabelValue(float theValue);
  void clearLabelValues(void);
  void clearLabels(void);

  const std::string& labelMarker(void) const;
  const std::string& labelMarkerRule(void) const;
  float labelMarkerAlphaFactor(void) const;
  const std::string& labelFont(void) const;
  int labelColor(void) const;
  const std::string& labelRule(void) const;

  const std::string& labelAlignment(void) const;
  const std::string& labelFormat(void) const;
  const std::string& labelMissing(void) const;
  float labelOffsetX(void) const;
  float labelOffsetY(void) const;
  float labelDX(void) const;
  float labelDY(void) const;

  float labelXyX0(void) const;
  float labelXyY0(void) const;
  float labelXyDX(void) const;
  float labelXyDY(void) const;

  std::string labelCaption(void) const;
  float labelCaptionDX(void) const;
  float labelCaptionDY(void) const;
  std::string labelCaptionAlignment(void) const;

  void labelMarker(const std::string& theValue);
  void labelMarkerRule(const std::string& theValue);
  void labelMarkerAlphaFactor(float theValue);
  void labelFont(const std::string& theValue);
  void labelColor(int theValue);
  void labelRule(const std::string& theValue);
  void labelFillRule(const std::string& theValue);
  void labelAlignment(const std::string& theValue);
  void labelFormat(const std::string& theValue);
  void labelMissing(const std::string& theValue);
  void labelOffsetX(float theValue);
  void labelOffsetY(float theValue);
  void labelDX(float theValue);
  void labelDY(float theValue);
  void labelXyX0(float theValue);
  void labelXyY0(float theValue);
  void labelXyDX(float theValue);
  void labelXyDY(float theValue);

  void labelCaption(const std::string& theValue);
  void labelCaptionDX(float theValue);
  void labelCaptionDY(float theValue);
  void labelCaptionAlignment(const std::string& theValue);

  const std::string& contourLabelFont() const;
  int contourLabelColor() const;
  int contourLabelBackgroundColor() const;
  int contourLabelBackgroundXMargin() const;
  int contourLabelBackgroundYMargin() const;

  void contourLabelFont(const std::string& theValue);
  void contourLabelColor(int theValue);
  void contourLabelBackgroundColor(int theValue);
  void contourLabelBackgroundXMargin(int theValue);
  void contourLabelBackgroundYMargin(int theValue);

  void addContourLabelText(float theValue, const std::string& theText);

 private:
  ContourSpec(void);

  std::string itsParam;
  int itsLevel;
  std::string itsContourInterpolation;
  std::string itsSmoother;
  float itsSmootherRadius;
  int itsSmootherFactor;

  std::list<ContourRange> itsContourFills;
  std::list<ContourValue> itsContourValues;
  std::list<ContourPattern> itsContourPatterns;
  std::list<ContourSymbol> itsContourSymbols;
  std::list<ContourLabel> itsContourLabels;
  std::list<ContourFont> itsContourFonts;

  float itsExactHiLimit;
  float itsDataLoLimit;
  float itsDataHiLimit;

  bool itHasReplace;
  float itsReplaceSourceValue;
  float itsReplaceTargetValue;

  bool itHasDespeckle;
  float itsDespeckleLoLimit;
  float itsDespeckleHiLimit;
  int itsDespeckleRadius;
  float itsDespeckleWeight;
  int itsDespeckleIterations;

  // LatLon, optional label location pairs in pixels
  std::list<std::pair<NFmiPoint, NFmiPoint> > itsLabelPoints;
  // Respective values calculated while contouring
  std::vector<float> itsLabelValues;

  // And then labels for specific XY-coordinates
  std::list<std::pair<NFmiPoint, float> > itsPixelLabels;

  std::string itsLabelMarker;
  std::string itsLabelMarkerRule;
  float itsLabelMarkerAlphaFactor;
  std::string itsLabelFont;
  int itsLabelColor;
  std::string itsLabelRule;
  std::string itsLabelAlignment;
  std::string itsLabelFormat;
  std::string itsLabelMissing;
  float itsLabelOffsetX;
  float itsLabelOffsetY;
  float itsLabelDX;
  float itsLabelDY;
  float itsLabelXyX0;
  float itsLabelXyY0;
  float itsLabelXyDX;
  float itsLabelXyDY;

  std::string itsLabelCaption;
  float itsLabelCaptionDX;
  float itsLabelCaptionDY;
  std::string itsLabelCaptionAlignment;

  std::string itsContourLabelFont;
  int itsContourLabelColor;
  int itsContourLabelBackgroundColor;
  int itsContourLabelBackgroundXMargin;
  int itsContourLabelBackgroundYMargin;
  std::map<float, std::string> itsContourLabelTexts;

  std::string itsOverlay;
};

#endif  // CONTOURSPEC_H

// ======================================================================
