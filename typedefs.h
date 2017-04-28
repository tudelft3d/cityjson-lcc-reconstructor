#ifndef TYPEDEFS_H
#define TYPEDEFS_H

#include <citygml.h>
#include <citymodel.h>
#include <cityobject.h>
#include <geometry.h>
#include <polygon.h>

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>

#include <CGAL/Linear_cell_complex.h>
#include <CGAL/Linear_cell_complex_constructors.h>
#include <CGAL/Linear_cell_complex_operations.h>
#include <CGAL/Combinatorial_map_save_load.h>

#include <CGAL/IO/Color.h>
#include <CGAL/Timer.h>

// Use to define properties on volumes.
#define LCC_DEMO_VISIBLE 1 // if not visible => hidden
#define LCC_DEMO_FILLED  2 // if not filled, wireframe

class Volume_info
{
  friend void CGAL::read_cmap_attribute_node<Volume_info>
  (const boost::property_tree::ptree::value_type &v,Volume_info &val);

  friend void CGAL::write_cmap_attribute_node<Volume_info>(boost::property_tree::ptree & node,
                                                           const Volume_info& arg);
public:
  Volume_info() : m_color(CGAL::Color(255, 255, 255)),
    m_status( LCC_DEMO_VISIBLE | LCC_DEMO_FILLED )
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

  void negate_visible()
  { set_visible(!is_visible()); }
  void negate_filled()
  { set_filled(!is_filled()); }

private:
  CGAL::Color m_color;
  char        m_status;
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
}

// Definition of function allowing to save custon information.
template<>
inline void write_cmap_attribute_node<Volume_info>(boost::property_tree::ptree & node,
                                                   const Volume_info& arg)
{
	std::cout<<"CALLED"<<std::endl;
  boost::property_tree::ptree & nValue = node.add("v","");
  nValue.add("status",(int)arg.m_status);
  nValue.add("color-r",(int)arg.m_color.r());
  nValue.add("color-g",(int)arg.m_color.g());
  nValue.add("color-b",(int)arg.m_color.b());
}

}

class Myitems
{
public:
  template < class Refs >
  struct Dart_wrapper
  {
    typedef CGAL::Cell_attribute_with_point< Refs > Vertex_attrib;
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