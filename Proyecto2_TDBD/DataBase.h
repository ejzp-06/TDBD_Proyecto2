#pragma once
#include "Datablock.h"
#include <vector>
#include <string>

using namespace std;

class DataBase {

public:
	char name[20];
	int num_blocks;
	
	//constructor
	DataBase(char[20],int);
	DataBase();
};



