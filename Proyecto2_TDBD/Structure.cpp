#include "Structure.h"
#include <fstream>
#include <iostream>

void Structure::create_structure(string path)
{
	ifstream file(path, ios::beg | ios::binary);
	
	if (!file) {
		cout << "Ha surgido un error, la estructura no pudo ser creada." << endl;
		return;
	}

	file.seekg(1032);

}
