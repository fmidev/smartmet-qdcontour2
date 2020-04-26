// ======================================================================
/*!
 * \file
 * \brief Interface of namespace MetaFunctions
 */
// ======================================================================
/*!
 * \namespace MetaFunctions
 * \brief Various functions related to meteorology
 *
 */
// ======================================================================

#ifndef METAFUNCTIONS_H
#define METAFUNCTIONS_H

#include <string>
#include "LazyQueryData.h"
#include "NFmiDataMatrix.h"

namespace MetaFunctions
{
bool isMeta(const std::string& theFunction);
int id(const std::string& theFunction);
NFmiDataMatrix<float> values(const std::string& theFunction, LazyQueryData& theQI);

}  // namespace MetaFunctions

#endif  // METAFUNCTIONS_H

// ======================================================================
