#pragma once

#include "Field.h"
#include <string>
#include <vector>

using namespace std;

class Table {
public:
	char name[20];
	vector<Field> fields;

	Table();
	Table(char[20]);
};