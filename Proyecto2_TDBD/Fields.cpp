#include "Field.h"
#pragma warning(disable : 4996)

Field::Field()
{

}

Field::Field(char name[20], char datatype[20] ,char size[20])
{
	strcpy(this->name,name);
	strcpy(this->datatype, datatype);
	strcpy(this->size, size);
}