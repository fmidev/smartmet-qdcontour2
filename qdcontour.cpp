// ======================================================================
/*!
 * \file
 * \brief Main program for qdcontour
 */
// ======================================================================

#include "Globals.h"
#include "ColorTools.h"
#include "ContourCalculator.h"
#include "ContourSpec.h"
#include "GramTools.h"
#include "LazyQueryData.h"
#include "MetaFunctions.h"
#include "ProjectionFactory.h"
#include "ShapeSpec.h"
#include "TimeTools.h"

#include "imagine/NFmiColorTools.h"
#include "imagine/NFmiImage.h"			// for rendering
#include "imagine/NFmiGeoShape.h"		// for esri data
#include "imagine/NFmiText.h"			// for labels
#include "imagine/NFmiFontHershey.h"	// for Hershey fonts

#include "newbase/NFmiAreaFactory.h"
#include "newbase/NFmiCmdLine.h"			// command line options
#include "newbase/NFmiDataMatrix.h"
#include "newbase/NFmiDataModifierClasses.h"
#include "newbase/NFmiEnumConverter.h"		// FmiParameterName<-->string
#include "newbase/NFmiFileSystem.h"			// FileExists()
#include "newbase/NFmiLatLonArea.h"			// Geographic projection
#include "newbase/NFmiSettings.h"			// Configuration
#include "newbase/NFmiSmoother.h"		// for smoothing data
#include "newbase/NFmiStereographicArea.h"	// Stereographic projection
#include "newbase/NFmiStringTools.h"
#include "newbase/NFmiPreProcessor.h"

#include "boost/shared_ptr.hpp"

#include <fstream>
#include <list>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>

using namespace std;
using namespace boost;
using namespace Imagine;

// ----------------------------------------------------------------------
// Global instance of global variables
// ----------------------------------------------------------------------

static Globals globals;

// ----------------------------------------------------------------------
// Usage
// ----------------------------------------------------------------------

void Usage(void)
{
  cout << "Usage: qdcontour [conffiles]" << endl << endl;
  cout << "Commands in configuration files:" << endl << endl;
}

// ----------------------------------------------------------------------
/*!
 * Test whether the given pixel coordinate is masked. This by definition
 * means the respective pixel in the given mask is not fully transparent.
 * Also, we define all pixels outside the mask image to be masked similarly
 * as pixel(0,0).
 *
 * \param thePoint The pixel coordinate
 * \param theMask The mask filename
 * \param theMaskImage The mask image
 * \return True, if the pixel is masked out
 */
// ----------------------------------------------------------------------

bool IsMasked(const NFmiPoint & thePoint,
			  const std::string & theMask,
			  const NFmiImage & theMaskImage)
{
  if(theMask.empty())
	return false;

  long theX = static_cast<int>(FmiRound(thePoint.X()));
  long theY = static_cast<int>(FmiRound(thePoint.Y()));

  // Handle outside pixels the same way as pixel 0,0
  if( theX<0 ||
	  theY<0 ||
	  theX>=theMaskImage.Width() ||
	  theY>=theMaskImage.Height())
	{
	  theX = 0;
	  theY = 0;
	}

  const NFmiColorTools::Color c = theMaskImage(theX,theY);
  const int alpha = NFmiColorTools::GetAlpha(c);

  return (alpha != NFmiColorTools::Transparent);
}

// ----------------------------------------------------------------------
/*!
 * \brief Parse the command line options
 */
// ----------------------------------------------------------------------

void parse_command_line(int argc, const char * argv[])
{

  NFmiCmdLine cmdline(argc,argv,"vfq!");
  
  if(cmdline.NumberofParameters() == 0)
	throw runtime_error("Atleast one command line parameter is required");
  
  // Check for parsing errors
  
  if(cmdline.Status().IsError())
	throw runtime_error(cmdline.Status().ErrorLog().CharPtr());
  
  // Read -v option
  
  if(cmdline.isOption('v'))
    globals.verbose = true;
  
  // Read -f option
  
  if(cmdline.isOption('f'))
    globals.force = true;
  
  if(cmdline.isOption('q'))
	globals.cmdline_querydata = cmdline.OptionValue('q');
  
  // Read command filenames
  
  for(int i=1; i<=cmdline.NumberofParameters(); i++)
	globals.cmdline_files.push_back(cmdline.Parameter(i));

}

// ----------------------------------------------------------------------
// Main program.
// ----------------------------------------------------------------------

int domain(int argc, const char *argv[])
{
  // Initialize configuration variables
  
  NFmiSettings::Init();

  // Parse command line

  parse_command_line(argc,argv);

  // Tallennetut kontuurit

  ContourCalculator theCalculator;

  // Aktiiviset contour-speksit (ja label speksit)
  
  list<ContourSpec> theSpecs;
  
  // Aktiiviset shape-speksit
  
  list<ShapeSpec> theShapeSpecs;
  
  // Aktiiviset tuulinuolet
  
  list<NFmiPoint> theArrowPoints;
  
  // Komentotiedostosta luettavat optiot
  
  string theParam = "";
  string theShapeFileName = "";
  string theContourInterpolation = "Linear";
  string theSmoother = "None";
  float theSmootherRadius = 1.0;
  int theTimeStepRoundingFlag = 1;
  int theTimeStampFlag	= 1;
  string theTimeStampZone = "local";
  int theSmootherFactor = 1;
  int theTimeStepSkip	= 0;	// skipattava minuuttimäärä
  int theTimeStep	= 0;		// aika-askel
  int theTimeInterval   = 0;	// inklusiivinen aikamäärä
  int theTimeSteps	= 24;		// max kuvien lukumäärä
  
  int theTimeStampImageX = 0;
  int theTimeStampImageY = 0;
  string theTimeStampImage = "none";
  
  // Projection määritelmä

  string theProjection;

  string theSavePath	= ".";
  string thePrefix	= "";
  string theSuffix	= "";
  string theFormat	= "png";
  bool   theSaveAlphaFlag = true;
  bool   theWantPaletteFlag = false;
  bool   theForcePaletteFlag = false;
  string theLegendErase = "white";
  string theErase	= "#7F000000";
  string theBackground	= "";
  string theForeground	= "";
  string theMask = "";
  string theFillRule	= "Atop";
  string theStrokeRule	= "Atop";
  
  string theForegroundRule = "Over";

  string theCombine = "";
  int theCombineX;
  int theCombineY;
  string theCombineRule = "Over";
  float theCombineFactor = 1.0;
  
  string theFilter = "none";

  string theDirectionParameter = "WindDirection";
  string theSpeedParameter = "WindSpeedMS";
  
  float theArrowScale = 1.0;

  float theWindArrowScaleA = 0.0;	// a*log10(b*x+1)+c = 0*log10(0+1)+1 = 1
  float theWindArrowScaleB = 0.0;
  float theWindArrowScaleC = 1.0;

  string theArrowFillColor = "white";
  string theArrowStrokeColor = "black";
  string theArrowFillRule = "Over";
  string theArrowStrokeRule = "Over";
  string theArrowFile = "";
  
  unsigned int theWindArrowDX = 0;
  unsigned int theWindArrowDY = 0;
  
  int theContourDepth	= 0;
  int theContourTrianglesOn = 1;
  
  int thePngQuality = -1;
  int theJpegQuality = -1;
  int theAlphaLimit = -1;
  float theGamma = -1.0;
  string theIntent = "";
  
  // Related variables
  
  NFmiImage theBackgroundImage;
  NFmiImage theForegroundImage;
  NFmiImage theMaskImage;
  NFmiImage theCombineImage;
  
  // This holds a vector of querydatastreams
  
  vector<LazyQueryData *> theQueryStreams;
  int theQueryDataLevel = 1;
  string theQueryStreamNames = "";
  vector<string> theFullQueryFileNames;
  
  // These will hold the querydata for the active parameter
  
  LazyQueryData *theQueryInfo = 0;
  

  // Process all command files
  // ~~~~~~~~~~~~~~~~~~~~~~~~~
  
  list<string>::const_iterator fileiter = globals.cmdline_files.begin();
  for( ; fileiter!=globals.cmdline_files.end(); ++fileiter)
    {
      const string & cmdfilename = *fileiter;
	  
      if(globals.verbose)
		cout << "Processing file: " << cmdfilename << endl;
	  
      // Open command file for reading

	  const bool strip_pound = false;
	  NFmiPreProcessor processor(strip_pound);
	  processor.SetDefine("#define");
	  processor.SetIncluding("include", "", "");
	  if(!processor.ReadAndStripFile(cmdfilename))
		throw runtime_error("Could not parse "+cmdfilename);

	  // Extract the assignments
	  string text = processor.GetString();

	  // Insert querydata command if option -q was used

	  if(!globals.cmdline_querydata.empty())
		{
		  if(globals.verbose)
			cout << "Using querydata " << globals.cmdline_querydata << endl;
		  text = "querydata "+globals.cmdline_querydata + '\n' + text;
		}

	  istringstream input(text);
	  
      // Process the commands
      string command;
      while( input >> command)
		{
		  // Handle comments
		  
		  if(command == "#" || command == "//" || command[0]=='#')
			{
			  // Should use numeric_limits<int>::max() to by definition
			  // skip to end of line, but numeric_limits does not exist
			  // in g++ v2.95
			  
			  input.ignore(1000000,'\n');
			}
		  
		  else if(command == "cache")
			{
			  int flag;
			  input >> flag;
			  theCalculator.cache(flag);
			}

		  else if(command == "querydata")
			{
			  string newnames;
			  input >> newnames;
			  
			  if(theQueryStreamNames != newnames)
				{
				  theQueryStreamNames = newnames;
				  
				  // Delete possible old infos
				  
				  for(unsigned int i=0; i<theQueryStreams.size(); i++)
					delete theQueryStreams[i];
				  theQueryStreams.resize(0);
				  theQueryInfo = 0;
				  
				  // Split the comma separated list into a real list
				  
				  list<string> qnames;
				  unsigned int pos1 = 0;
				  while(pos1<theQueryStreamNames.size())
					{
					  unsigned int pos2 = theQueryStreamNames.find(',',pos1);
					  if(pos2==std::string::npos)
						pos2 = theQueryStreamNames.size();
					  qnames.push_back(theQueryStreamNames.substr(pos1,pos2-pos1));
					  pos1 = pos2 + 1;
					}
			  
				  // Read the queryfiles
				  
					{
					  list<string>::const_iterator iter;
					  for(iter=qnames.begin(); iter!=qnames.end(); ++iter)
						{
						  LazyQueryData * tmp = new LazyQueryData();
						  string filename = NFmiFileSystem::FileComplete(*iter,globals.datapath);
						  theFullQueryFileNames.push_back(filename);
						  tmp->Read(filename);
						  theQueryStreams.push_back(tmp);
						}
					}
				}
			}

		  else if(command == "querydatalevel")
			input >> theQueryDataLevel;
		  
		  else if(command == "filter")
			input >> theFilter;
		  
		  else if(command == "timestepskip")
			input >> theTimeStepSkip;
		  
		  else if(command == "timestep")
			{
			  input >> theTimeStep;
			  theTimeInterval = theTimeStep;
			}
		  
		  else if(command == "timeinterval")
			input >> theTimeInterval;
		  
		  else if(command == "timesteps")
			input >> theTimeSteps;
		  
		  else if(command == "timestamp")
			input >> theTimeStampFlag;
		  
		  else if(command == "timestampzone")
			input >> theTimeStampZone;

		  else if(command == "timesteprounding")
			input >> theTimeStepRoundingFlag;
		  
		  else if(command == "timestampimage")
			input >> theTimeStampImage;
		  
		  else if(command == "timestampimagexy")
			input >> theTimeStampImageX >> theTimeStampImageY;
		  
		  else if(command == "projection")
			{
			  input >> theProjection;
			}
		  
		  else if(command == "erase")
			{
			  input >> theErase;
			  ColorTools::checkcolor(theErase);
			}
		  
		  else if(command == "legenderase")
			{
			  input >> theLegendErase;
			  ColorTools::checkcolor(theLegendErase);
			}
		  
		  else if(command == "fillrule")
			{
			  input >> theFillRule;
			  ColorTools::checkrule(theFillRule);
			  if(!theShapeSpecs.empty())
				theShapeSpecs.back().fillrule(theFillRule);
			}
		  else if(command == "strokerule")
			{
			  input >> theStrokeRule;
			  ColorTools::checkrule(theStrokeRule);
			  if(!theShapeSpecs.empty())
				theShapeSpecs.back().strokerule(theStrokeRule);
			}
		  
		  else if(command == "directionparam")
		    input >> theDirectionParameter;

		  else if(command == "speedparam")
		    input >> theSpeedParameter;

		  else if(command == "arrowscale")
			input >> theArrowScale;

		  else if(command == "windarrowscale")
			input >> theWindArrowScaleA >> theWindArrowScaleB >> theWindArrowScaleC;
		  
		  else if(command == "arrowfill")
			{
			  input >> theArrowFillColor >> theArrowFillRule;
			  ColorTools::checkcolor(theArrowFillColor);
			  ColorTools::checkrule(theArrowFillRule);
			}
		  else if(command == "arrowstroke")
			{
			  input >> theArrowStrokeColor >> theArrowStrokeRule;
			  ColorTools::checkcolor(theArrowStrokeColor);
			  ColorTools::checkrule(theArrowStrokeRule);
			}
		  
		  else if(command == "arrowpath")
			input >> theArrowFile;
		  
		  else if(command == "windarrow")
			{
			  float lon,lat;
			  input >> lon >> lat;
			  theArrowPoints.push_back(NFmiPoint(lon,lat));
			}
		  
		  else if(command == "windarrows")
			input >> theWindArrowDX >> theWindArrowDY;
		  
		  else if(command == "background")
			{
			  input >> theBackground;
			  if(theBackground != "none")
				theBackgroundImage.Read(NFmiFileSystem::FileComplete(theBackground,globals.mapspath));
			  else
				theBackground = "";
			}
		  
		  else if(command == "foreground")
			{
			  input >> theForeground;
			  if(theForeground != "none")
				theForegroundImage.Read(NFmiFileSystem::FileComplete(theForeground,globals.mapspath));
			  else
				theForeground = "";
			}
		  
		  else if(command == "mask")
			{
			  input >> theMask;
			  if(theMask != "none")
				theMaskImage.Read(NFmiFileSystem::FileComplete(theMask,globals.mapspath));
			  else
				theMask = "";
			}
		  else if(command == "combine")
			{
			  input >> theCombine;
			  if(theCombine != "none")
				{
				  input >> theCombineX >> theCombineY;
				  input >> theCombineRule >> theCombineFactor;
				  ColorTools::checkrule(theCombineRule);
				  theCombineImage.Read(NFmiFileSystem::FileComplete(theCombine,globals.mapspath));
				}
			  else
				theCombine = "";
			}

		  else if(command == "foregroundrule")
			{
			  input >> theForegroundRule;
			  ColorTools::checkrule(theForegroundRule);
			}
		  
		  else if(command == "savepath")
			{
			  input >> theSavePath;
			  if(!NFmiFileSystem::DirectoryExists(theSavePath))
				throw runtime_error("savepath "+theSavePath+" does not exist");
			}
		  
		  else if(command == "prefix")
			input >> thePrefix;
		  
		  else if(command == "suffix")
			input >> theSuffix;
		  
		  else if(command == "format")
			input >> theFormat;
		  
		  else if(command == "gamma")
			input >> theGamma;
		  
		  else if(command == "intent")
			input >> theIntent;
		  
		  else if(command == "pngquality")
			input >> thePngQuality;
		  
		  else if(command == "jpegquality")
			input >> theJpegQuality;
		  
		  else if(command == "savealpha")
			input >> theSaveAlphaFlag;
		  
		  else if(command == "wantpalette")
			input >> theWantPaletteFlag;
		  
		  else if(command == "forcepalette")
			input >> theForcePaletteFlag;
		  
		  else if(command == "alphalimit")
			input >> theAlphaLimit;
		  
		  else if(command == "hilimit")
			{
			  float limit;
			  input >> limit;
			  if(!theSpecs.empty())
				theSpecs.back().exactHiLimit(limit);
			}
		  else if(command == "datalolimit")
			{
			  float limit;
			  input >> limit;
			  if(!theSpecs.empty())
				theSpecs.back().dataLoLimit(limit);
			}
		  else if(command == "datahilimit")
			{
			  float limit;
			  input >> limit;
			  if(!theSpecs.empty());
			  theSpecs.back().dataHiLimit(limit);
			}
		  else if(command == "datareplace")
			{
			  float src,dst;
			  input >> src >> dst;
			  if(!theSpecs.empty())
				theSpecs.back().replace(src,dst);
			}
		  else if(command == "contourdepth")
			{
			  input >> theContourDepth;
			  if(!theSpecs.empty())
				theSpecs.back().contourDepth(theContourDepth);
			}
		  
		  else if(command == "contourinterpolation")
			{
			  input >> theContourInterpolation;
			  if(!theSpecs.empty())
				theSpecs.back().contourInterpolation(theContourInterpolation);
			}
		  else if(command == "contourtriangles")
			{
			  input >> theContourTrianglesOn;
			}

		  else if(command == "smoother")
			{
			  input >> theSmoother;
			  if(!theSpecs.empty())
				theSpecs.back().smoother(theSmoother);
			}
		  else if(command == "smootherradius")
			{
			  input >> theSmootherRadius;
			  if(!theSpecs.empty())
				theSpecs.back().smootherRadius(theSmootherRadius);
			}
		  else if(command == "smootherfactor")
			{
			  input >> theSmootherFactor;
			  if(!theSpecs.empty())
				theSpecs.back().smootherFactor(theSmootherFactor);
			}
		  else if(command == "param")
			{
			  input >> theParam;
			  theSpecs.push_back(ContourSpec(theParam,
											 theContourInterpolation,
											 theSmoother,
											 theContourDepth,
											 theSmootherRadius,
											 theSmootherFactor));
			}
		  
		  else if(command == "shape")
			{
			  input >> theShapeFileName;
			  string arg1;
			  
			  input >> arg1;
			  
			  if(arg1=="mark")
				{
				  string marker, markerrule;
				  float markeralpha;
				  input >> marker >> markerrule >> markeralpha;
				  
				  ColorTools::checkrule(markerrule);
				  ShapeSpec spec(theShapeFileName);
				  spec.marker(marker,markerrule,markeralpha);
				  theShapeSpecs.push_back(spec);
				}
			  else
				{
				  string fillcolor = arg1;
				  string strokecolor;
				  input >> strokecolor;
				  NFmiColorTools::Color fill = ColorTools::checkcolor(fillcolor);
				  NFmiColorTools::Color stroke = ColorTools::checkcolor(strokecolor);

				  theShapeSpecs.push_back(ShapeSpec(theShapeFileName,
													fill,stroke,
													theFillRule,theStrokeRule));
				}
			}
		  
		  else if(command == "contourfill")
			{
			  string slo,shi,scolor;
			  input >> slo >> shi >> scolor;
			  
			  float lo,hi;
			  if(slo == "-")
				lo = kFloatMissing;
			  else
				lo = atof(slo.c_str());
			  if(shi == "-")
				hi = kFloatMissing;
			  else
				hi = atof(shi.c_str());
			  
			  NFmiColorTools::Color color = ColorTools::checkcolor(scolor);
			  
			  if(!theSpecs.empty())
				theSpecs.back().add(ContourRange(lo,hi,color,theFillRule));
			}
		  
		  else if(command == "contourpattern")
			{
			  string slo,shi,spattern,srule;
			  float alpha;
			  input >> slo >> shi >> spattern >> srule >> alpha;
			  
			  float lo,hi;
			  if(slo == "-")
				lo = kFloatMissing;
			  else
				lo = atof(slo.c_str());
			  if(shi == "-")
				hi = kFloatMissing;
			  else
				hi = atof(shi.c_str());
			  
			  if(!theSpecs.empty())
				theSpecs.back().add(ContourPattern(lo,hi,spattern,srule,alpha));
			}
		  
		  else if(command == "contourline")
			{
			  string svalue,scolor;
			  input >> svalue >> scolor;
			  
			  float value;
			  if(svalue == "-")
				value = kFloatMissing;
			  else
				value = atof(svalue.c_str());
			  
			  NFmiColorTools::Color color = ColorTools::checkcolor(scolor);
			  if(!theSpecs.empty())
				theSpecs.back().add(ContourValue(value,color,theStrokeRule));
			}
		  
		  else if(command == "contourfills")
			{
			  float lo,hi,step;
			  string scolor1,scolor2;
			  input >> lo >> hi >> step >> scolor1 >> scolor2;
			  
			  int color1 = ColorTools::checkcolor(scolor1);
			  int color2 = ColorTools::checkcolor(scolor2);
			  
			  int steps = static_cast<int>((hi-lo)/step);
			  
			  for(int i=0; i<steps; i++)
				{
				  float tmplo=lo+i*step;
				  float tmphi=lo+(i+1)*step;
				  int color = color1;	// in case steps=1
				  if(steps!=1)
					color = NFmiColorTools::Interpolate(color1,color2,i/(steps-1.0));
				  if(!theSpecs.empty())
					theSpecs.back().add(ContourRange(tmplo,tmphi,color,theFillRule));
				}
			}
	      
		  else if(command == "contourlines")
			{
			  float lo,hi,step;
			  string scolor1,scolor2;
			  input >> lo >> hi >> step >> scolor1 >> scolor2;
			  
			  int color1 = ColorTools::checkcolor(scolor1);
			  int color2 = ColorTools::checkcolor(scolor2);
			  
			  int steps = static_cast<int>((hi-lo)/step);
			  
			  for(int i=0; i<=steps; i++)
				{
				  float tmplo=lo+i*step;
				  int color = color1;	// in case steps=1
				  if(steps!=0)
					color = NFmiColorTools::Interpolate(color1,color2,i/static_cast<float>(steps));
				  if(!theSpecs.empty())
					theSpecs.back().add(ContourValue(tmplo,color,theStrokeRule));
				}
			}
	      
		  else if(command == "clear")
			{
			  input >> command;
			  if(command=="contours")
				theSpecs.clear();
			  else if(command=="shapes")
				theShapeSpecs.clear();
			  else if(command=="cache")
				theCalculator.clearCache();
			  else if(command=="arrows")
				{
				  theArrowPoints.clear();
				  theWindArrowDX = 0;
				  theWindArrowDY = 0;
				}
			  else if(command=="labels")
				{
				  list<ContourSpec>::iterator piter;
				  for(piter=theSpecs.begin(); piter!=theSpecs.end(); ++piter)
					piter->clearLabels();
				}
			  else
				throw runtime_error("Unknown clear target: " + command);
			}
		  
		  else if(command == "labelmarker")
			{
			  string filename, rule;
			  float alpha;
			  
			  input >> filename >> rule >> alpha;
			  
			  if(!theSpecs.empty())
				{
				  theSpecs.back().labelMarker(filename);
				  theSpecs.back().labelMarkerRule(rule);
				  theSpecs.back().labelMarkerAlphaFactor(alpha);
				}
			}
		  
		  else if(command == "labelfont")
			{
			  string font;
			  input >> font;
			  if(!theSpecs.empty())
				theSpecs.back().labelFont(font);
			}
		  
		  else if(command == "labelsize")
			{
			  float size;
			  input >> size;
			  if(!theSpecs.empty())
				theSpecs.back().labelSize(size);
			}
		  
		  else if(command == "labelstroke")
			{
			  string color,rule;
			  input >> color >> rule;
			  if(!theSpecs.empty())
				{
				  theSpecs.back().labelStrokeColor(ColorTools::checkcolor(color));
				  theSpecs.back().labelStrokeRule(rule);
				}
			}
		  
		  else if(command == "labelfill")
			{
			  string color,rule;
			  input >> color >> rule;
			  if(!theSpecs.empty())
				{
				  theSpecs.back().labelFillColor(ColorTools::checkcolor(color));
				  theSpecs.back().labelFillRule(rule);
				}
			}
		  
		  else if(command == "labelalign")
			{
			  string align;
			  input >> align;
			  if(!theSpecs.empty())
				theSpecs.back().labelAlignment(align);
			}
		  
		  else if(command == "labelformat")
			{
			  string format;
			  input >> format;
			  if(format == "-") format = "";
			  if(!theSpecs.empty())
				theSpecs.back().labelFormat(format);
			}
		  
		  else if(command == "labelmissing")
			{
			  string label;
			  input >> label;
			  if(label == "none") label = "";
			  if(!theSpecs.empty())
				theSpecs.back().labelMissing(label);
			}
		  
		  else if(command == "labelangle")
			{
			  float angle;
			  input >> angle;
			  if(!theSpecs.empty())
				theSpecs.back().labelAngle(angle);
			}
		  
		  else if(command == "labeloffset")
			{
			  float dx,dy;
			  input >> dx >> dy;
			  if(!theSpecs.empty())
				{
				  theSpecs.back().labelOffsetX(dx);
				  theSpecs.back().labelOffsetY(dy);
				}
			}
		  
		  else if(command == "labelcaption")
			{
			  string name,align;
			  float dx,dy;
			  input >> name >> dx >> dy >> align;
			  if(!theSpecs.empty())
				{
				  theSpecs.back().labelCaption(name);
				  theSpecs.back().labelCaptionDX(dx);
				  theSpecs.back().labelCaptionDY(dy);
				  theSpecs.back().labelCaptionAlignment(align);
				}
			}
		  
		  else if(command == "label")
			{
			  float lon,lat;
			  input >> lon >> lat;
			  if(!theSpecs.empty())
				theSpecs.back().add(NFmiPoint(lon,lat));
			}
		  
		  else if(command == "labelxy")
			{
			  float lon,lat;
			  input >> lon >> lat;
			  int dx, dy;
			  input >> dx >> dy;
			  if(!theSpecs.empty())
				theSpecs.back().add(NFmiPoint(lon,lat),NFmiPoint(dx,dy));
			}
		  
		  else if(command == "labels")
			{
			  int dx,dy;
			  input >> dx >> dy;
			  if(!theSpecs.empty())
				{
				  theSpecs.back().labelDX(dx);
				  theSpecs.back().labelDY(dy);
				}

			}
		  
		  else if(command == "labelfile")
			{
			  string datafilename;
			  input >> datafilename;
			  ifstream datafile(datafilename.c_str());
			  if(!datafile)
				throw runtime_error("No data file named " + datafilename);
			  string datacommand;
			  while( datafile >> datacommand)
				{
				  if(datacommand == "#" || datacommand == "//")
					datafile.ignore(1000000,'\n');
				  else if(datacommand == "label")
					{
					  float lon,lat;
					  datafile >> lon >> lat;
					  if(!theSpecs.empty())
						theSpecs.back().add(NFmiPoint(lon,lat));
					}
				  else
					throw runtime_error("Unknown datacommand " + datacommand);
				}
			  datafile.close();
			}
		  
		  else if(command == "draw")
			{
			  // Draw what?
			  
			  input >> command;
			  
			  // --------------------------------------------------
			  // Draw legend
			  // --------------------------------------------------
			  
			  if(command == "legend")
				{
				  string legendname;
				  int width, height;
				  float lolimit, hilimit;
				  input >> legendname >> lolimit >> hilimit >> width >> height;
				  
				  if(!theSpecs.empty())
					{
					  NFmiImage legend(width,height);
					  
					  NFmiColorTools::Color color = ColorTools::checkcolor(theLegendErase);
					  legend.Erase(color);
					  
					  list<ContourRange>::const_iterator citer;
					  list<ContourRange>::const_iterator cbegin;
					  list<ContourRange>::const_iterator cend;
					  
					  cbegin = theSpecs.back().contourFills().begin();
					  cend   = theSpecs.back().contourFills().end();
					  
					  for(citer=cbegin ; citer!=cend; ++citer)
						{
						  float thelo = citer->lolimit();
						  float thehi = citer->hilimit();
						  NFmiColorTools::NFmiBlendRule rule = ColorTools::checkrule(citer->rule());
						  
						  if(thelo==kFloatMissing) thelo=-1e6;
						  if(thehi==kFloatMissing) thehi= 1e6;
						  
						  NFmiPath path;
						  path.MoveTo(0,height*(1-(thelo-lolimit)/(hilimit-lolimit)));
						  path.LineTo(width,height*(1-(thelo-lolimit)/(hilimit-lolimit)));
						  path.LineTo(width,height*(1-(thehi-lolimit)/(hilimit-lolimit)));
						  path.LineTo(0,height*(1-(thehi-lolimit)/(hilimit-lolimit)));
						  path.CloseLineTo();
						  
						  path.Fill(legend,citer->color(),rule);
						}
					  
					  list<ContourValue>::const_iterator liter;
					  list<ContourValue>::const_iterator lbegin;
					  list<ContourValue>::const_iterator lend;
					  
					  lbegin = theSpecs.back().contourValues().begin();
					  lend   = theSpecs.back().contourValues().end();
					  
					  for(liter=lbegin ; liter!=lend; ++liter)
						{
						  float thevalue = liter->value();
						  
						  if(thevalue==kFloatMissing)
							continue;
						  
						  NFmiPath path;
						  path.MoveTo(0,height*(1-(thevalue-lolimit)/(hilimit-lolimit)));
						  path.LineTo(width,height*(1-(thevalue-lolimit)/(hilimit-lolimit)));
						  path.Stroke(legend,liter->color());
						}
					  
					  legend.WritePng(legendname+".png");
					}
				  
				}
			  
			  // --------------------------------------------------
			  // Render shapes
			  // --------------------------------------------------
			  
			  else if(command == "shapes")
				{
				  // The output filename
				  
				  string filename;
				  input >> filename;

				  auto_ptr<NFmiArea> theArea;

				  if(theProjection.empty())
					throw runtime_error("No projection has been specified for rendering shapes");
				  else
					theArea.reset(NFmiAreaFactory::Create(theProjection).release());
				  
				  
				  if(globals.verbose)
					cout << "Area corners are"
						 << endl
						 << "bottomleft\t= " 
						 << theArea->BottomLeftLatLon().X()
						 << ','
						 << theArea->BottomLeftLatLon().Y()
						 << endl
						 << "topright\t= "
						 << theArea->TopRightLatLon().X()
						 << ','
						 << theArea->TopRightLatLon().Y()
						 << endl;

				  int imgwidth = static_cast<int>(theArea->Width()+0.5);
				  int imgheight = static_cast<int>(theArea->Height()+0.5);
				  
				  // Initialize the background
				  
				  NFmiImage theImage(imgwidth, imgheight);
				  theImage.SaveAlpha(theSaveAlphaFlag);
				  theImage.WantPalette(theWantPaletteFlag);
				  theImage.ForcePalette(theForcePaletteFlag);
				  if(theGamma>0) theImage.Gamma(theGamma);
				  if(!theIntent.empty()) theImage.Intent(theIntent);
				  if(thePngQuality>=0) theImage.PngQuality(thePngQuality);
				  if(theJpegQuality>=0) theImage.JpegQuality(theJpegQuality);
				  if(theAlphaLimit>=0) theImage.AlphaLimit(theAlphaLimit);
				  
				  NFmiColorTools::Color erasecolor = ColorTools::checkcolor(theErase);
				  theImage.Erase(erasecolor);
				  
				  // Draw all the shapes
				  
				  list<ShapeSpec>::const_iterator iter;
				  list<ShapeSpec>::const_iterator begin = theShapeSpecs.begin();
				  list<ShapeSpec>::const_iterator end   = theShapeSpecs.end();
				  
				  for(iter=begin; iter!=end; ++iter)
					{
					  NFmiGeoShape geo(iter->filename(),kFmiGeoShapeEsri);
					  geo.ProjectXY(*theArea);
					  
					  if(iter->marker()=="")
						{
						  NFmiColorTools::NFmiBlendRule fillrule = ColorTools::checkrule(iter->fillrule());
						  NFmiColorTools::NFmiBlendRule strokerule = ColorTools::checkrule(iter->strokerule());
						  geo.Fill(theImage,iter->fillcolor(),fillrule);
						  geo.Stroke(theImage,iter->strokecolor(),strokerule);
						}
					  else
						{
						  NFmiColorTools::NFmiBlendRule markerrule = ColorTools::checkrule(iter->markerrule());
						  
						  NFmiImage marker;
						  marker.Read(iter->marker());
						  geo.Mark(theImage,marker,markerrule,
								   kFmiAlignCenter,
								   iter->markeralpha());
						}
					}
				  
				  string outfile = filename + "." + theFormat;
				  if(globals.verbose)
					cout << "Writing " << outfile << endl;
				  if(theFormat=="png")
					theImage.WritePng(outfile);
				  else if(theFormat=="jpg" || theFormat=="jpeg")
					theImage.WriteJpeg(outfile);
				  else if(theFormat=="gif")
					theImage.WriteGif(outfile);
				  else
					throw runtime_error("Image format " + theFormat + " is not supported");
				}
			  
			  // --------------------------------------------------
			  // Generate imagemap data
			  // --------------------------------------------------
			  
			  else if(command == "imagemap")
				{
				  // The relevant field name and filenames
				  
				  string fieldname, filename;
				  input >> fieldname >> filename;
				  

				  auto_ptr<NFmiArea> theArea;

				  if(theProjection.empty())
					throw runtime_error("No projection has been specified for rendering shapes");
				  else
					theArea.reset(NFmiAreaFactory::Create(theProjection).release());
				  
				  // Generate map from all shapes in the list
				  
				  list<ShapeSpec>::const_iterator iter;
				  list<ShapeSpec>::const_iterator begin = theShapeSpecs.begin();
				  list<ShapeSpec>::const_iterator end   = theShapeSpecs.end();
				  
				  string outfile = filename + ".map";
				  ofstream out(outfile.c_str());
				  if(!out)
					throw runtime_error("Failed to open "+outfile+" for writing");
				  if(globals.verbose)
					cout << "Writing " << outfile << endl;
				  
				  for(iter=begin; iter!=end; ++iter)
					{
					  NFmiGeoShape geo(iter->filename(),kFmiGeoShapeEsri);
					  geo.ProjectXY(*theArea);
					  geo.WriteImageMap(out,fieldname);
					}
				  out.close();
				}
			  
			  // --------------------------------------------------
			  // Draw contours
			  // --------------------------------------------------
			  
			  else if(command == "contours")
				{
				  // 1. Make sure query data has been read
				  // 2. Make sure image has been initialized
				  // 3. Loop over all times
				  //   4. If the time is acceptable,
				  //   5. Loop over all parameters
				  //     6. Fill all specified intervals
				  //     7. Patternfill all specified intervals
				  //     8. Stroke all specified contours
				  //   9. Overwrite with foreground if so desired
				  //   10. Loop over all parameters
				  //     11. Label all specified points
				  //   12. Draw arrows if requested
				  //   13. Save the image
				  
				  if(theQueryStreams.empty())
					throw runtime_error("No query data has been read!");
				  
				  auto_ptr<NFmiArea> theArea;

				  if(theProjection.empty())
					throw runtime_error("No projection has been specified for rendering shapes");
				  else
					theArea.reset(NFmiAreaFactory::Create(theProjection).release());

				  // This message intentionally ignores globals.verbose
				  
				  if(theBackground != "")
					cout << "Contouring for background " << theBackground << endl;
				  

				  if(globals.verbose)
					cout << "Area corners are"
						 << endl
						 << "bottomleft\t= " 
						 << theArea->BottomLeftLatLon().X()
						 << ','
						 << theArea->BottomLeftLatLon().Y()
						 << endl
						 << "topright\t= "
						 << theArea->TopRightLatLon().X()
						 << ','
						 << theArea->TopRightLatLon().Y()
						 << endl;

				  // Establish querydata timelimits and initialize
				  // the XY-coordinates simultaneously.
				  
				  // Note that we use world-coordinates when smoothing
				  // so that we can use meters as the smoothing radius.
				  // Also, this means the contours are independent of
				  // the image size.
				  
				  NFmiTime utctime, time1, time2;
				  
				  NFmiDataMatrix<float> vals;
				  
				  unsigned int qi;
				  for(qi=0; qi<theQueryStreams.size(); qi++)
					{
					  // Initialize the queryinfo
					  
					  theQueryInfo = theQueryStreams[qi];
					  theQueryInfo->FirstLevel();
					  if(theQueryDataLevel>0)
						{
						  int level = theQueryDataLevel;
						  while(--level > 0)
							theQueryInfo->NextLevel();
						}
					  
					  // Establish time limits
					  theQueryInfo->LastTime();
					  utctime = theQueryInfo->ValidTime();
					  NFmiTime t2 = TimeTools::ConvertZone(utctime,theTimeStampZone);
					  theQueryInfo->FirstTime();
					  utctime = theQueryInfo->ValidTime();
					  NFmiTime t1 = TimeTools::ConvertZone(utctime,theTimeStampZone);
					  
					  if(qi==0)
						{
						  time1 = t1;
						  time2 = t2;
						}
					  else
						{
						  if(time1.IsLessThan(t1))
							time1 = t1;
						  if(!time2.IsLessThan(t2))
							time2 = t2;
						}
					  
					}
				  
				  if(globals.verbose)
					{
					  cout << "Data start time " << time1 << endl
						   << "Data end time " << time2 << endl;
					}
				  
				  // Skip to first time
				  
				  NFmiMetTime tmptime(time1,
									  theTimeStepRoundingFlag ?
									  (theTimeStep>0 ? theTimeStep : 1) :
									  1);
				  
				  tmptime.ChangeByMinutes(theTimeStepSkip);
				  if(theTimeStepRoundingFlag)
					tmptime.PreviousMetTime();
				  NFmiTime t = tmptime;
				  
				  // Loop over all times
				  
				  int imagesdone = 0;
				  bool labeldxdydone = false;
				  for(;;)
					{
					  if(imagesdone>=theTimeSteps)
						break;
					  
					  // Skip to next time to be drawn
					  
					  t.ChangeByMinutes(theTimeStep > 0 ? theTimeStep : 1);
					  
					  // If the time is after time2, we're done
					  
					  if(time2.IsLessThan(t))
						break;
					  
					  // Search first time >= the desired time
					  // This is quaranteed to succeed since we've
					  // already tested against time2, the last available
					  // time.
					  
					  bool ok = true;
					  for(qi=0; ok && qi<theQueryStreams.size(); qi++)
						{
						  theQueryInfo = theQueryStreams[qi];
						  theQueryInfo->ResetTime();
						  while(theQueryInfo->NextTime())
							{
							  NFmiTime utc = theQueryInfo->ValidTime();
							  NFmiTime loc = TimeTools::ConvertZone(utc,theTimeStampZone);
							  if(!loc.IsLessThan(t))
								break;
							}
						  NFmiTime utc = theQueryInfo->ValidTime();
						  NFmiTime tnow = TimeTools::ConvertZone(utc,theTimeStampZone);
						  
						  // we wanted
						  
						  if(theTimeStep==0)
							t = tnow;
						  
						  // If time is before time1, ignore it
						  
						  if(t.IsLessThan(time1))
							{
							  ok = false;
							  break;
							}
						  
						  // Is the time exact?
						  
						  bool isexact = t.IsEqual(tnow);
						  
						  // The previous acceptable time step in calculations
						  // Use NFmiTime, not NFmiMetTime to avoid rounding up!
						  
						  NFmiTime tprev = t;
						  tprev.ChangeByMinutes(-theTimeInterval);
						  
						  bool hasprevious = !tprev.IsLessThan(time1);
						  
						  // Skip this image if we are unable to render it
						  
						  if(theFilter=="none")
							{
							  // Cannot draw time with filter none
							  // if time is not exact.
							  
							  ok = isexact;
							  
							}
						  else if(theFilter=="linear")
							{
							  // OK if is exact, otherwise previous step required
							  
							  ok = !(!isexact && !hasprevious);
							}
						  else
							{
							  // Time must be exact, and previous steps
							  // are required
							  
							  ok = !(!isexact || !hasprevious);
							}
						}
					  
					  if(!ok)
						continue;
					  
					  // The image is accepted for rendering, but
					  // we might not overwrite an existing one.
					  // Hence we update the counter here already.
					  
					  imagesdone++;
					  
					  // Create the filename
					  
					  // The timestamp as a string
					  
					  NFmiString datatimestr = t.ToStr(kYYYYMMDDHHMM);
					  
					  if(globals.verbose)
						cout << "Time is " << datatimestr.CharPtr() << endl;

					  string filename =
						theSavePath
						+ "/"
						+ thePrefix
						+ datatimestr.CharPtr();
					  
					  if(theTimeStampFlag)
						{
						  for(qi=0; qi<theFullQueryFileNames.size(); qi++)
							{
							  time_t secs = NFmiFileSystem::FileModificationTime(theFullQueryFileNames[qi]);
							  NFmiTime tlocal(secs);
							  filename += "_" + tlocal.ToStr(kDDHHMM);
							}
						}
					  
					  filename +=
						theSuffix
						+ "."
						+ theFormat;
					  
					  // In force-mode we always write, but otherwise
					  // we first check if the output image already
					  // exists. If so, we assume it is up to date
					  // and skip to the next time stamp.
					  
					  if(!globals.force && !NFmiFileSystem::FileEmpty(filename))
						{
						  if(globals.verbose)
							cout << "Not overwriting " << filename << endl;
						  continue;
						}

					  // Initialize the background

					  int imgwidth = static_cast<int>(theArea->Width()+0.5);
					  int imgheight = static_cast<int>(theArea->Height()+0.5);
					  
					  NFmiImage theImage(imgwidth,imgheight);
					  theImage.SaveAlpha(theSaveAlphaFlag);
					  theImage.WantPalette(theWantPaletteFlag);
					  theImage.ForcePalette(theForcePaletteFlag);
					  if(theGamma>0) theImage.Gamma(theGamma);
					  if(!theIntent.empty()) theImage.Intent(theIntent);
					  if(thePngQuality>=0) theImage.PngQuality(thePngQuality);
					  if(theJpegQuality>=0) theImage.JpegQuality(theJpegQuality);
					  if(theAlphaLimit>=0) theImage.AlphaLimit(theAlphaLimit);
					  
					  NFmiColorTools::Color erasecolor = ColorTools::checkcolor(theErase);
					  theImage.Erase(erasecolor);
					  
					  if(theBackground != "")
						theImage = theBackgroundImage;
					  
					  // Loop over all parameters
					  
					  list<ContourSpec>::iterator piter;
					  list<ContourSpec>::iterator pbegin = theSpecs.begin();
					  list<ContourSpec>::iterator pend   = theSpecs.end();
					  
					  for(piter=pbegin; piter!=pend; ++piter)
						{
						  // Establish the parameter
						  
						  string name = piter->param();

						  bool ismeta = false;
						  ok = false;
						  FmiParameterName param = FmiParameterName(NFmiEnumConverter().ToEnum(name));
						  
						  if(param==kFmiBadParameter)
							{
							  if(!MetaFunctions::isMeta(name))
								throw runtime_error("Unknown parameter "+name);
							  ismeta = true;
							  ok = true;
							  // We always assume the first querydata is ok
							  qi = 0;
							  theQueryInfo = theQueryStreams[0];
							}
						  else
							{
							  // Find the proper queryinfo to be used
							  // Note that qi will be used later on for
							  // getting the coordinate matrices
							  
							  for(qi=0; qi<theQueryStreams.size(); qi++)
								{
								  theQueryInfo = theQueryStreams[qi];
								  theQueryInfo->Param(param);
								  ok = theQueryInfo->IsParamUsable();
								  if(ok) break;
								}
							}
						  
						  if(!ok)
							throw runtime_error("The parameter is not usable: " + name);
						  
						  if(globals.verbose)
							{
							  cout << "Param " << name << " from queryfile number "
								   << (qi+1) << endl;
							}
						  
						  // Establish the contour method
						  
						  string interpname = piter->contourInterpolation();
						  NFmiContourTree::NFmiContourInterpolation interp
							= NFmiContourTree::ContourInterpolationValue(interpname);
						  if(interp==NFmiContourTree::kFmiContourMissingInterpolation)
							throw runtime_error("Unknown contour interpolation method " + interpname);
						  
						  // Get the values. 
						  
						  if(!ismeta)
							theQueryInfo->Values(vals);
						  else
							vals = MetaFunctions::values(piter->param(),
														 theQueryInfo);
						  
						  // Replace values if so requested
						  
						  if(piter->replace())
							vals.Replace(piter->replaceSourceValue(),piter->replaceTargetValue());
						  
						  if(theFilter=="none")
							{
							  // The time is known to be exact
							}
						  else if(theFilter=="linear")
							{
							  NFmiTime utc = theQueryInfo->ValidTime();
							  NFmiTime tnow = TimeTools::ConvertZone(utc,theTimeStampZone);
							  bool isexact = t.IsEqual(tnow);
							  
							  if(!isexact)
								{
								  NFmiDataMatrix<float> tmpvals;
								  NFmiTime t2utc = theQueryInfo->ValidTime();
								  NFmiTime t2 = TimeTools::ConvertZone(t2utc,theTimeStampZone);
								  theQueryInfo->PreviousTime();
								  NFmiTime t1utc = theQueryInfo->ValidTime();
								  NFmiTime t1 = TimeTools::ConvertZone(t1utc,theTimeStampZone);
								  if(!ismeta)
									theQueryInfo->Values(tmpvals);
								  else
									tmpvals = MetaFunctions::values(piter->param(), theQueryInfo);
								  if(piter->replace())
									tmpvals.Replace(piter->replaceSourceValue(),
													piter->replaceTargetValue());
								  
								  // Data from t1,t2, we want t
								  
								  long offset = t.DifferenceInMinutes(t1);
								  long range = t2.DifferenceInMinutes(t1);
								  
								  float weight = (static_cast<float>(offset))/range;
								  
								  vals.LinearCombination(tmpvals,weight,1-weight);
								  
								}
							}
						  else
							{
							  NFmiTime tprev = t;
							  tprev.ChangeByMinutes(-theTimeInterval);
							  
							  NFmiDataMatrix<float> tmpvals;
							  int steps = 1;
							  for(;;)
								{
								  theQueryInfo->PreviousTime();
								  NFmiTime utc = theQueryInfo->ValidTime();
								  NFmiTime tnow = TimeTools::ConvertZone(utc,theTimeStampZone);
								  if(tnow.IsLessThan(tprev))
									break;
								  
								  steps++;
								  if(!ismeta)
									theQueryInfo->Values(tmpvals);
								  else
									tmpvals = MetaFunctions::values(piter->param(), theQueryInfo);
								  if(piter->replace())
									tmpvals.Replace(piter->replaceSourceValue(),
													piter->replaceTargetValue());
								  
								  if(theFilter=="min")
									vals.Min(tmpvals);
								  else if(theFilter=="max")
									vals.Max(tmpvals);
								  else if(theFilter=="mean")
									vals += tmpvals;
								  else if(theFilter=="sum")
									vals += tmpvals;
								}
							  
							  if(theFilter=="mean")
								vals /= steps;
							}
						  
						  
						  // Smoothen the values
						  
						  NFmiSmoother smoother(piter->smoother(),
												piter->smootherFactor(),
												piter->smootherRadius());
						  
						  shared_ptr<NFmiDataMatrix<NFmiPoint> > worldpts = theQueryInfo->LocationsWorldXY(*theArea);
						  vals = smoother.Smoothen(*worldpts,vals);
						  
						  // Find the minimum and maximum
						  
						  float valmin = kFloatMissing;
						  float valmax = kFloatMissing;
						  for(unsigned int j=0; j<vals.NY(); j++)
							for(unsigned int i=0; i<vals.NX(); i++)
							  if(vals[i][j]!=kFloatMissing)
								{
								  if(valmin==kFloatMissing || vals[i][j]<valmin)
									valmin = vals[i][j];
								  if(valmax==kFloatMissing || vals[i][j]>valmax)
									valmax = vals[i][j];
								}
						  
						  if(globals.verbose)
							cout << "Data range for " << name << " is " << valmin << "," << valmax << endl;
						  
						  // Setup the contourer with the values

						  theCalculator.data(vals);

						  // Save the data values at desired points for later
						  // use, this lets us avoid using InterpolatedValue()
						  // which does not use smoothened values.

						  // First, however, if this is the first image, we add
						  // the grid points to the set of points, if so requested

						  if(!labeldxdydone && piter->labelDX() > 0 && piter->labelDY() > 0)
							{
							  for(unsigned int j=0; j<worldpts->NY(); j+=piter->labelDY())
								for(unsigned int i=0; i<worldpts->NX(); i+=piter->labelDX())
								  piter->add(theArea->WorldXYToLatLon((*worldpts)[i][j]));
							}

						  piter->clearLabelValues();
						  if((piter->labelFormat() != "") &&
							 !piter->labelPoints().empty() )
							{
							  list<pair<NFmiPoint,NFmiPoint> >::const_iterator iter;
							  
							  for(iter=piter->labelPoints().begin();
								  iter!=piter->labelPoints().end();
								  ++iter)
								{
								  NFmiPoint latlon = iter->first;
								  NFmiPoint ij = theQueryInfo->LatLonToGrid(latlon);
								  
								  float value;
								  
								  if(fabs(ij.X()-FmiRound(ij.X()))<0.00001 &&
									 fabs(ij.Y()-FmiRound(ij.Y()))<0.00001)
									{
									  value = vals[FmiRound(ij.X())][FmiRound(ij.Y())];
									}
								  else
									{
									  int i = static_cast<int>(ij.X()); // rounds down
									  int j = static_cast<int>(ij.Y());
									  float v00 = vals.At(i,j,kFloatMissing);
									  float v10 = vals.At(i+1,j,kFloatMissing);
									  float v01 = vals.At(i,j+1,kFloatMissing);
									  float v11 = vals.At(i+1,j+1,kFloatMissing);
									  if(!theQueryInfo->BiLinearInterpolation(ij.X(),
																			  ij.Y(),
																			  value,
																			  v00,v10,
																			  v01,v11))
										value = kFloatMissing;

									}
								  piter->addLabelValue(value);
								}
							}
					  
						  // Fill the contours
						  
						  list<ContourRange>::const_iterator citer;
						  list<ContourRange>::const_iterator cbegin;
						  list<ContourRange>::const_iterator cend;
						  
						  cbegin = piter->contourFills().begin();
						  cend   = piter->contourFills().end();
						  
						  for(citer=cbegin ; citer!=cend; ++citer)
							{
							  // Skip to next contour if this one is outside
							  // the value range. As a special case
							  // min=max=missing is ok, if both the limits
							  // are missing too. That is, when we are
							  // contouring missing values.
							  
							  if(valmin==kFloatMissing || valmax==kFloatMissing)
								{
								  if(citer->lolimit()!=kFloatMissing &&
									 citer->hilimit()!=kFloatMissing)
									continue;
								}
							  else
								{
								  if(citer->lolimit()!=kFloatMissing &&
									 valmax<citer->lolimit())
									continue;
								  if(citer->hilimit()!=kFloatMissing &&
									 valmin>citer->hilimit())
									continue;
								}
							  
							  bool exactlo = true;
							  bool exacthi = (citer->hilimit()!=kFloatMissing &&
											  piter->exactHiLimit()!=kFloatMissing &&
											  citer->hilimit()==piter->exactHiLimit());

							  NFmiPath path =
								theCalculator.contour(*theQueryInfo,
													  citer->lolimit(),
													  citer->hilimit(),
													  exactlo,
													  exacthi,
													  piter->dataLoLimit(),
													  piter->dataHiLimit(),
													  piter->contourDepth(),
													  interp,
													  theContourTrianglesOn);
							  
							  if(globals.verbose && theCalculator.wasCached())
								cout << "Using cached "
									 << citer->lolimit() << " - "
									 << citer->hilimit() << endl;

							  NFmiColorTools::NFmiBlendRule rule = ColorTools::checkrule(citer->rule());
							  path.Project(theArea.get());
							  path.Fill(theImage,citer->color(),rule);
							  
							}
						  
						  // Fill the contours with patterns
						  
						  list<ContourPattern>::const_iterator patiter;
						  list<ContourPattern>::const_iterator patbegin;
						  list<ContourPattern>::const_iterator patend;
						  
						  patbegin = piter->contourPatterns().begin();
						  patend   = piter->contourPatterns().end();
						  
						  for(patiter=patbegin ; patiter!=patend; ++patiter)
							{
							  // Skip to next contour if this one is outside
							  // the value range. As a special case
							  // min=max=missing is ok, if both the limits
							  // are missing too. That is, when we are
							  // contouring missing values.
							  
							  if(valmin==kFloatMissing || valmax==kFloatMissing)
								{
								  if(patiter->lolimit()!=kFloatMissing &&
									 patiter->hilimit()!=kFloatMissing)
									continue;
								}
							  else
								{
								  if(patiter->lolimit()!=kFloatMissing &&
									 valmax<patiter->lolimit())
									continue;
								  if(patiter->hilimit()!=kFloatMissing &&
									 valmin>patiter->hilimit())
									continue;
								}
							  
							  bool exactlo = true;
							  bool exacthi = (patiter->hilimit()!=kFloatMissing &&
											  piter->exactHiLimit()!=kFloatMissing &&
											  patiter->hilimit()==piter->exactHiLimit());

							  NFmiPath path =
								theCalculator.contour(*theQueryInfo,
													  patiter->lolimit(),
													  patiter->hilimit(),
													  exactlo, exacthi,
													  piter->dataLoLimit(),
													  piter->dataHiLimit(),
													  piter->contourDepth(),
													  interp,
													  theContourTrianglesOn);

							  if(globals.verbose && theCalculator.wasCached())
								cout << "Using cached "
									 << patiter->lolimit() << " - "
									 << patiter->hilimit() << endl;

							  NFmiColorTools::NFmiBlendRule rule = ColorTools::checkrule(patiter->rule());
							  NFmiImage pattern(patiter->pattern());

							  path.Project(theArea.get());
							  path.Fill(theImage,pattern,rule,patiter->factor());
							  
							}
						  
						  // Stroke the contours
						  
						  list<ContourValue>::const_iterator liter;
						  list<ContourValue>::const_iterator lbegin;
						  list<ContourValue>::const_iterator lend;
						  
						  lbegin = piter->contourValues().begin();
						  lend   = piter->contourValues().end();
						  
						  for(liter=lbegin ; liter!=lend; ++liter)
							{
							  // Skip to next contour if this one is outside
							  // the value range.
							  
							  if(valmin!=kFloatMissing && valmax!=kFloatMissing)
								{
								  if(liter->value()!=kFloatMissing &&
									 valmax<liter->value())
									continue;
								  if(liter->value()!=kFloatMissing &&
									 valmin>liter->value())
									continue;
								}

							  NFmiPath path =
								theCalculator.contour(*theQueryInfo,
													  liter->value(),
													  kFloatMissing,
													  true, false,
													  piter->dataLoLimit(),
													  piter->dataHiLimit(),
													  piter->contourDepth(),
													  interp,
													  theContourTrianglesOn);

							  NFmiColorTools::NFmiBlendRule rule = ColorTools::checkrule(liter->rule());
							  path.Project(theArea.get());
							  path.SimplifyLines(10);
							  path.Stroke(theImage,liter->color(),rule);
							  
							}
						}
					  
					  // Bang the foreground
					  
					  if(theForeground != "")
						{
						  NFmiColorTools::NFmiBlendRule rule = ColorTools::checkrule(theForegroundRule);
						  
						  theImage.Composite(theForegroundImage,rule,kFmiAlignNorthWest,0,0,1);
						  
						}

					  // Draw wind arrows if so requested
					  
					  NFmiEnumConverter converter;
					  if((!theArrowPoints.empty() || (theWindArrowDX!=0 && theWindArrowDY!=0)) &&
						 (theArrowFile!=""))
						{
						  
						  FmiParameterName param = FmiParameterName(NFmiEnumConverter().ToEnum(theDirectionParameter));
						  if(param==kFmiBadParameter)
							throw runtime_error("Unknown parameter "+theDirectionParameter);
						  
						  // Find the proper queryinfo to be used
						  // Note that qi will be used later on for
						  // getting the coordinate matrices
						  
						  ok = false;
						  for(qi=0; qi<theQueryStreams.size(); qi++)
							{
							  theQueryInfo = theQueryStreams[qi];
							  theQueryInfo->Param(param);
							  ok = theQueryInfo->IsParamUsable();
							  if(ok) break;
							}
						  
						  if(!ok)
							throw runtime_error("Parameter is not usable: " + theDirectionParameter);

						  // Read the arrow definition
						  
						  NFmiPath arrowpath;
						  if(theArrowFile != "meteorological")
							{
							  ifstream arrow(theArrowFile.c_str());
							  if(!arrow)
								throw runtime_error("Could not open " + theArrowFile);
							  // Read in the entire file
							  string pathstring = NFmiStringTools::ReadFile(arrow);
							  arrow.close();

							  // Convert to a path
							  
							  arrowpath.Add(pathstring);
							}
						  
						  // Handle all given coordinates
						  
						  list<NFmiPoint>::const_iterator iter;
						  
						  for(iter=theArrowPoints.begin();
							  iter!=theArrowPoints.end();
							  ++iter)
							{

							  // The start point
							  NFmiPoint xy0 = theArea->ToXY(*iter);

							  // Skip rendering if the start point is masked

							  if(IsMasked(xy0,theMask,theMaskImage))
								continue;

							  float dir = theQueryInfo->InterpolatedValue(*iter);
							  if(dir==kFloatMissing)	// ignore missing
								continue;
							  
							  float speed = -1;
							  
							  if(theQueryInfo->Param(FmiParameterName(converter.ToEnum(theSpeedParameter))))
								speed = theQueryInfo->InterpolatedValue(*iter);
							  theQueryInfo->Param(FmiParameterName(converter.ToEnum(theDirectionParameter)));
							  
						  
							  // Direction calculations
							  
							  const float pi = 3.141592658979323;
							  const float length = 0.1;	// degrees
							  
							  float x1 = iter->X()+sin(dir*pi/180)*length;
							  float y1 = iter->Y()+cos(dir*pi/180)*length;
							  
							  NFmiPoint xy1 = theArea->ToXY(NFmiPoint(x1,y1));
							  
							  // Calculate the actual angle
							  
							  float alpha = atan2(xy1.X()-xy0.X(),
												  xy1.Y()-xy0.Y());
							  
							  // Create a new path
							  
							  NFmiPath thispath;

							  if(theArrowFile == "meteorological")
								thispath.Add(GramTools::metarrow(speed*theWindArrowScaleC));
							  else
								thispath.Add(arrowpath);

							  if(speed>0 && speed!=kFloatMissing)
								thispath.Scale(theWindArrowScaleA*log10(theWindArrowScaleB*speed+1)+theWindArrowScaleC);
							  thispath.Scale(theArrowScale);
							  thispath.Rotate(alpha*180/pi);
							  thispath.Translate(xy0.X(),xy0.Y());
							  
							  // And render it
							  
							  thispath.Fill(theImage,
											ColorTools::checkcolor(theArrowFillColor),
											ColorTools::checkrule(theArrowFillRule));
							  thispath.Stroke(theImage,
											  ColorTools::checkcolor(theArrowStrokeColor),
											  ColorTools::checkrule(theArrowStrokeRule));
							}
						  
						  // Draw the full grid if so desired
						  
						  if(theWindArrowDX!=0 && theWindArrowDY!=0)
							{

							  NFmiDataMatrix<float> speedvalues(vals.NX(),vals.NY(),-1);
							  if(theQueryInfo->Param(FmiParameterName(converter.ToEnum(theSpeedParameter))))
								theQueryInfo->Values(speedvalues);
							  theQueryInfo->Param(FmiParameterName(converter.ToEnum(theDirectionParameter)));
							  
							  shared_ptr<NFmiDataMatrix<NFmiPoint> > worldpts = theQueryInfo->LocationsWorldXY(*theArea);							  
							  for(unsigned int j=0; j<worldpts->NY(); j+=theWindArrowDY)
								for(unsigned int i=0; i<worldpts->NX(); i+=theWindArrowDX)
								  {
									// The start point
									
									NFmiPoint latlon = theArea->WorldXYToLatLon((*worldpts)[i][j]);
									NFmiPoint xy0 = theArea->ToXY(latlon);

									// Skip rendering if the start point is masked
									if(IsMasked(xy0,theMask,theMaskImage))
									  continue;

									float dir = vals[i][j];
									if(dir==kFloatMissing)	// ignore missing
									  continue;
									
									float speed = speedvalues[i][j];

									// Direction calculations
									
									const float pi = 3.141592658979323;
									const float length = 0.1;	// degrees
									
									float x0 = latlon.X();
									float y0 = latlon.Y();
									
									float x1 = x0+sin(dir*pi/180)*length;
									float y1 = y0+cos(dir*pi/180)*length;
									
									NFmiPoint xy1 = theArea->ToXY(NFmiPoint(x1,y1));
									
									// Calculate the actual angle
									
									float alpha = atan2(xy1.X()-xy0.X(),
														xy1.Y()-xy0.Y());
									
									// Create a new path
									
									NFmiPath thispath;
									if(theArrowFile == "meteorological")
									  thispath.Add(GramTools::metarrow(speed*theWindArrowScaleC));
									else
									  thispath.Add(arrowpath);
									if(speed>0 && speed != kFloatMissing)
									  thispath.Scale(theWindArrowScaleA*log10(theWindArrowScaleB*speed+1)+theWindArrowScaleC);
									thispath.Scale(theArrowScale);
									thispath.Rotate(alpha*180/pi);
									thispath.Translate(xy0.X(),xy0.Y());
									
									// And render it
									
									thispath.Fill(theImage,
												  ColorTools::checkcolor(theArrowFillColor),
												  ColorTools::checkrule(theArrowFillRule));
									thispath.Stroke(theImage,
													ColorTools::checkcolor(theArrowStrokeColor),
													ColorTools::checkrule(theArrowStrokeRule));
								  }
							}
						}
					  
					  // Draw labels
					  
					  for(piter=pbegin; piter!=pend; ++piter)
						{
						  
						  // Draw label markers first
						  
						  if(!piter->labelMarker().empty())
							{
							  // Establish that something is to be done
							  
							  if(piter->labelPoints().empty())
								continue;
							  
							  // Establish the marker specs
							  
							  NFmiImage marker;
							  marker.Read(piter->labelMarker());
							  
							  NFmiColorTools::NFmiBlendRule markerrule = ColorTools::checkrule(piter->labelMarkerRule());
							  
							  float markeralpha = piter->labelMarkerAlphaFactor();
							  
							  // Draw individual points
							  
							  unsigned int pointnumber = 0;
							  list<pair<NFmiPoint,NFmiPoint> >::const_iterator iter;
							  for(iter=piter->labelPoints().begin();
								  iter!=piter->labelPoints().end();
								  ++iter)
								{
								  // The point in question
								  
								  NFmiPoint xy = theArea->ToXY(iter->first);
								  
								  // Skip rendering if the start point is masked
								  
								  if(IsMasked(xy,theMask,theMaskImage))
									continue;
								  
								  // Skip rendering if LabelMissing is "" and value is missing
								  if(piter->labelMissing().empty())
									{
									  float value = piter->labelValues()[pointnumber++];
									  if(value == kFloatMissing)
										continue;
									}
								  
								  theImage.Composite(marker,
													 markerrule,
													 kFmiAlignCenter,
													 FmiRound(xy.X()),
													 FmiRound(xy.Y()),
													 markeralpha);
								}
							  
							}
						  
						  // Label markers now drawn, only label texts remain
						  
						  // Quick exit from loop if no labels are
						  // desired for this parameter
						  
						  if(piter->labelFormat() == "")
							continue;

						  // Create the font object to be used
						  
						  NFmiFontHershey font(piter->labelFont());
							  
						  // Create the text object to be used
						  
						  NFmiText text("",
										font,
										piter->labelSize(),
										0.0,	// x
										0.0,	// y
										AlignmentValue(piter->labelAlignment()),
										piter->labelAngle());
						  
						  
						  NFmiText caption(piter->labelCaption(),
										   font,
										   piter->labelSize(),
										   0.0,
										   0.0,
										   AlignmentValue(piter->labelCaptionAlignment()),
										   piter->labelAngle());
						  
						  // The rules
						  
						  NFmiColorTools::NFmiBlendRule fillrule
							= ColorTools::checkrule(piter->labelFillRule());
						  
						  NFmiColorTools::NFmiBlendRule strokerule
							= ColorTools::checkrule(piter->labelStrokeRule());
						  
						  // Draw labels at specifing latlon points if requested
						  
						  list<pair<NFmiPoint,NFmiPoint> >::const_iterator iter;
						  
						  int pointnumber = 0;
						  for(iter=piter->labelPoints().begin();
							  iter!=piter->labelPoints().end();
							  ++iter)
							{

							  // The point in question
							  
							  float x,y;
							  if(iter->second.X() == kFloatMissing)
								{
								  NFmiPoint xy = theArea->ToXY(iter->first);
								  x = xy.X();
								  y = xy.Y();
								}
							  else
								{
								  x = iter->second.X();
								  y = iter->second.Y();
								}

							  // Skip rendering if the start point is masked
							  
							  if(IsMasked(NFmiPoint(x,y),theMask,theMaskImage))
								continue;

							  float value = piter->labelValues()[pointnumber++];
							  
							  // Convert value to string
							  string strvalue = piter->labelMissing();
							  
							  if(value!=kFloatMissing)
								{
								  char tmp[20];
								  sprintf(tmp,piter->labelFormat().c_str(),value);
								  strvalue = tmp;
								}

							  // Don't bother drawing empty strings
							  if(strvalue.empty())
								continue;
							  
							  // Set new text properties
							  
							  text.Text(strvalue);
							  text.X(x + piter->labelOffsetX());
							  text.Y(y + piter->labelOffsetY());
							  
							  // And render the text
							  
							  text.Fill(theImage,piter->labelFillColor(),fillrule);
							  text.Stroke(theImage,piter->labelStrokeColor(),strokerule);
							  
							  // Then the label caption
							  
							  if(!piter->labelCaption().empty())
								{
								  caption.X(text.X() + piter->labelCaptionDX());
								  caption.Y(text.Y() + piter->labelCaptionDY());
								  caption.Fill(theImage,piter->labelFillColor(),fillrule);
								  caption.Stroke(theImage,piter->labelStrokeColor(),strokerule);
								}
							  
							}
							  
						}
		  
  
					  
					  // Bang the combine image (legend, logo, whatever)
					  
					  if(theCombine != "")
						{
						  NFmiColorTools::NFmiBlendRule rule = ColorTools::checkrule(theCombineRule);
						  
						  theImage.Composite(theCombineImage,rule,kFmiAlignNorthWest,theCombineX,theCombineY,theCombineFactor);
						  
						}

					  // Finally, draw a time stamp on the image if so
					  // requested
					  
					  string thestamp = "";
					  
					  {
						int obsyy = t.GetYear();
						int obsmm = t.GetMonth();
						int obsdd = t.GetDay();
						int obshh = t.GetHour();
						int obsmi = t.GetMin();
						
						// Interpretation: The age of the forecast is the age
						// of the oldest forecast
						
						NFmiTime tfor;
						for(qi=0; qi<theQueryStreams.size(); qi++)
						  {
							theQueryInfo = theQueryStreams[qi];
							NFmiTime futctime = theQueryInfo->OriginTime(); 
							NFmiTime tlocal = TimeTools::ConvertZone(futctime,theTimeStampZone);
							if(qi==0 || tlocal.IsLessThan(tfor))
							  tfor = tlocal;
						  }
						
						int foryy = tfor.GetYear();
						int formm = tfor.GetMonth();
						int fordd = tfor.GetDay();
						int forhh = tfor.GetHour();
						int formi = tfor.GetMin();
						
						char buffer[100];
						
						if(theTimeStampImage == "obs")
						  {
							// hh:mi dd.mm.yyyy
							sprintf(buffer,"%02d:%02d %02d.%02d.%04d",
									obshh,obsmi,obsdd,obsmm,obsyy);
							thestamp = buffer;
						  }
						else if(theTimeStampImage == "for")
						  {
							// hh:mi dd.mm.yyyy
							sprintf(buffer,"%02d:%02d %02d.%02d.%04d",
									forhh,formi,fordd,formm,foryy);
							thestamp = buffer;
						  }
						else if(theTimeStampImage == "forobs")
						  {
							// hh:mi dd.mm.yyyy +hh
							long diff = t.DifferenceInMinutes(tfor);
							if(diff%60==0 && theTimeStep%60==0)
							  sprintf(buffer,"%02d.%02d.%04d %02d:%02d %s%ldh",
									  fordd,formm,foryy,forhh,formi,
									  (diff<0 ? "" : "+"), diff/60);
							else
							  sprintf(buffer,"%02d.%02d.%04d %02d:%02d %s%ldm",
									  fordd,formm,foryy,forhh,formi,
									  (diff<0 ? "" : "+"), diff);
							thestamp = buffer;
						  }
					  }
					  
					  if(!thestamp.empty())
						{
						  NFmiFontHershey font("TimesRoman-Bold");
						  
						  int x = theTimeStampImageX;
						  int y = theTimeStampImageY;
						  
						  if(x<0) x+= theImage.Width();
						  if(y<0) y+= theImage.Height();
						  
						  NFmiText text(thestamp,font,14,x,y,kFmiAlignNorthWest,0.0);
						  
						  // And render the text
						  
						  NFmiPath path = text.Path();
						  
						  NFmiEsriBox box = path.BoundingBox();
						  
						  NFmiPath rect;
						  int w = 4;
						  rect.MoveTo(box.Xmin()-w,box.Ymin()-w);
						  rect.LineTo(box.Xmax()+w,box.Ymin()-w);
						  rect.LineTo(box.Xmax()+w,box.Ymax()+w);
						  rect.LineTo(box.Xmin()-w,box.Ymax()+w);
						  rect.CloseLineTo();
						  
						  rect.Fill(theImage,
									NFmiColorTools::MakeColor(180,180,180,32),
									NFmiColorTools::kFmiColorOver);
						  
						  path.Stroke(theImage,
									  NFmiColorTools::Black,
									  NFmiColorTools::kFmiColorCopy);
						  
						}

					  // dx and dy labels have now been extracted into a list,
					  // disable adding them again and again and again..
					  
					  labeldxdydone = true;
					  
					  // Save
					  
					  if(globals.verbose)
						cout << "Writing " << filename << endl;
					  if(theFormat=="png")
						theImage.WritePng(filename);
					  else if(theFormat=="jpg" || theFormat=="jpeg")
						theImage.WriteJpeg(filename);
					  else if(theFormat=="gif")
						theImage.WriteGif(filename);
					  else
						throw runtime_error("Image format "+theFormat+" is not supported");
					}
				}
			  
			  else
				throw runtime_error("draw " + command + " not implemented");
			}
		  else
			throw runtime_error("Unknown command " + command);
		}
    }
  return 0;
}

// ----------------------------------------------------------------------
// Main program.
// ----------------------------------------------------------------------

int main(int argc, const char* argv[])
{
  try
	{
	  return domain(argc, argv);
	}
  catch(const runtime_error & e)
	{
	  cerr << "Error: qdcontour failed due to" << endl
		   << "--> " << e.what() << endl;
	  return 1;
	}
}

// ======================================================================
