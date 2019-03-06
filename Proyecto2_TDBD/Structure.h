#pragma once
#include "Table.h"
#include <vector>

class Structure {
	vector<Table> tables;

	Structure();
	void create_structure(string path);
};
