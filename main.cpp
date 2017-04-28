#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <stdio.h>

#include <boost/foreach.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/lexical_cast.hpp>

#include "typedefs.h"

#include "citygml_reader.h"

using namespace std;

void show_help()
{
	cout << "CityGML to Linear Complex Cell converter (for Combinatorial Map)" << endl;
	cout << "============" << endl;
}

int main(int argc, char *argv[])
{	
	if (argc == 1)
	{
		show_help();
		return 0;
	}

	// Load the CityGML model
	citygml::ParserParams params;
	const char *filename = argv[1];
	const char *out_filename = "";
	const char *off_filename = "";
	bool show_log = false;

	shared_ptr<const citygml::CityModel> city = citygml::load(filename, params);
	cout << "We found " << city->getNumRootCityObjects() << " root city objects!" << endl << endl;

	// Initialize the CityGML reader
	CityGmlReader reader;
	for (int i = 2; i < argc; ++i)
	{
		if (string(argv[i]) == "-o") {
			out_filename = argv[++i];
			cout << " - Will export as " << out_filename << endl;
		}
		else if (string(argv[i]) == "-off") {
			off_filename = argv[++i];
			cout << " - Will export off file as " << off_filename << endl;
		}
		else if (string(argv[i]) == "-s") {
			reader.setStartingIndex(atoi(argv[++i]));
			cout << " - Will start from index " << reader.getStartingIndex() << endl;
		}
		else if (string(argv[i]) == "-c") {
			reader.setObjectLimit(atoi(argv[++i]));
			cout << " - Will only export " << reader.getObjectLimit() << " objects" << endl;
		}
		else if (string(argv[i]) == "-p") {
			reader.setPrecision(atoi(argv[++i]));
			cout << " - Will work with precision of " << reader.getPrecision() << " digits" << endl;
		}
		else if (string(argv[i]) == "-l")
		{
			show_log = true;
		}
	}

	LCC lcc = reader.readCityModel(city);

	if (out_filename != NULL && out_filename[0] != '\0')
	{
		save_combinatorial_map(lcc, out_filename);
	}

	if (off_filename != NULL && off_filename[0] != '\0')
	{
		write_off(lcc, off_filename);
	}

	if (show_log)
	{
		cout << reader.getLog();
		cout << reader.getIndex();
	}

	return 0;
}