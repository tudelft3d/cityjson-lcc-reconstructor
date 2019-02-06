#ifndef TYPEDEFS_H
#define TYPEDEFS_H

#include "thirdparty/json.hpp"

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>

#include <CGAL/Linear_cell_complex.h>
#include <CGAL/Linear_cell_complex_constructors.h>
#include <CGAL/Linear_cell_complex_operations.h>
#include <CGAL/Combinatorial_map_save_load.h>

#include <CGAL/IO/Color.h>
#include <CGAL/Timer.h>
#include <stdlib.h>

// Use to define properties on volumes.
#define LCC_DEMO_VISIBLE 1 // if not visible => hidden
#define LCC_DEMO_FILLED  2 // if not filled, wireframe

class Vertex_info
{
public:
  Vertex_info() : m_vertex(0)
  {}

  const unsigned long vertex() const
  {
    return m_vertex;
  }

  void set_vertex(unsigned long i)
  {
    m_vertex = i;
  }

private:
  unsigned long m_vertex = 0;
};

class Volume_info
{
  friend void CGAL::read_cmap_attribute_node<Volume_info>
  (const boost::property_tree::ptree::value_type &v,Volume_info &val);

  friend void CGAL::write_cmap_attribute_node<Volume_info>(boost::property_tree::ptree & node,
                                                           const Volume_info& arg);
public:
  Volume_info() : m_color(CGAL::Color(rand() % 256, rand() % 256, rand() % 256)),
    m_status( LCC_DEMO_VISIBLE | LCC_DEMO_FILLED ),
    m_guid("nothing")
  {}

  CGAL::Color& color()
  { return m_color; }
  const CGAL::Color& color() const
  { return m_color; }

  std::string color_name() const
  {
    std::ostringstream ss;
    ss<<std::setfill('0');
    ss<<"#"<<std::hex<<std::setw(2)<<(int)m_color.red()
     <<std::setw(2)<<(int)m_color.green()<<std::setw(2)<<(int)m_color.blue();
    return ss.str();
  }

  bool is_visible() const
  { return (m_status & LCC_DEMO_VISIBLE)!=0; }
  bool is_filled() const
  { return (m_status & LCC_DEMO_FILLED)!=0; }
  bool is_filled_and_visible() const
  { return is_filled() && is_visible(); }

  void set_visible(bool val=true)
  {
    if ( is_visible()==val ) return;
    if ( val ) m_status = m_status | LCC_DEMO_VISIBLE;
    else       m_status = m_status ^ LCC_DEMO_VISIBLE;
  }
  void set_filled(bool val=true)
  {
    if ( is_filled()==val ) return;
    if ( val ) m_status = m_status | LCC_DEMO_FILLED;
    else       m_status = m_status ^ LCC_DEMO_FILLED;
  }

  void set_guid(std::string guid)
  {
  	m_guid = guid;
  }

  std::string get_guid() const
  {
  	return m_guid;
  }

  void set_attributes(std::map<std::string, std::string> attributes)
  {
      m_attributes = attributes;
  }

  std::map<std::string, std::string> get_attributes()
  {
      return m_attributes;
  }

  void negate_visible()
  { set_visible(!is_visible()); }
  void negate_filled()
  { set_filled(!is_filled()); }

private:
  CGAL::Color m_color;
  char        m_status;
  std::string m_guid;
  std::map<std::string, std::string> m_attributes;
};

namespace CGAL
{

template<>
inline void read_cmap_attribute_node<Volume_info>
(const boost::property_tree::ptree::value_type &v,Volume_info &val)
{
  try
  {
    val.m_status = v.second.get<int>("status");
  }
  catch(const std::exception &  )
  {}

  try
  {
    char r = v.second.get<int>("color-r");
    char g = v.second.get<int>("color-g");
    char b = v.second.get<int>("color-b");
    val.m_color = CGAL::Color(r,g,b);
  }
  catch(const std::exception &  )
  {}

  try
  {
  	val.m_guid = v.second.get<std::string>("guid");
  }
  catch(const std::exception &  )
  {}

  try
  {
    boost::property_tree::ptree pt = v.second.get_child("attributes");
    for(boost::property_tree::ptree::iterator iter = pt.begin(); iter != pt.end(); iter++)
    {
      val.m_attributes[iter->first] =  iter->second.data();
    }
  }
  catch(const std::exception &  )
  {
    
  }
}

// Definition of function allowing to save custon information.
template<>
inline void write_cmap_attribute_node<Volume_info>(boost::property_tree::ptree & node,
                                                   const Volume_info& arg)
{
  boost::property_tree::ptree & nValue = node.add("v","");
  nValue.add("status",(int)arg.m_status);
  nValue.add("color-r",(int)arg.m_color.r());
  nValue.add("color-g",(int)arg.m_color.g());
  nValue.add("color-b",(int)arg.m_color.b());
  nValue.add("guid",arg.m_guid);
  for (auto it = arg.m_attributes.begin(); it != arg.m_attributes.end(); ++it)
  {
      nValue.add("attributes." + it->first, it->second);
  }
}

}

class Myitems
{
public:
  template < class Refs >
  struct Dart_wrapper
  {
    typedef CGAL::Cell_attribute_with_point< Refs, Vertex_info > Vertex_attrib;
    typedef CGAL::Cell_attribute< Refs, Volume_info> Volume_attrib;

    typedef CGAL::cpp11::tuple<Vertex_attrib,void,void,
                               Volume_attrib> Attributes;
  };
};

typedef CGAL::Linear_cell_complex_traits
<3,CGAL::Exact_predicates_inexact_constructions_kernel> Mytraits;

typedef CGAL::Linear_cell_complex_for_combinatorial_map<3,3,Mytraits,Myitems> LCC;

typedef LCC::Dart_handle Dart_handle;
typedef LCC::Point Point;
typedef LCC::Vector Vector;
typedef LCC::FT FT;

#endif
