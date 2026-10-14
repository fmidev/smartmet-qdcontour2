// ======================================================================
/*!
 * \file
 * \brief Interface of class LabelLocator
 */
// ======================================================================

#ifndef LABELLOCATOR_H
#define LABELLOCATOR_H

#include <list>
#include <map>

class LabelLocator
{
 public:
  ~LabelLocator();
  LabelLocator();

  bool empty() const;
  void clear();

  void boundingBox(int theX1, int theY1, int theX2, int theY2);
  void minDistanceToSameValue(float theDistance);
  void minDistanceToDifferentValue(float theDistance);
  void minDistanceToDifferentParameter(float theDistance);

  void parameter(int theParameter);
  void nextTime();

  void add(float theContour, int theX, int theY);

  typedef std::pair<int, int> XY;
  typedef std::multimap<float, XY> Coordinates;
  typedef std::map<float, Coordinates> ContourCoordinates;
  typedef std::map<int, ContourCoordinates> ParamCoordinates;

  const ParamCoordinates& chooseLabels();

 private:
  // Intentionally disabled:

  LabelLocator(const LabelLocator& theLocator);
  LabelLocator& operator=(const LabelLocator& theLocator);

  bool itHasBBox = false;
  int itsBBoxX1 = 0;
  int itsBBoxY1 = 0;
  int itsBBoxX2 = 0;
  int itsBBoxY2 = 0;

  float itsMinDistanceToSameValue = 175;
  float itsMinDistanceToDifferentValue = 30;
  float itsMinDistanceToDifferentParameter = 30;

  int itsActiveParameter = 0;
  ParamCoordinates itsPreviousCoordinates;
  ParamCoordinates itsCurrentCoordinates;

  // Private methods:

  bool inside(int theX1, int theY1) const;

  float distanceToBorder(float theX, float theY) const;

  void removeCandidates(ParamCoordinates& theCandidates,
                        const XY& thePoint,
                        int theParam,
                        float theContour);

  void removeEmpties(ParamCoordinates& theCandidates);

};  // class LabelLocator

#endif  // LABELLOCATOR_H

// ======================================================================
