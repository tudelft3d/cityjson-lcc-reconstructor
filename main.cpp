#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <CGAL/Linear_cell_complex_for_combinatorial_map.h>
#include <stdio.h>
#include <citygml.h>
#include <citymodel.h>
#include <cityobject.h>
#include <geometry.h>
#include <polygon.h>

using namespace std;

typedef CGAL::Linear_cell_complex_for_combinatorial_map<3> LCC;
typedef LCC::Dart_handle Dart_handle;
typedef LCC::Point Point;
typedef LCC::Vector Vector;
typedef LCC::FT FT;

LCC lcc;
map<string, vector<Dart_handle>> index_1_cell;

Dart_handle add_vertex(TVec3d v)
{
	Dart_handle result;
	bool found = false;

	for( LCC::Dart_range::iterator it = lcc.darts().begin(); it != lcc.darts().end(); ++it )
	{
		if( lcc.point(it) == Point(v.x, v.y, v.z) )
		{
			cout << "Found " << lcc.point(it) << endl;
			found = true;
			result = it;
			break;
		}
	}

	if( !found )
	{
		result = lcc.create_dart(Point(v.x, v.y, v.z));
		cout << "Created " << lcc.point(result) << endl;
	}

	return result;
}

Point tvec_to_point(TVec3d v)
{
	return Point(v.x, v.y, v.z);
}

double round_by(float f, int d)
{
	float ex = pow(10, d);
	return round(f * ex) / ex;
}

string get_point_name(Point p)
{
	 ostringstream st;
	 
	 st << round_by(p.x(), 3) << round_by(p.y(), 3) << round_by(p.z(), 3);

	 return st.str();
}

void polygon_to_string(shared_ptr<const citygml::Polygon> poly, ostringstream &stream, int level = 1)
{
	stream << "+" << string(level * 2 - 2, '-') << " Polygon (" << poly->getId() << ")" << endl;

	const vector<TVec3d> verts = poly->getVertices();
	for( vector<const TVec3d>::iterator it = verts.begin(); it != verts.end(); it++ )
	{
		stream << "|" << string( level * 2 - 1, '.') << " <" << it->x << ", " << it->y << ", " << it->z << ">" << endl;
		add_vertex(*it);
	}

	cout << "Now we have " << lcc.number_of_darts() << " darts!" << endl << endl; 
}

void geometry_to_string(const citygml::Geometry &geo, ostringstream &stream, int level = 1)
{
	stream << string(level * 2 - 1, '-') << " Geometry (" << geo.getId() << ")" << endl;

	stream << "|" << string(level * 2 - 1, '-') << " Polygon count: " << geo.getPolygonsCount() << endl;
	for (int i = 0; i < geo.getPolygonsCount(); i++)
	{
		polygon_to_string( geo.getPolygon(i), stream, level + 1 );
	}

	stream << "|" << string(level * 2 - 1, '-') << " LineString count: " << geo.getLineStringCount() << endl;

	for (int i = 0; i < geo.getGeometriesCount(); i++)
	{
		geometry_to_string( geo.getGeometry(i), stream, level + 1 );
	}
}

string cityobject_to_string(const citygml::CityObject &obj)
{
	ostringstream string_stream;

	string_stream << "Object " << obj.getId() << endl;
	string_stream << "---------------------" << endl;

	string_stream << "Type: " << obj.getTypeAsString() << endl;
	string_stream << "Geometry count: " << obj.getGeometriesCount() << endl;

	for (int i = 0; i < obj.getGeometriesCount(); i++)
	{
		geometry_to_string( obj.getGeometry(i), string_stream );
	}

	return string_stream.str();
}

int main(int argc, char *argv[])
{
	LCC lcc;

	Point p = Point(1,1,1);
	
	citygml::ParserParams params;
	const char *filename = argv[1];

	shared_ptr<const citygml::CityModel> city = citygml::load(filename, params);
	cout << "We found " << city->getNumRootCityObjects() << " root city objects!" << endl << endl;

	if (city->getNumRootCityObjects() > 0)
	{
		const citygml::CityObject *obj;
		obj = &city->getRootCityObject(0);

		cout << cityobject_to_string(*obj);
	}

	return 1;
}
