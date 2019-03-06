#include "DataBase.h"
#include <iostream>
#include <fstream>

#pragma warning(disable : 4996)

using namespace std;

DataBase::DataBase()
{

}

DataBase::DataBase(char name[20], int num_blocks) 
{
	strcpy(this->name, name);
	this->num_blocks = num_blocks;
}
