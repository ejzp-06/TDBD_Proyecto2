#include "Table.h"

#pragma warning(disable : 4996)

Table::Table()
{

}

Table::Table(char name[20])
{
	strcpy(this->name, name);
}
