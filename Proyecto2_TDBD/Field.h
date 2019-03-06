#pragma once
#include <string>

using namespace std;

class Field {
public:
	char name[20];
	char datatype[20];
	char size[20]; //if varchar

	Field();
	Field(char[20], char[20] ,char[20]);
};