#include <citygml.h>
#include <citymodel.h>
#include <cityobject.h>
#include <geometry.h>
#include <polygon.h>

typedef CGAL::Linear_cell_complex_for_combinatorial_map<3> LCC;
typedef LCC::Dart_handle Dart_handle;
typedef LCC::Point Point;
typedef LCC::Vector Vector;
typedef LCC::FT FT;