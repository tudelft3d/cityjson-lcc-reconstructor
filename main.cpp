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
map<string, Dart_handle> index_1_cell;

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
	 
	 st << round_by(p.x(), 3) << "-" << round_by(p.y(), 3) << "-" << round_by(p.z(), 3);

	 return st.str();
}

string get_point_name(TVec3d v)
{
	return get_point_name(tvec_to_point(v));
}

template <class T>
string get_edge_name(T v1, T v2)
{
	return get_point_name(v1) + "-" + get_point_name(v2);
}

Dart_handle add_vertex(TVec3d v, int i_free = -1)
{
	Dart_handle result;
	bool found = false;

	for( LCC::Dart_range::iterator it = lcc.darts().begin(); it != lcc.darts().end(); ++it )
	{
		Point c_point = lcc.point(it);
		if( c_point == tvec_to_point(v) )
		{
			if (i_free < 0 || lcc.beta(it, i_free) == lcc.null_dart_handle)
			{
				cout << "Found " << get_point_name(c_point) << " as " << i_free << "-free dart." << endl;
				found = true;
				result = it;
				break;
			}
		}
	}

	if( !found )
	{
		result = lcc.create_dart( tvec_to_point(v) );
		cout << "Created " << lcc.point(result) << endl;
	}

	return result;
}

Dart_handle add_edge(TVec3d v1, TVec3d v2)
{
	Dart_handle result;

	result = add_vertex(v1, 1);
	Dart_handle temp_dart = add_vertex(v2, 0);

	lcc.sew<1>(result, temp_dart);

	index_1_cell[get_edge_name(v1, v2)] = result;

	if (index_1_cell[get_edge_name(v2, v1)] != NULL)
	{
		cout << "Sewing " << get_edge_name(v1, v2) << " with " << get_edge_name(v2, v1) << endl;
		lcc.sew<2>(result, index_1_cell[get_edge_name(v2, v1)]);

		index_1_cell.erase(get_edge_name(v1, v2));
		index_1_cell.erase(get_edge_name(v2, v1));
	}

	return result;
}

void polygon_to_string(shared_ptr<const citygml::Polygon> poly, ostringstream &stream, int level = 1)
{
	stream << "+" << string(level * 2 - 2, '-') << " Polygon (" << poly->getId() << ")" << endl;

	const vector<TVec3d> verts = poly->getVertices();
	for( vector<const TVec3d>::iterator it = verts.begin(); (it + 1) != verts.end(); )
	{
		stream << "|" << string( level * 2 - 1, '.') << " <" << it->x << ", " << it->y << ", " << it->z << ">" << endl;
		add_edge(*it, *++it);
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
	Point p = Point(1,1,1);
	
	citygml::ParserParams params;
	const char *filename = argv[1];

	shared_ptr<const citygml::CityModel> city = citygml::load(filename, params);
	cout << "We found " << city->getNumRootCityObjects() << " root city objects!" << endl << endl;

	int object_limit = city->getNumRootCityObjects();
	if (argc > 3)
	{
		object_limit = atoi(argv[3]);
	}

	for (int i = 0; i < object_limit; i++)
	{
		const citygml::CityObject *obj;
		obj = &city->getRootCityObject(i);

		cout << cityobject_to_string(*obj);

		cout << i << ") ";
		lcc.display_characteristics(cout);
		cout << endl;
	}

	for(map<string, Dart_handle>::iterator it=index_1_cell.begin(); it!=index_1_cell.end(); ++it)
		cout << it->first << endl;

	if (argc > 2)
	{
		const char *out_filename = argv[2];
		save_combinatorial_map(lcc, out_filename);
	}

	return 0;
}
