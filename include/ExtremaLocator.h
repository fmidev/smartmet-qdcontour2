// ======================================================================
/*!
 * \file
 * \brief Interface of class ExtremaLocator
 */
// ======================================================================

#ifndef EXTREMALOCATOR_H
#define EXTREMALOCATOR_H

#include <list>
#include <map>

class ExtremaLocator
{
 public:
  enum Extremum
  {
    Minimum,
    Maximum
  };

  ~ExtremaLocator();
  ExtremaLocator();

  bool empty() const;
  void clear();

  void minDistanceToSame(float theDistance);
  void minDistanceToDifferent(float theDistance);

  void nextTime();

  void add(Extremum theType, double theX, double theY);

  typedef std::pair<double, double> XY;
  typedef std::list<XY> Coordinates;
  typedef std::map<Extremum, Coordinates> ExtremaCoordinates;

  const ExtremaCoordinates& chooseCoordinates();

 private:
  // Intentionally disabled:

  ExtremaLocator(const ExtremaLocator& theLocator);
  ExtremaLocator& operator=(const ExtremaLocator& theLocator);

  float itsMinDistanceToSame;
  float itsMinDistanceToDifferent;

  ExtremaCoordinates itsPreviousCoordinates;
  ExtremaCoordinates itsCurrentCoordinates;

  // Private methods:

  Coordinates::const_iterator chooseOne(const Coordinates& theCandidates, Extremum theType);

  Coordinates::const_iterator chooseClosestToPrevious(const Coordinates& theCandidates,
                                                      const Coordinates& thePreviousChoices,
                                                      Extremum theType);

  Coordinates::const_iterator chooseClosestToBorder(const Coordinates& theCandidates,
                                                    Extremum theType);

  void removeCandidates(ExtremaCoordinates& theCandidates, const XY& thePoint, Extremum theType);

  void removeEmpties(ExtremaCoordinates& theCandidates);

};  // class ExtremaLocator

#endif  // EXTREMALOCATOR_H

// ======================================================================
