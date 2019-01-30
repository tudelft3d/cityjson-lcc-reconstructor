#include <iostream>
#include <fstream>
#include <map>
#include <vector>

#include "typedefs.h"

using namespace std;

class CityJsonReader
{
private:
  LCC lcc;
  nlohmann::json cityModel;
  unsigned int start_i = 0, object_limit = 0;
  int precision = 3;
  string id_filter = "";
  int lod_filter = -1;
  bool index_1_per_object = false;

  ostringstream log_str;
  unordered_map<string, vector<Dart_handle> > index_0_cell;
  unordered_map<string, Dart_handle> index_1_cell;
  unordered_map<string, Dart_handle> index_2_cell;

public:
  Point tvec_to_point(TVec3d v)
  {
    return Point(v.x, v.y, v.z);
  }

  Point json_to_point(nlohmann::json p)
  {
    if (cityModel.find("transform") != cityModel.end())
    {
      nlohmann::json scale, translate;
      scale = cityModel["transform"]["scale"];
      translate = cityModel["transform"]["translate"];

      return Point((double)p[0] * (double)scale[0] + (double)translate[0], (double)p[1] * (double)scale[1] + (double)translate[1], (double)p[2] * (double)scale[2] + (double)translate[2]);
    }

    return Point(p[0], p[1], p[2]);
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
    st << p.x() << "-" << p.y() << "-" << p.z();

    //st << round_by(p.x(), precision) << "-" << round_by(p.y(), precision) << "-" << round_by(p.z(), precision);

    return st.str();
  }

  string get_point_name(TVec3d v)
  {
    return get_point_name(tvec_to_point(v));
  }

  string index_to_string(unordered_map<string, Dart_handle>::iterator it)
  {
    ostringstream str;

    str << it->first << ": ";
    if (it->second == nullptr)
      str << "IS NULL" << endl;
    else
      str << lcc.point(it->second) << endl;

    return str.str();
  }

  void show_null_index_records()
  {
#ifdef DEBUG
    for( unordered_map<string, Dart_handle>::iterator it = index_1_cell.begin(); it != index_1_cell.end(); it++)
    {
      if (it->second == NULL)
      {
        log_str << "NOW " << it->first << " IS NULL!!!!!" << endl;
      }
    }
#endif // DEBUG
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

  Dart_handle add_vertex(Point v, int i_free = -1)
  {
    Dart_handle result;
    bool found = false;
    string v_name = get_point_name(v);
    if (index_0_cell.find(v_name) != index_0_cell.end())
    {
      for (vector<Dart_handle>::iterator it = index_0_cell[v_name].begin(); it != index_0_cell[v_name].end(); ++it)
      {
        if (i_free < 0 || lcc.beta(*it, i_free) == lcc.null_dart_handle)
        {
          found = true;
          result = *it;

          index_0_cell[v_name].erase(it);
          if (index_0_cell[v_name].empty())
          {
            index_0_cell.erase(v_name);
          }

          break;
        }
      }
    }

    if( !found )
    {
      result = lcc.create_dart( v );
      index_0_cell[v_name].push_back(result);
      // log_str << "Created " << lcc.point(result) << endl;
    }

    return result;
  }

  Dart_handle add_edge(Point v1, Point v2)
  {
    Dart_handle result;

    result = add_vertex(v1, 1);
    Dart_handle temp_dart = add_vertex(v2, 0);

    lcc.sew<1>(result, temp_dart);

    string edge_v1_v2_name = get_edge_name(v1, v2);
    string edge_v2_v1_name = get_edge_name(v2, v1);

    index_1_cell[edge_v1_v2_name] = result;

    if (index_1_cell.find(edge_v2_v1_name) != index_1_cell.end())
    {
      lcc.sew<2>(result, index_1_cell[edge_v2_v1_name]);

      index_1_cell.erase(edge_v1_v2_name);
      index_1_cell.erase(edge_v2_v1_name);
    }

    show_null_index_records();

    return result;
  }

  vector<Point> parse_vertices(nlohmann::json vertices)
  {
    vector<Point> verts;
    for (int i : vertices)
    {
      verts.push_back(json_to_point(cityModel["vertices"][i]));
    }

    return verts;
  }

  vector<Dart_handle> parse_polygon(nlohmann::json poly, ostringstream &stream, int level = 1)
  {
    vector<Dart_handle> result;

    stream << "+" << string(level * 2 - 2, '-') << " Polygon" << endl;

    // TODO: Add support for holes
    const vector<Point> verts = parse_vertices(poly[0]);

    if (verts.size() > 2)
    {
      for(auto it = verts.begin(); (it + 1) != verts.end(); it++)
      {
        if (get_point_name(*it) != get_point_name(*(it + 1)))
        {
          result.push_back(add_edge(*it, *(it + 1)));
        }
      }

      if (get_point_name(verts.back()) != get_point_name(*verts.begin()))
      {
        result.push_back(add_edge(verts.back(), *verts.begin()));
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

  vector<Dart_handle> parse_geometry(nlohmann::json geom, ostringstream &stream, int level = 1)
  {
    vector<Dart_handle> result;

    stream << string(level * 2 - 1, '-') << " Geometry (" << geom["type"] << ")" << endl;

    if (lod_filter > 0 && geom.find("lod") != geom.end() && geom["lod"] != lod_filter)
    {
      stream << "Skipping LoD " << geom["lod"] << " because of LoD" << lod_filter << " filter!" << endl;
      return result;
    }

    if (geom["type"] == "Solid")
    {
      stream << "|" << string(level * 2 - 1, '-') << " Solid count: " << geom["boundaries"].size() << endl;
      for (auto& solid : geom["boundaries"])
      {
        for (auto& polygon : solid)
        {
          auto temp_darts = parse_polygon( polygon, stream, level + 1 );
          result.insert(result.end(), temp_darts.begin(), temp_darts.end());
        }
      }
    }
    else if (geom["type"] == "MultiSurface")
    {
      stream << "|" << string(level * 2 - 1, '-') << " Polygon count: " << geom["boundaries"].size() << endl;
      for (auto& polygon : geom["boundaries"])
      {
        auto temp_darts = parse_polygon( polygon, stream, level + 1 );
        result.insert(result.end(), temp_darts.begin(), temp_darts.end());
      }
    }

    return result;
  }

  string parse_object(pair<const string, nlohmann::json> &obj)
  {
    ostringstream string_stream;

    string_stream << "Object " << obj.first << endl;
    string_stream << "---------------------" << endl;

    auto obj_content = obj.second;

    string_stream << "Type: " << obj_content["type"] << endl;
    string_stream << "Geometry count: " << obj_content["geometry"].size() << endl;

    for (auto& geom : obj_content["geometry"])
    {
      vector<Dart_handle> darts = parse_geometry( geom, string_stream );

      for (vector<Dart_handle>::iterator it = darts.begin(); it != darts.end(); ++it)
      {
        init_volume(*it);
        lcc.info<3>(*it).set_guid(obj.first);
        // TODO: Add attributes assignment
        // set_attributes(*it, obj_content["attributes"]);
      }
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

  void set_attributes(Dart_handle dh, nlohmann::json attributes)
  {
    // TODO: Make this more flexible
    // lcc.info<3>(dh).set_attributes(attributes);
  }

  LCC readCityModel(nlohmann::json city)
  {
    cityModel = city;

    map<string, nlohmann::json> objs = city["CityObjects"];
    int obj_count = objs.size();

    if (object_limit == 0)
    {
      object_limit = obj_count;
    }

    if (start_i + object_limit > objs.size())
    {
      object_limit = obj_count - start_i;
    }

    unsigned int i = 0;
    for (auto& obj : objs)
    {
      if (!id_filter.empty())
      {
        size_t pos = obj.first.find(id_filter);
        if (pos == string::npos)
        {
          continue;
        }
      }

      log_str << parse_object(obj);

      log_str << i << ") ";
#ifdef DEBUG
      lcc.display_characteristics(log_str);
#endif
      log_str << endl << endl;

      if (index_1_per_object) {
        index_1_cell.clear();
      }

      cout << "\rDone with " << i - start_i + 1 << "/" << object_limit;
      cout.flush();

      i++;
      if (i + 1 > object_limit)
      {
        break;
      }
    }

    cout << endl;

    init_all_volumes();

    return lcc;
  }

  void setStartingIndex(unsigned int start_index)
  {
    start_i = start_index;
  }

  unsigned int getStartingIndex()
  {
    return start_i;
  }

  void setObjectLimit(unsigned int new_limit)
  {
    object_limit = new_limit;
  }

  unsigned int getObjectLimit()
  {
    return object_limit;
  }

  void setIdFilter(string filter)
  {
    id_filter = filter;
  }

  void setLodFilter(int lod)
  {
    lod_filter = lod;
  }

  string getLog()
  {
    return log_str.str();
  }

  string getIndex()
  {
    ostringstream str;

    str << "This is the final status of the 1-cell index" << endl << "--------" << endl;
    for(unordered_map<string, Dart_handle>::iterator it=index_1_cell.begin(); it!=index_1_cell.end(); ++it)
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
