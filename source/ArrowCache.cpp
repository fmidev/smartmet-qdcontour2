Client: Concurrent Versions System (CVSNT) 2.5.03 (Scorpio) Build 2260 (client/server)
Server: Concurrent Versions System (CVS) 1.11.1p1 (client/server)
// ======================================================================
/*!
 * \file
 * \brief Implementation of class ArrowCache
 */
// ======================================================================

#include "ArrowCache.h"
#include "NFmiStringTools.h"
#include <fstream>
#include <stdexcept>

using namespace std;

// ----------------------------------------------------------------------
/*!
 * \brief Test if the cache is empty
 */
// ----------------------------------------------------------------------

bool ArrowCache::empty() const
{
  return itsCache.empty();
}

// ----------------------------------------------------------------------
/*!
 * \brief Clear the cache
 */
// ----------------------------------------------------------------------

void ArrowCache::clear()
{
  itsCache.clear();
}

// ----------------------------------------------------------------------
/*!
 * \brief Return the desired arrow from the cache
 */
// ----------------------------------------------------------------------

const string & ArrowCache::find(const string & theName)
{
  cache_type::const_iterator it = itsCache.find(theName);
  if(it != itsCache.end())
	return it->second;

  ifstream arrow(theName.c_str());
  if(!arrow)
	throw runtime_error("Could not open arrow '"+theName+"' for reading");
  string pathstring = NFmiStringTools::ReadFile(arrow);
  arrow.close();

  pair<cache_type::const_iterator,bool> ret = itsCache.insert(cache_type::value_type(theName,pathstring));

  if(!ret.second)
	throw runtime_error("Failed to insert arrow '"+theName+"' into the internal cache");

  return ret.first->second;
}
