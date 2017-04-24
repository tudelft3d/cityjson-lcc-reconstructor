#include <iostream>
#include <fstream>
#include <map>

#include "typedefs.h"

using namespace std;

class CityGmlReader
{
	typedef shared_ptr<const citygml::CityModel> CityMdl;

private:
	LCC lcc;
	int start_i = 0, object_limit = -1;
	int precision = 3;

	ostringstream log_str;
	map<string, Dart_handle> index_1_cell;

public:
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

		st << fixed << setprecision(precision);

		st << round_by(p.x(), precision) << "-" << round_by(p.y(), precision) << "-" << round_by(p.z(), precision);

		return st.str();
	}

	string get_point_name(TVec3d v)
	{
		return get_point_name(tvec_to_point(v));
	}

	string index_to_string(map<string, Dart_handle>::iterator it)
	{
		ostringstream str;

		str << it->first << ": ";
		if (it->second == NULL)
			str << "IS NULL" << endl;
		else
			str << lcc.point(it->second) << endl;

		return str.str();
	}

	void show_null_index_records()
	{
		for( map<string, Dart_handle>::iterator it = index_1_cell.begin(); it != index_1_cell.end(); it++)
		{
			if (it->second == NULL)
			{
				cout << "NOW " << it->first << " IS NULL!!!!!" << endl;
			}
		}
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

		if (index_1_cell.find(get_edge_name(v2, v1)) != index_1_cell.end())
		{
			cout << "Sewing " << get_edge_name(v1, v2) << " with " << get_edge_name(v2, v1) << endl;
			lcc.sew<2>(result, index_1_cell[get_edge_name(v2, v1)]);

			index_1_cell.erase(get_edge_name(v1, v2));
			index_1_cell.erase(get_edge_name(v2, v1));
		}

		show_null_index_records();

		return result;
	}

	void polygon_to_string(shared_ptr<const citygml::Polygon> poly, ostringstream &stream, int level = 1)
	{
		stream << "+" << string(level * 2 - 2, '-') << " Polygon (" << poly->getId() << ")" << endl;

		const vector<TVec3d> verts = poly->getVertices();
		int i = 0;
		// The loop condition is a stupid hack in order to avoid vertices of inner rings
		for( vector<const TVec3d>::iterator it = verts.begin(); (it + 1) != verts.end() && (*it != *verts.begin() || i == 0); it++, i = 1)
		{
			if (get_point_name(*it) == get_point_name(*(it + 1)))
			{
				stream << "|" << string( level * 2 - 1, '.') << " IGNORING <" << it->x << "=" << (it + 1)->x << ", " << it->y << ", " << it->z << "> - " << get_point_name(*it) << "-" << get_point_name(*(it + 1)) << endl;
			}
			else
			{
				stream << "|" << string( level * 2 - 1, '.') << " <" << it->x << ", " << it->y << ", " << it->z << ">" << endl;
				add_edge(*it, *(it + 1));
			}
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

		for (int i = 0; i < obj.getChildCityObjectsCount(); i++)
		{
			string_stream << "+ Child City Object +" << endl;
			string_stream << cityobject_to_string( obj.getChildCityObject(i));
			string_stream << "+ dEND of Child City Object +" << endl;
		}

		return string_stream.str();
	}

	LCC readCityModel(CityMdl city)
	{
		if (object_limit == -1)
		{
			object_limit = city->getNumRootCityObjects();
		}

		for (int i = 0; i < object_limit; i++)
		{
			const citygml::CityObject *obj;
			obj = &city->getRootCityObject(i);

			log_str << cityobject_to_string(*obj);

			log_str << i << ") ";
			lcc.display_characteristics(log_str);
			log_str << endl << endl;
		}

		return lcc;
	}

	void setObjectLimit(int new_limit)
	{
		object_limit = new_limit;
	}

	string getLog()
	{
		return log_str.str();
	}

	string getIndex()
	{
		ostringstream str;

		str << "This is the final status of the 1-cell index" << endl << "--------" << endl;
		for(map<string, Dart_handle>::iterator it=index_1_cell.begin(); it!=index_1_cell.end(); ++it)
		{
			str << index_to_string(it);
		}

		return str.str();
	}

	LCC getLinearCellComplex()
	{
		return lcc;
	}

	void setPrecision(int new_precision)
	{
		precision = new_precision;
	}
};