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
#include "cityjson_reader.h"

using namespace std;

void show_help()
{
	cout << "CityGML to Linear Complex Cell converter (for Combinatorial Map)" << endl;
	cout << "============" << endl;
	cout << "Usage: citygml input_file.json [options]" << endl;
	cout << "	options:" << endl;
	cout << "		-o [output_file.3map]	Export the C-Map as 3map file" << endl;
	cout << "		-off [output_file.off]	Export the C-Map as off file" << endl;
	cout << "		-p [precision]		Use the provided number of decimal digits for comparing coordinates" << endl;
	cout << "		-s [starting_index]	Start from the provided index" << endl;
	cout << "		-c [count]		Process only the provided number of city objects" << endl;
	cout << "		-f [filter]		Process only objects where the id matches the provided filter" << endl;
	cout << "		-n [new_cityjson.json]	Save the city model in a new CityJSON appended with the C-Map" << endl;
	cout << "		-i			Clear the 2-free index after every city object" << endl;
	cout << "		-l			Show log in standard output" << endl;
}

void append_cityjson(nlohmann::json& city, LCC lcc)
{
	nlohmann::json darts;

	std::map<LCC::Dart_const_handle, LCC::size_type> myDarts;

	// First we numbered each dart by using the std::map.
    LCC::Dart_range::const_iterator it(lcc.darts().begin());
    for(LCC::size_type num=1; num<=lcc.number_of_darts(); ++num, ++it)
    {
      	myDarts[it] = num;
    }

	// Now we save each dart, and its neighbors.
	it=lcc.darts().begin();
    for(LCC::size_type num=0; num<lcc.number_of_darts(); ++num, ++it)
    {
		// make a dart
		nlohmann::json new_dart;

		nlohmann::json betas;
		// the beta, only for non free sews
		for(unsigned int dim=1; dim<=lcc.dimension; dim++)
		{
			if(!lcc.is_free(it, dim))
			{
				betas.push_back(myDarts[lcc.beta(it, dim)]);
			}
			else
			{
				betas.push_back(-1);
			}
		}

		new_dart["betas"] = betas;
		new_dart["parent"] = lcc.info<3>(it).get_guid();

		darts.push_back(new_dart);
    }

	city["darts"] = darts;
}

int main(int argc, char *argv[])
{	
	if (argc == 1)
	{
		show_help();
		return 0;
	}

	// Load the CityJSON model
	citygml::ParserParams params;
	const char *filename = argv[1];
	const char *out_filename = "";
	const char *off_filename = "";
	const char *cityjson_filename = "";
	const char *id_filter = "";
	bool show_log = false;

	ifstream input_file(filename);
	nlohmann::json city_model;
	input_file >> city_model;

	cout << "We found " << city_model["CityObjects"].size() << " root city objects!" << endl << endl;

	// Initialize the CityJSON reader
	CityJsonReader reader;
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
		else if (string(argv[i]) == "-n")
		{
			cityjson_filename = argv[++i];
			cout << " - Will save the city model as " << cityjson_filename << endl;
		}
		else if (string(argv[i]) == "-s") {
			reader.setStartingIndex(static_cast<unsigned int>(atoi(argv[++i])));
			cout << " - Will start from index " << reader.getStartingIndex() << endl;
		}
		else if (string(argv[i]) == "-c") {
			reader.setObjectLimit(static_cast<unsigned int>(atoi(argv[++i])));
			cout << " - Will only export " << reader.getObjectLimit() << " objects" << endl;
		}
		else if (string(argv[i]) == "-p") {
			reader.setPrecision(atoi(argv[++i]));
			cout << " - Will work with precision of " << reader.getPrecision() << " digits" << endl;
		}
		else if (string(argv[i]) == "-f") {
			reader.setFilter(argv[++i]);
			cout << " - Will only process objects with id containing '" << id_filter << "'" << endl;
		}
		else if (string(argv[i]) == "-i") {
			reader.setIndexPerObject(true);
			cout << " - Will only keep the 2-free index per city object" << endl;
		}
		else if (string(argv[i]) == "-l")
		{
			show_log = true;
		}
	}

	LCC lcc = reader.readCityModel(city_model);

	if (out_filename != nullptr && out_filename[0] != '\0')
	{
		save_combinatorial_map(lcc, out_filename);
	}

	if (off_filename != nullptr && off_filename[0] != '\0')
	{
		write_off(lcc, off_filename);
	}

	if (cityjson_filename != nullptr && cityjson_filename[0] != '\0')
	{
		append_cityjson(city_model, lcc);

		ofstream output_file(cityjson_filename);
		output_file << city_model;
	} 

	if (show_log)
	{
		cout << reader.getLog();
		cout << reader.getIndex();
	}

	return 0;
}
