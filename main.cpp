#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <CGAL/Linear_cell_complex_for_combinatorial_map.h>
#include <stdio.h>

#include "typedefs.h"

#include "citygml_reader.h"

using namespace std;

int main(int argc, char *argv[])
{	
	// Load the CityGML model
	citygml::ParserParams params;
	const char *filename = argv[1];

	shared_ptr<const citygml::CityModel> city = citygml::load(filename, params);
	cout << "We found " << city->getNumRootCityObjects() << " root city objects!" << endl << endl;

	// Initialize the CityGML reader
	CityGmlReader reader;
	if (argc > 3)
	{
		reader.setObjectLimit(atoi(argv[3]));
	}

	LCC lcc = reader.readCityModel(city);

	if (argc > 2)
	{
		const char *out_filename = argv[2];
		save_combinatorial_map(lcc, out_filename);
	}

	return 0;
}
