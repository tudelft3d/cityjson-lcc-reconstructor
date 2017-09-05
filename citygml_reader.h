#include <iostream>
#include <fstream>
#include <map>
#include <vector>

#include "typedefs.h"

using namespace std;

class CityGmlReader
{
	typedef shared_ptr<const citygml::CityModel> CityMdl;

private:
	LCC lcc;
	int start_i = 0, object_limit = -1;
	int precision = 3;
	string id_filter = "";
	bool index_1_per_object = false;

	ostringstream log_str;
	map<string, vector<Dart_handle> > index_0_cell;
	map<string, Dart_handle> index_1_cell;
	map<string, Dart_handle> index_2_cell;

public:
	Point tvec_to_point(TVec3d v)
	{
		return Point(v.x, v.y, v.z);
	}

	double round_by(double f, int d)
	{
		double ex = pow(10, d);
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
				log_str << "NOW " << it->first << " IS NULL!!!!!" << endl;
			}
		}
	}

	template <class T>
	string get_edge_name(T v1, T v2)
	{
		return get_point_name(v1) + "-" + get_point_name(v2);
	}

	void get_polygon_name(vector<Dart_handle> darts, string &name, Dart_handle &lowest_dart, bool step_forward)
	{
		string lowest_name = get_point_name(lcc.point(darts.front()));
		lowest_dart = darts.front();
		for( vector<Dart_handle>::iterator it = darts.begin(); it != darts.end(); ++it)
		{
			string new_point = get_point_name(lcc.point(*darts.begin()));
			if (new_point < lowest_name)
			{
				lowest_name = new_point;
				lowest_dart = *it;
			}
		}

		int beta_i = step_forward ? 1 : 0;
		name = lowest_name;
		Dart_handle next = lcc.beta(lowest_dart, beta_i);
        while (next != lowest_dart && next != lcc.null_dart_handle)
		{
			name += "-" + get_point_name(lcc.point(next));
			next = lcc.beta(next, beta_i);
		}
	}

	Dart_handle add_vertex(TVec3d v, int i_free = -1)
	{
		Dart_handle result;
		bool found = false;
		
		if (index_0_cell.find(get_point_name(v)) != index_0_cell.end())
		{
			for (vector<Dart_handle>::iterator it = index_0_cell[get_point_name(v)].begin(); it != index_0_cell[get_point_name(v)].end(); ++it)
			{
				if (i_free < 0 || lcc.beta(*it, i_free) == lcc.null_dart_handle)
				{
					// log_str << "Found " << get_point_name(c_point) << " as " << i_free << "-free dart." << endl;
					found = true;
					result = *it;

					index_0_cell[get_point_name(v)].erase(it);
					if (index_0_cell[get_point_name(v)].empty())
					{
						index_0_cell.erase(get_point_name(v));
					}

					break;
				}
			}
		}

		if( !found )
		{
			result = lcc.create_dart( tvec_to_point(v) );
			index_0_cell[ get_point_name(v) ].push_back(result);
			// log_str << "Created " << lcc.point(result) << endl;
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
            // log_str << "Sewing " << get_edge_name(v1, v2) << " with " << get_edge_name(v2, v1) << endl;
			lcc.sew<2>(result, index_1_cell[get_edge_name(v2, v1)]);

			index_1_cell.erase(get_edge_name(v1, v2));
			index_1_cell.erase(get_edge_name(v2, v1));
		}

		show_null_index_records();

		return result;
	}

    vector<Dart_handle> polygon_to_string(shared_ptr<const citygml::Polygon> poly, ostringstream &stream, int level = 1)
	{
        vector<Dart_handle> result;

		stream << "+" << string(level * 2 - 2, '-') << " Polygon (" << poly->getId() << ")" << endl;

		const vector<TVec3d> verts = poly->getVertices();
		int i = 0;

		// This loop condition is a hack in order to avoid vertices of inner rings
		for( vector<const TVec3d>::iterator it = verts.begin(); (it + 1) != verts.end() && (*it != *verts.begin() || i == 0); it++)
		{
			if (get_point_name(*it) == get_point_name(*(it + 1)))
			{
				stream << "|" << string( level * 2 - 1, '.') << " IGNORING <" << it->x << "=" << (it + 1)->x << ", " << it->y << ", " << it->z << "> - " << get_point_name(*it) << "-" << get_point_name(*(it + 1)) << endl;
			}
			else
			{
				stream << "|" << string( level * 2 - 1, '.') << " <" << it->x << ", " << it->y << ", " << it->z << ">" << endl;
				i++;
			}
		}

		if (i > 2)
		{
			i = 0;
			for( vector<const TVec3d>::iterator it = verts.begin(); (it + 1) != verts.end() && (*it != *verts.begin() || i == 0); it++, i++)
			{
				if (get_point_name(*it) != get_point_name(*(it + 1)))
				{
                    result.push_back(add_edge(*it, *(it + 1)));
				}
			}

            if (result.size() > 2)
            {
                // Try to 3-sew with other polygons
                string new_name;
                Dart_handle new_dart;
                get_polygon_name(result, new_name, new_dart, true);

                string inverse_name;
                get_polygon_name(result, inverse_name, new_dart, false);

                if (index_2_cell.find(inverse_name) != index_2_cell.end())
                {
                    Dart_handle other_dart = lcc.beta<0>(index_2_cell[inverse_name]);
                    log_str << "3-Sewing " << new_name << " with " << inverse_name << endl;
                    lcc.sew<3>(new_dart, other_dart);

                    index_2_cell.erase(inverse_name);
                }
                else
                {
                    index_2_cell[new_name] = new_dart;
                }
            }
        }
		else
		{
			log_str << "Ignoring this polygon because only 2 individual lines where found." << endl;
		}

        log_str << "Now we have " << lcc.number_of_darts() << " darts!" << endl << endl;

        return result;
	}

    vector<Dart_handle> geometry_to_string(const citygml::Geometry &geo, ostringstream &stream, int level = 1)
	{
        vector<Dart_handle> result;

		stream << string(level * 2 - 1, '-') << " Geometry (" << geo.getId() << ")" << endl;

		stream << "|" << string(level * 2 - 1, '-') << " Polygon count: " << geo.getPolygonsCount() << endl;
		for (int i = 0; i < geo.getPolygonsCount(); i++)
		{
            result = polygon_to_string( geo.getPolygon(i), stream, level + 1 );
		}

		stream << "|" << string(level * 2 - 1, '-') << " LineString count: " << geo.getLineStringCount() << endl;

		for (int i = 0; i < geo.getGeometriesCount(); i++)
		{
            vector<Dart_handle> new_darts;
            new_darts = geometry_to_string( geo.getGeometry(i), stream, level + 1 );

            for(vector<Dart_handle>::iterator it = new_darts.begin(); it != new_darts.end(); ++it)
            {
                result.push_back(*it);
            }
		}

        return result;
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
            vector<Dart_handle> darts = geometry_to_string( obj.getGeometry(i), string_stream );

            for (vector<Dart_handle>::iterator it = darts.begin(); it != darts.end(); ++it)
            {
                init_volume(*it);
                lcc.info<3>(*it).set_guid(obj.getId());
                set_attributes(*it, obj.getAttributes());
            }
		}

		for (int i = 0; i < obj.getChildCityObjectsCount(); i++)
		{
			string_stream << "+ Child City Object +" << endl;
			string_stream << cityobject_to_string( obj.getChildCityObject(i));
			string_stream << "+ END of Child City Object +" << endl;
		}

		return string_stream.str();
	}

	void init_all_volumes()
	{
		for (LCC::One_dart_per_cell_range<3>::iterator
       		it(lcc.one_dart_per_cell<3>().begin());
       		it.cont(); ++it)
            init_volume(it);
	}

    void init_volume(Dart_handle dh)
    {
        if ( lcc.attribute<3>(dh)==LCC::null_handle )
            { lcc.set_attribute<3>(dh, lcc.create_attribute<3>()); }
    }

    void set_attributes(Dart_handle dh, citygml::AttributesMap attributes)
    {
        lcc.info<3>(dh).set_attributes(attributes);
    }

	LCC readCityModel(CityMdl city)
	{
		if (object_limit == -1)
		{
			object_limit = city->getNumRootCityObjects();
		}

		if (start_i + object_limit > city->getNumRootCityObjects())
		{
			object_limit = city->getNumRootCityObjects() - start_i;
		}

		for (int i = start_i; i < start_i + object_limit; i++)
		{
			const citygml::CityObject *obj;
			obj = &city->getRootCityObject(i);

			if (!id_filter.empty())
			{
				size_t pos = obj->getId().find(id_filter);
				if (pos == string::npos)
				{
					continue;
				}
			}

			log_str << cityobject_to_string(*obj);

			log_str << i << ") ";
			lcc.display_characteristics(log_str);
			log_str << endl << endl;

			if (index_1_per_object) {
				index_1_cell.clear();
			}

            cout << "\rDone with " << i - start_i + 1 << "/" << object_limit;
			cout.flush();
		}

		cout << endl;

		init_all_volumes();

		return lcc;
	}

	void setStartingIndex(int start_index)
	{
		start_i = start_index;
	}

	int getStartingIndex()
	{
		return start_i;
	}

	void setObjectLimit(int new_limit)
	{
		object_limit = new_limit;
	}

	int getObjectLimit()
	{
		return object_limit;
	}

	void setFilter(string filter)
	{
		id_filter = filter;
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

	int getPrecision()
	{
		return precision;
	}

	void setIndexPerObject(bool new_value)
	{
		index_1_per_object = new_value;
	}

	bool getIndexPerObject()
	{
		return index_1_per_object;
	}
};
