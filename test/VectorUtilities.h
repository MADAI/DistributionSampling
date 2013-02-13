#ifndef __vector_utilities_h
#define __vector_utilities_h

#include <iostream>
#include <iterator>

// A helper function to simplify printing vectors to std::ostreams
template< class T >
std::ostream& operator<<( std::ostream& os, const std::vector< T >& v )
{
  copy( v.begin(), v.end(), std::ostream_iterator< T >( std::cout, " " ));
  return os;
}


#endif
