#include "FileManagment.h"
#include <fstream>
#include <iostream>

#pragma warning(disable : 4996)

//TODO: HACER LA FUNCION SELECT 
//TODO: MOSTRAR TODAS LAS TABLAS AL MOMENTO DE INGRESAR, ACTUALIZAR O ELIMINAR REGISTROS
//TODO: IMPLEMENTAR LLAVES PK Y TODO LO QUE ESTO INCLUYE
//TODO: HACER EL DROP DATABASE

bool eliminados = false;
vector<char*> v;


FileManagement::FileManagement()
{

}


//funciones para la creacion de tablas
void FileManagement::create_table(Table table, string path)
{
	Table tmp = table;

	fstream file(path, ios::in | ios::out | ios::binary);

	if (!file) {
		cout << "Ha surgido un error, la tabla no pudo ser creada.\n";
		return;
	}

	int index = search_freeBlock(path);
	make_blockBusy(path,index);
	file.seekp(index*sizeof(DataBlock)-sizeof(DataBlock));
	file.write(tmp.name,20);

	for (int i = 0; i < tmp.fields.size(); i++)
	{
		file.write(tmp.fields[i].name,20);
		file.write(tmp.fields[i].datatype,20);
		if (strcmp(tmp.fields[i].datatype, "char") == 0){
			file.write(tmp.fields[i].size, 20);
		}
	}

	file << "/"; //final de metadata
	file << "|"; //representa el comienzo de los registros
	file.close();

	new_sysRegister(path, table, 2, index);
}

void FileManagement::list_allTables(string path)
{
	fstream file(path, ios::in | ios::out | ios::binary);

	if (!file) {
		cout << "Ha surgido un error, la tabla no pudo ser creada.\n";
		return;
	}
	
	char tableName[20];
	char index[20];
	char end = ';';
	int firstRegister = find_firstRegistry(path, 2);
	file.seekg(firstRegister);
	
	while (end != '|')
	{

		file.read(reinterpret_cast<char*>(tableName), 20);
		file.read(reinterpret_cast<char*>(index), 20);

		if(tableName[0]!='Ì')
			cout << tableName << endl;
		
		file.read(&end, 1);

		if (end == '|')
			break;

		int pos = file.tellp();
		file.seekp(pos - 1);

	}


}



//funciones para la eliminacion de tablas
void FileManagement::delete_table(string path, int index, int sysIndex,char table_name[20])
{
	fstream file(path, ios::in | ios::out | ios::binary);

	if (!file) {
		cout << "Ha surgido un error, la tabla no pudo ser creada.\n";
		return;
	}

	file.seekp(index * sizeof(DataBlock) - sizeof(DataBlock));

	DataBlock tmp;

	DataBlock datablock;
	file.read(reinterpret_cast<char*>(&tmp), sizeof(DataBlock));

	int pos = file.tellp();
	file.seekp(pos - sizeof(DataBlock));
	file.write(reinterpret_cast<char*>(&datablock), sizeof(DataBlock));

	if (tmp.next==-1) {
		delete_sysRegister(path, table_name, sysIndex);
		file.close();
		return;
	}
	else {
		delete_table(path, tmp.next, sysIndex ,table_name);
	}

}

void FileManagement::delete_sysRegister(string path, char table_name[100], int index_)
{
	fstream file(path, ios::in | ios::out | ios::binary);

	if (!file) {
		cout << "Ha surgido un error, la tabla no pudo ser creada.\n";
		return;
	}

	Table table = read_tableData(path, 2);
	char identifier[20];
	strcpy(identifier, (char*)"nombreTabla");
	int index = find_registerIndex(path, 2, identifier , table, table_name);
	
	char trashData[10000];
	file.seekp(index);

	int registerSize = get_registerSize(table);

	if (isLast_Register(path, 2, table_name,table_name,table)) {
		trashData[0] = '|';
	}
	

	file.write(reinterpret_cast<char*>(trashData), registerSize);

	file.close();

}


//funciones para el ingreso de registros
void FileManagement::new_register(string path, int index, int tmpindex)
{
	fstream file(path, ios::in | ios::out | ios::binary);

	if (!file) {
		return;
	}

	if (tmpindex == -1) {
		return;
	}
	
	Table table = read_tableData(path, index);

	if (!fills_dataBlock(path, tmpindex, get_registerSize(table)))
	{
		file.seekp(find_lastRegister(path, tmpindex));
		for (int i = 0; i < table.fields.size(); i++)
		{
			char field[4000];
			cout << "Ingrese el campo \"" + (string)table.fields[i].name + "\" con tipo de dato (" + (string)table.fields[i].datatype + "):";
			cin >> field;
			if (strcmp(table.fields[i].datatype, (char*)"char") == 0) {
				int size = atoi(table.fields[i].size);
				file.write(field, size);
			}
			else {
				file.write(field, 20);
			}
		}
		file << "|";
		file.close();
	}else
	{
		if (!hasNext(path, tmpindex)) {
			int next = assing_nextBlock(path, tmpindex);
			file.seekp(find_lastRegister(path, next));

			for (int i = 0; i < table.fields.size(); i++)
			{
				char field[4000];
				cout << "Ingrese el campo \"" + (string)table.fields[i].name + "\" con tipo de dato (" + (string)table.fields[i].datatype + "):";
				cin >> field;
				if (strcmp(table.fields[i].datatype, (char*)"char") == 0) {
					int size = atoi(table.fields[i].size);
					file.write(field, size);
				}
				else {
					file.write(field, 20);
				}
			}

			file << "|";
			file.close();
		}
		else {
			DataBlock datablock;
			file.seekg(tmpindex * sizeof(DataBlock) - sizeof(DataBlock));
			file.read(reinterpret_cast<char*>(&datablock), sizeof(DataBlock));
			new_register(path, index,datablock.next);
		}

	}

	file.close();

}

//funciones para la elmininacion de registros
int FileManagement::delete_register(string path, int index, char identifier[100],char filter[100],Table table)
{
	fstream file(path, ios::in | ios::out | ios::binary);

	if (!file) {
		cout << "Ha surgido un error, la tabla no pudo ser creada.\n";
		return -1;
	}

	
	int registerSize = get_registerSize(table);

	

	cout << endl;
	

	if (has_noRegisters(path, index)) {
		if (hasNext(path, index)) {
			DataBlock datablock;
			file.seekg(index * sizeof(DataBlock) - sizeof(DataBlock));
			file.read(reinterpret_cast<char*>(&datablock), sizeof(DataBlock));
			delete_register(path, datablock.next,identifier,filter,table);
			return-1;
		}
		else {
			return-1;
		}
	}


	int registerIndex = find_registerIndex(path, index, identifier, table, filter);

	if (registerIndex != -1) {
		eliminados = true;
	}
	else {
		return -1;
	}

	file.seekp(registerIndex);

	char value[4000];
	char trashData[10000];

	strcpy(value, filter);
	 
	if (isLast_Register(path, index, identifier,value,table))
		trashData[0] = '|';

	file.write(trashData, registerSize);
	return 1;
}

void FileManagement::delete_allRegisters(string path, int index_)
{

	fstream file(path, ios::in | ios::out | ios::binary);

	if (!file) {
		return;
	}
	
	if (has_noRegisters(path, index_)) {
		return;
	}

	file.seekp(find_firstRegistry(path, index_));
	int firstRegiser = find_firstRegistry(path, index_);
	int lastRegiser = find_lastRegister(path, index_);

	int writingSize = lastRegiser - firstRegiser;

	char trashData[4096];
	file.write(reinterpret_cast<char*>(trashData), writingSize);
	int index = file.tellg();
	file.seekp(index - writingSize);
	file << "|";

	if (hasNext(path, index_)) {
		DataBlock datablock;
		file.seekg(index_* sizeof(DataBlock) - sizeof(DataBlock));
		file.read(reinterpret_cast<char*>(&datablock), sizeof(DataBlock));
		delete_allRegisters(path, datablock.next);
	}

	file.close();
}


//funciones para la actualizacion de registros
void FileManagement::update_register(string path)
{
	fstream file(path, ios::in | ios::out | ios::binary);

	if (!file) {
		//cout << "Ha surgido un error, la tabla no pudo ser creada.\n";
		return;
	}

	char name[20];
	cout << "Ingrese el nombre de la tabla donde se actualizaran datos: ";
	cin >> name;

	int index = get_tableIndex(path, name);
	if (index == -1) {
		cout << "\nLa tabla donde usted quiere actualizar datos no existe." << endl;
		system("pause");
		return;
	}
	else {
		cout << endl;
	}

	if (has_noRegisters(path, index)) {
		cout << "Esta tabla no contiene registros." << endl;
		system("Pause");
		return;
	}

	Table table = read_tableData(path, index);

	cout << "\nCampos de la tabla " + (string)table.name + ": " << endl;
	for (int i = 0; i < table.fields.size(); i++)
	{
		cout << (string)table.fields[i].name << endl;;
	}
	cout << endl;
	char identifier[20];
	char filter[100];
	cout << "Eliga el campo que usara para identificar el archivo: ";
	cin >> identifier;
	cout << "Escriba el valor del campo que se utilizara para identificar al registro: ";
	cin >> filter;

	int register_index = find_registerIndex(path, index, identifier,table, filter);

	if (register_index == -1) {
		cout << "El registro no pudo ser encontrado." << endl;
		system("pause");
		return;
	}

	file.seekg(register_index);

	for (int i = 0; i < table.fields.size(); i++)
	{
		char field[4000];
		cout << "Ingrese el campo \"" + (string)table.fields[i].name + "\" con tipo de dato (" + (string)table.fields[i].datatype + "):";
		cin >> field;
		if (strcmp(table.fields[i].datatype, (char*)"char") == 0) {
			int size = atoi(table.fields[i].size);
			file.write(field, size);
		}
		else {
			file.write(field, 20);
		}
	}

	file.close();
	system("pause");



}

//funciones para la seleccion de registros
void FileManagement::select_allRegisters(string path, int index)
{
	fstream file(path, ios::in | ios::out | ios::binary);

	if (!file) {
		//cout << "Ha surgido un error, la tabla no pudo ser creada.\n";
		return;
	}

	if (has_noRegisters(path, index))
	{
		cout << "Esta tabla no tiene registros." << endl;
		return;
	}



	Table table = read_tableData(path, index);

	system("cls");
	cout << "Columnas: \n";
	for (int i = 0; i < table.fields.size(); i++)
	{
		cout << table.fields[i].name;
		cout << "\t\t\t";
	}
	cout << "\n";

	label:
	file.seekg(find_firstRegistry(path, index));	
	
	char end = ';';
	char data[4000];
	int contador = 0;

	while (end != '|')
	{

		if (contador == table.fields.size()) {
			contador = 0;
			cout << "\n";
		}

		file.read(&end, 1);
		if (end == '|')
			break;

		int pos = file.tellp();
		file.seekp(pos - 1);
		

		if (strcmp(table.fields[contador].datatype, (char*)"char") == 0)
			file.read(reinterpret_cast<char*>(data), atoi(table.fields[contador].size));
		else
			file.read(reinterpret_cast<char*>(data), 20);

		if (data[0] != 'Ì') {
			cout << data;
		}

		cout << "\t\t\t";	
		contador++;
	}

	if (hasNext(path, index)) {
		DataBlock datablock;
		file.seekp(index * sizeof(DataBlock) - sizeof(DataBlock));
		file.read(reinterpret_cast<char*>(&datablock), sizeof(DataBlock));
		index = datablock.next;
		goto label;
	}
	file.close();
	cout << "\n\n";
	system("pause");
}

void FileManagement::select_register(string path, char name[20])
{
	fstream file(path, ios::in | ios::out | ios::binary);

	if (!file) {
		//cout << "Ha surgido un error, la tabla no pudo ser creada.\n";
		return;
	}

	int index = get_tableIndex(path, name);

	if (index == -1) {
		cout << "La tabla no fue encontrada." << endl;
		system("pause");
		return;
	}

	if (has_noRegisters(path, index)) {
		cout << "Esta tabla no contiene registros." << endl;
		system("Pause");
		return;
	}

	char identifier[4000];
	char value[4000];
	char data[4000];
	Table table = read_tableData(path, index);
	cout << "\nCampos de la tabla " + (string)table.name + ": " << endl;
	for (int i = 0; i < table.fields.size(); i++)
	{
		cout << (string)table.fields[i].name << endl;;
	}

	cout << endl;
	cout << "Eliga el campo que usara para identificar el archivo: ";
	cin >> identifier;
	cout << "Escriba el valor del campo que se utilizara para identificar al registro: ";
	cin >> value;

	cout << "Columnas: " << endl;
	for (int i = 0; i < table.fields.size(); i++)
	{
		
		cout << table.fields[i].name << "\t" << "\t" << "\t";
	}

	cout << "\n";
	label:
	int firstRegister = find_firstRegistry(path, index);
	int registerSize = get_registerSize(table);
	int lastRegister = find_lastRegister(path, index);
	int readingSize = reading_Size(table, identifier);
	int identifierSize = get_identifierSize(table, identifier);
	file.seekg(firstRegister);
	if (readingSize != registerSize)
		file.read(data, readingSize);

	
	char end = ';';
	while (end != '|') 
	{
		file.read(&end, 1);
		if (end == '|')
			break;

		int pos = file.tellp();
		file.seekp(pos - 1);


		file.read(data, identifierSize);

		if (strcmp(data, value) == 0) {

			if (readingSize != registerSize) {

				int pos = file.tellg();
				file.seekg(pos - identifierSize - readingSize);
			}
			else {
				int pos = file.tellg();
				file.seekg(pos - identifierSize);
			}

			for (int i = 0; i < table.fields.size(); i++)
			{
				if (strcmp(table.fields[i].datatype, (char*)"char") == 0)
					file.read(reinterpret_cast<char*>(data), atoi(table.fields[i].size));
				else
					file.read(reinterpret_cast<char*>(data), 20);

				cout << data;
				cout << "\t\t\t";
			}
			cout << "\n";

			file.read(&end, 1);
			if (end == '|')
				break;

			int pos2 = file.tellp();
			file.seekp(pos2 - 1);

			int pos = file.tellg();
			if(readingSize!=registerSize)
				file.seekg(pos + readingSize);
		}
		else {
			file.read(&end, 1);
			if (end == '|')
				break;
			
			int pos2 = file.tellp();
			file.seekp(pos2 - 1);

			int pos3 = file.tellg();
			file.seekg(pos3 + registerSize - readingSize - identifierSize);

			file.read(&end, 1);
			if (end == '|')
				break;

			int pos4 = file.tellp();
			file.seekp(pos2 - 1);


			int pos = file.tellg();
			file.seekg(pos - identifierSize + registerSize);
		}
	}

	if (hasNext(path, index)) {
		DataBlock datablock;
		file.seekp(index * sizeof(DataBlock) - sizeof(DataBlock));
		file.read(reinterpret_cast<char*>(&datablock), sizeof(DataBlock));
		index = datablock.next;
		goto label;
	}

	cout << "\n";
	system("pause");
}

int FileManagement::reading_Size(Table table, char identifier[4000])
{
	int size = 0;

	for (int i = 0; i < table.fields.size(); i++)
	{
		if (strcmp(table.fields[i].name, identifier) == 0 && i == 0)
			return get_registerSize(table);
		else if (strcmp(table.fields[i].name, identifier) == 0)
			return size;
		else if (strcmp(table.fields[i].datatype, "char") == 0)
			size += atoi(table.fields[i].size);
		else if (strcmp(table.fields[i].datatype, "int") == 0 | strcmp(table.fields[i].datatype, "double") == 0)
			size += 20;
	}

	return get_registerSize(table);
}

int FileManagement::get_identifierSize(Table table, char identifier[20])
{
	for (int i = 0; i < table.fields.size(); i++)
	{
		if (strcmp(table.fields[i].name, identifier) == 0 && strcmp(table.fields[i].datatype, (char*)"char") ==0)
			return atoi(table.fields[i].size);
		else if (strcmp(table.fields[i].name, identifier) == 0 && (strcmp(table.fields[i].datatype, "int") == 0 | strcmp(table.fields[i].datatype, "double") == 0))
			return 20;
	}
	return 0;
}



//funciones para uso general
int FileManagement::get_tableIndex(string path, char name[20]) 
{
	//esta funcion se posiciona en el primer registro de la tabla del sistema
	// y llama a la funcion search_tableIndex.
	fstream file(path, ios::in | ios::out | ios::binary);

	if (!file) {
		//cout << "Ha surgido un error, la tabla no pudo ser creada.\n";
		return -1;
	}

	char pos = 'j';
	int index = (2 * sizeof(DataBlock) - sizeof(DataBlock));
	file.seekg(index);
	for (int i = 0; i < 4096; i++)
	{
		file.read(&pos, 1);
		if (pos == '/') {
			index = file.tellg();
			break;
		}
	}

	file.close();
	return search_tableIndex(path, name, index);
}

int FileManagement::search_tableIndex(string path, char name_[20], int index_)
{

	//esta funcion recibe de parametro la posicion del primer registro de la tabla del sistema
	// y busca el  nombre e index de la tabla que se esta tratando de encontrar

	fstream file(path, ios::in | ios::out | ios::binary);

	if (!file) {
		//cout << "Ha surgido un error, la tabla no pudo ser creada.\n";
		return -1;
	}

	file.seekg(index_);
	char end = 'a';
	char name[20] = "a";
	char index_table[20];
	int pos = 0;
	while (end != '|')
	{
		file.read(name, 20);
		file.read(index_table, 20);
		if (strcmp(name, name_) == 0) {
			file.close();
			return atoi(index_table);
		}

		file.read(&end, 1);
		if (end == '|')
			return -1;
		pos = file.tellg();
		file.seekg(pos - 1);
	}
	file.close();
	return -1;
}

int FileManagement::get_metadataSize(string path, int index_)
{
	//esta funcion lee la metadata y retorna el tamano en bytes que esta ocupa

	ifstream file(path, ios::beg | ios::in | ios::binary);

	if (!file) {
		//cout << "Ha surgido un error, la tabla no pudo ser creada.\n";
		return -1;
	}

	char reader[20];
	char pos = 'a';
	int index = (index_ * sizeof(DataBlock) - sizeof(DataBlock));
	file.seekg(index);
	int size = 0;
	for (int i = 0; i < 4096; i++)
	{
		file.read(reader, 20);
		file.read(&pos, 1);
		if (pos == '/') {
			int index = file.tellg();
			file.close();
			size += 21;
			return size;
		}
		else {
			int index = file.tellg();
			file.seekg(index - 1);
			size += 20;
		}

	}
	file.close();
	return 0;
}

Table FileManagement::read_tableData(string path, int index_)
{
	//esta funcion recibe de parametro el index de la tabla a leer
	//luego lee la metadata de dicha tabla, crea un objeto de tabla y la retorna 

	fstream file(path, ios::in | ios::out | ios::binary);

	if (!file) {
		//cout << "Ha surgido un error, la tabla no pudo ser creada.\n";
		return Table();
	}
	char end = 'a';
	char name[20];
	file.seekg(index_ * sizeof(DataBlock) - sizeof(DataBlock));
	file.read(name, 20);
	Table table(name);


	file.read(&end, 1);
	if (end == '/') //verifico si la tabla no tiene campos
		return Table(name);

	int index = file.tellg();
	file.seekg(index - 1);

	int i = 0;
	while (true)
	{
		if (i != 0) {
			file.read(&end, 1);

			if (end == '/')
				break;

			int index = file.tellg();
			file.seekg(index - 1);
		}

		Field field;
		int d = file.tellg();
		file.read(field.name, 20);
		file.read(field.datatype, 20);

		if (strcmp(field.datatype, (char*)"char") == 0)
			file.read(field.size, 20);


		table.fields.push_back(field);
		i++;

	}
	file.close();
	return table;
}

bool FileManagement::block_isFull(string path, int index)
{

	//verifica si un bloque en especifico esta lleno

	fstream file(path, ios::in | ios::out | ios::binary);

	if (!file) {
		return -1;
	}

	int delimeter_pos = find_lastRegister(path, index);
	int pos = delimeter_pos - (index - 1) * sizeof(DataBlock);
	return  pos == sizeof(DataBlock);
}

double FileManagement::get_registerCount(string path, int index, int registerSize, int metadataSize)
{
	return 0.0;
}



//manejo de registros
int FileManagement::get_registerSize(Table table)
{
	//calcula el tamano del registro de una tabla en especifico
	int size = 0;

	for (int i = 0; i < table.fields.size(); i++)
	{
		if (strcmp(table.fields[i].datatype, "char") == 0)
			size += atoi(table.fields[i].size);
		else if (strcmp(table.fields[i].datatype, "int") == 0 | strcmp(table.fields[i].datatype, "double") == 0)
			size += 20;
	}

	return size;
}

int FileManagement::find_firstRegistry(string path, int index_)
{
	//encuentra el primer registro de una tabla en especifico


	ifstream file(path, ios::beg | ios::in | ios::binary);

	if (!file) {
		//cout << "Ha surgido un error, la tabla no pudo ser creada.\n";
		return -1;
	}

	char pos = 'j';
	int index = (index_ * sizeof(DataBlock) - sizeof(DataBlock));
	file.seekg(index);
	for (int i = 0; i < 4096; i++)
	{
		file.read(&pos, 1);
		if (pos == '/') {
			int index = file.tellg();
			file.close();
			return index;
		}

	}
	file.close();
	return 0;
}

int FileManagement::find_registerIndex(string path, int index_, char identifier_[20], Table table, char value[100])
{
	//encuentra la posicion en bytes de un registro en especifico

	fstream file(path, ios::in | ios::out | ios::binary);

	if (!file) {
		//cout << "Ha surgido un error, la tabla no pudo ser creada.\n";
		return -1;
	}

	Table tmp = table;

	char identifier[4000];
	int index = find_firstRegistry(path, index_);
	int registerSize = get_registerSize(table);
	int lastRegister = find_lastRegister(path, index_);
	int readingSize = reading_Size(table, identifier_);
	int identifierSize = get_identifierSize(table, identifier_);
	file.seekg(index);
	if (readingSize != registerSize)
		file.read(identifier, readingSize);

	for (int i = 0; i < 4096; i++)
	{
		file.read(identifier, identifierSize);

		if (strcmp(identifier, value) == 0) {
			int index = file.tellg();
			file.close();
			                                                             //me cuesta  mucho explicar lo que pasa aca
			if (readingSize == registerSize)							 //pero funciona
				return index -= identifierSize;



			return index - readingSize - identifierSize;
		}
		else {
			int pos = file.tellg();
			file.seekg(pos - identifierSize + registerSize);
		}
	}

	if (hasNext(path,index_)) {
		file.close();
		file.open(path, ios::in | ios::out | ios::binary);
		DataBlock datablock;
		file.seekg(index_ * sizeof(DataBlock) - sizeof(DataBlock));
		file.read(reinterpret_cast<char*>(&datablock), sizeof(DataBlock));
		return find_registerIndex(path, datablock.next, identifier_, table, value);
	}

	return -1;
}

bool FileManagement::fills_dataBlock(string path, int index, int registerSize)
{
	//esta funcion se utiliza para determinar si al insertar un registro se llena el bloque

	fstream file(path, ios::in | ios::out | ios::binary);

	if (!file) {
		return -1;
	}

	int delimeterPos = find_lastRegister(path, index);
	int pos = delimeterPos - (index - 1) * sizeof(DataBlock);

	return registerSize >= sizeof(DataBlock)-pos;
}

int FileManagement::register_count(string path, int index)
{
	fstream file(path, ios::in | ios::out | ios::binary);

	if (!file) {
		return -1;
	}
}

bool FileManagement::isLast_Register(string path, int index_, char identifier[20], char value[4000], Table table)
{
	fstream file(path, ios::in | ios::out | ios::binary);

	if (!file) {
		//cout << "Ha surgido un error, la tabla no pudo ser creada.\n";
		return -1;
	}

	int index = find_lastRegister(path, index_);
	int RegisterSize = get_registerSize(table);
	int readingSize = reading_Size(table, identifier);
	int identifierSize = get_identifierSize(table, identifier);
	file.seekg(index - RegisterSize);

	char data[4000];
	if (readingSize == RegisterSize)
		file.read(reinterpret_cast<char*>(data), identifierSize);
	else if (readingSize != RegisterSize) {
		file.read(reinterpret_cast<char*>(data), readingSize);
		file.read(reinterpret_cast<char*>(data), identifierSize);
	}

	if (strcmp(data, value) == 0) {
		file.close();
		return true;
	}

	file.close();
	return false;
}

void FileManagement::split(char * data)
{
	char *tmp = strtok(data, "Ì");
	v.push_back(tmp);
}



//funciones utilizadas para el manejo de bloques
int FileManagement::search_freeBlock(string path)
{
	ifstream file(path, ios::beg | ios::in | ios::binary);

	if (!file) {
		//cout << "Ha surgido un error, la tabla no pudo ser creada.\n";
		return -1;
	}

	file.seekg(0, ios::end);
	int blocks = file.tellg() / 4096;
	file.seekg(0, ios::beg);

	for (int i = 0; i < blocks; i++)
	{
		DataBlock datablock;
		file.read(reinterpret_cast<char*>(&datablock), sizeof(DataBlock));
		if (datablock.free == 1) {
			return file.tellg() / 4096;
		}
	}
}

int FileManagement::find_lastRegister(string path, int index_)
{
	//esta funcion encuentra el final de los registros de una tabla especifica
	ifstream file(path, ios::beg | ios::in | ios::binary);

	if (!file) {
		//cout << "Ha surgido un error, la tabla no pudo ser creada.\n";
		return -1;
	}

	char pos = 'j';
	int index = (index_ * sizeof(DataBlock) - sizeof(DataBlock));
	file.seekg(index);
	for (int i = 0; i < 4096; i++)
	{
		file.read(&pos, 1);
		if (pos == '|') {
			int index = file.tellg();
			return index - 1;
		}

	}
	file.close();
	return 0;
}

void FileManagement::make_blockBusy(string path, int index)
{
	fstream file(path, ios::in | ios::out | ios::binary);

	if (!file) {
		//cout << "Ha surgido un error, la tabla no pudo ser creada.\n";
		return;
	}

	file.seekp(index * sizeof(DataBlock) - sizeof(DataBlock));

	DataBlock datablock;
	datablock.free = 0;
	int size = sizeof(DataBlock);
	file.write(reinterpret_cast<char*>(&datablock), size);
	file.close();
}

bool FileManagement::hasNext(string path, int index)
{
	fstream file(path, ios::in | ios::out | ios::binary);

	if (!file) {
		//cout << "Ha surgido un error, la tabla no pudo ser creada.\n";
		return false;
	}

	DataBlock datablock;
	file.seekg(index * sizeof(DataBlock) - sizeof(DataBlock));
	file.read(reinterpret_cast<char*>(&datablock), sizeof(DataBlock));
	file.close();
	return datablock.next != -1;
}

int FileManagement::assing_nextBlock(string path, int index)
{
	fstream file(path, ios::in | ios::out | ios::binary);

	if (!file) {
		return -1;
	}
	
	DataBlock datablock;
	file.seekg(index * sizeof(DataBlock) - sizeof(DataBlock));
	file.read(reinterpret_cast<char*>(&datablock), sizeof(DataBlock));

	//asigno el next al bloque
	int next = search_freeBlock(path);
	datablock.next = next;

	file.seekp(index * sizeof(DataBlock) - sizeof(DataBlock));
	file.write(reinterpret_cast<char*>(&datablock), sizeof(DataBlock));

	file.close();

	file.open(path, ios::in | ios::out | ios::binary);
	DataBlock tmp;
	tmp.free = 0;
	tmp.data[0] = '/';
	tmp.data[1] = '|';
	file.seekp(next * sizeof(DataBlock) - sizeof(DataBlock));
	file.write(reinterpret_cast<char*>(&tmp), sizeof(DataBlock));

	file.close();
	return next;
}

bool FileManagement::has_noRegisters(string path, int index_)
{
	fstream file(path, ios::in | ios::out | ios::binary);

	if (!file) {
		return -1;
	}

	int index = find_lastRegister(path, index_);
	file.seekg(index - 1);
	char delimeter = 'a';
	file.read(&delimeter, 1);
	if (delimeter == '/')
		return true;

	return false;

		
}


//manejo de bases de datos
bool FileManagement::database_exists(string path)
{
	ifstream file(path);

	return (bool)file;
}

void FileManagement::create_database(char name[20], int num_blocks)
{
	DataBase *database = new DataBase(name, num_blocks);
	string dbName(name);
	fstream file(dbName + ".txt", ios::beg | ios::out | ios::binary);

	if (!file) {
		cout << "Ha surgido un error, la base de datos no pudo ser creada.\n";
		return;
	}

	file.seekp(0);
	int size = sizeof(DataBlock);
	for (int i = 0; i < database->num_blocks; i++)
	{
		DataBlock datablock;

		if (i == 0)
			datablock.free = 0;

		file.write(reinterpret_cast<char*>(&datablock), size);
	}

	file.seekp(0);
	file << database->name;
	file << database->num_blocks;
	create_systemTable(dbName + ".txt");

	file.close();
}

void FileManagement::drop_database(string path)
{
	if (remove(path.c_str())==0)
		cout << "Base de datos borrada exitosamente." << endl;
	else
		cout << "Ha surgido un problema, la base de datos no pudo ser borrada." << endl;;

	system("pause");
}



//tablas de sistema
void FileManagement::create_systemTable(string path)
{
	char nameTable[20];
	strcpy(nameTable, (char*)"systable");
	Table tabla(nameTable);

	//ingresa el primer campo de la tabla
	char nameField[20];
	strcpy(nameField, (char*)"nombreTabla");

	char datatype[20];
	strcpy(datatype, (char*)"char");

	char size[20];
	strcpy(size, (char*)"20");

	tabla.fields.push_back(Field(nameField, datatype, size));

	//ingresa el segundo campo de tabla
	char nameField1[20];
	strcpy(nameField1, (char*)"index");
	char datatype1[20];
	strcpy(datatype1, (char*)"int");

	tabla.fields.push_back(Field(nameField1, datatype1, (char*)"0"));

	create_table(tabla, path);
	
}

void FileManagement::new_sysRegister(string path, Table table, int sysIndex, int tableIndex)
{
	Table tmp = table;

	fstream file(path, ios::in | ios::out | ios::binary);

	if (!file) {
		cout << "Ha surgido un error, la tabla no pudo ser creada.\n";
		return;
	}

	int index = find_lastRegister(path, sysIndex);
	file.seekp(index);
	file.write(tmp.name, 20);
	char index_[20];
	sprintf(index_, "%d", tableIndex);
	file.write(index_, 20);
	file.write("|", 1);
	file.close();
}



//void main
void FileManagement::run()
{
	FileManagement dba;
	int opcion_inicial = 0;
	string path = "";

	while (opcion_inicial != 4) {
		system("cls");
		cout << "***** MENU DE OPCIONES *****\n";
		cout << "1. Conectarse a una base de datos existente.\n";
		cout << "2. Crear una base de datos.\n";
		cout << "3. Borrar base de datos.\n";
		cout << "4. Salir.\n";
		cout << "Ingrese la opcion: ";
		cin >> opcion_inicial;
		switch (opcion_inicial)
		{
		case 1:
		{
			
			cout << "Ingrese el path de la base de datos: ";
			cin >> path;

			if (!database_exists(path)) {
				cout << "\nHa surgido un error, la conexion a la base de datos no ha sido exitosa." << endl;
				system("pause");
				break;
			}
			int opcion = 0;

			while(opcion!=7)
			{
			system("cls");
			cout << "----------MENU PRINCIPAL-----------" << endl;;
			cout << "\n";
			cout << "1. Crear tabla.\n";
			cout << "2. Eliminar tabla.\n";
			cout << "3. Insertar datos a una tabla en especifico.\n";
			cout << "4. Actualizar datos de una tabla en especifico\n";
			cout << "5. Borrar datos de una tabla en especifico.\n";
			cout << "6. Realizar una consulta.\n";
			cout << "7. Salir.\n";
			cout << "Ingrese la opcion: ";
			cin >> opcion;
			switch (opcion)
			{
			case 1:
			{
				
				system("cls");
				cout << "----------CREACION DE TABLA-----------" << endl;;

				char nombre[20] = "a";
				cout << "Ingrese el nombre de la tabla: ";
				cin >> nombre;
				bool agregar = true;

				Table table(nombre); //creo una tabla nueva

				while (agregar)
				{
					//pregunto al usuario si quiere agregar un campo
					cout << "\n";
					cout << "Desea agregar un campo? Si:1 , No:0 :   ";
					cin >> agregar;
					if (!agregar)
						break;

					char name[20] = "a";
					char datatype[20] = "a";
					char size[20] = "a";

					cout << "Nombre del campo: ";
					cin >> name;
					cout << "Escriba el tipo de dato: ";
					cin >> datatype;

					if (strcmp(datatype, "char") == 0) {
						cout << "Escriba el tamaño: ";
						cin >> size;
					}

					Field new_field(name, datatype, size); //creo un campo de la tabla nueva

					table.fields.push_back(new_field); //agrego un campo al vector de campos
				}

				dba.create_table(table, path);
				cout << "\nTabla creada exitosamente.\n";
				system("pause");
			}
			break;
			case 2:
			{
				system("cls");
				
				cout << "----------ELIMINACION DE TABLA-----------" << endl;;
				cout << "\n";
				cout << "Lista de tablas: " << endl;
				dba.list_allTables(path);
				cout << "\n";
				char tableName[20];
				cout << "Ingrese el nombre de la tabla a eliminar: ";
				cin >> tableName;

				int index = get_tableIndex(path, tableName);
				if (index == -1) {
					cout << "La tabla no fue encontrada." << endl;
					system("pause");
					break;
				}
				dba.delete_table(path, index, index ,tableName);
				cout << "Tabla eliminada exitosamente.\n";
				system("pause");
			}
				break;
			case 3:
			{
				system("cls");
				cout << "----------INGRESO DE DATOS-----------\n" << endl;;
				cout << "Lista de tablas: " << endl;
				dba.list_allTables(path);
				cout << "\n";

				char name[20] = "a";
				cout << "Ingrese el nombre de la tabla donde quiere ingresar un registro: ";
				cin >> name;
				
				int index = get_tableIndex(path,name);
				int agregar = 1;

				if (index == -1) {
					cout << "La tabla donde usted quiere ingresar registros no existe." << endl;
					system("pause");
					break;
				}
			
				while (agregar == 1) {

					cout << "\n";
					dba.new_register(path, index, index);
					
					cout << "Desea agregar otro registro? Si:1 , No:0:  ";
					cin >> agregar;
				}
			}
				break;
			case 4:
				system("cls");
				cout << "----------ACTUALIZACION DE DATOS-----------\n" << endl;;
				cout << "Lista de tablas: " << endl;
				dba.list_allTables(path);
				cout << "\n";
			
				dba.update_register(path);

				break;
			case 5:
			{
				int opcion = 0;
				system("cls");
				cout << "1. Eliminar todos los datos. " << endl;
				cout << "2. Borrar datos con condicion. "<< endl;
				cout << "Ingrese la opcion: ";
				cin >> opcion;
				switch (opcion)
				{
				case 1:
				{
					cout << "\nLista de tablas: " << endl;
					dba.list_allTables(path);
					cout << "\n";

					char tableName[20];
					cout << "Ingrese el nombre de la tabla donde se eliminaran registros: ";
					cin >> tableName;
					int tableIndex = get_tableIndex(path, tableName);

					if (tableIndex == -1) {
						cout << "La tabla no fue encontrada." << endl;
						system("pause");
						break;
					}

					if (has_noRegisters(path, tableIndex)) {
						cout << "Esta tabla no contiene registros." << endl;
						system("Pause");
						break;
					}

					dba.delete_allRegisters(path, tableIndex);

				}
					break;
				case 2:
				{
					cout << "\nLista de tablas: " << endl;
					dba.list_allTables(path);
					cout << "\n";

					char tableName[20];
					cout << "Ingrese el nombre de la tabla donde se eliminaran registros: ";
					cin >> tableName;
					int tableIndex = get_tableIndex(path, tableName);

					if (tableIndex == -1) {
						cout << "La tabla no fue encontrada." << endl;
						system("pause");
						break;
					}

					if (has_noRegisters(path, tableIndex)) {
						cout << "Esta tabla no contiene registros." << endl;
						system("Pause");
						break;
					}

					Table table = read_tableData(path, tableIndex);
					char identifier[20];
					char filter[100];
					cout << "\nCampos de la tabla " + (string)table.name + ": " << endl;
					for (int i = 0; i < table.fields.size(); i++)
					{
						cout << (string)table.fields[i].name << endl;;
					}
					cout << endl;
					cout << "Eliga el campo que usara para identificar el archivo: ";
					cin >> identifier;
					cout << "Escriba el valor del campo que se utilizara para identificar al registro: ";
					cin >> filter;

					int deleted = 1;
					while (deleted == 1) {
						deleted = dba.delete_register(path, tableIndex, identifier, filter, table);
					}

					if (eliminados) {
						eliminados = false;
						cout << "Registros eliminados exitosamente." << endl;
						system("pause");
					}
				}
					break;
				default:
					break;
				}
				
			}
				break;
			case 6:
			{
				int opcion = 0;
				system("cls");
				cout << "1. Seleccionar todos los registros. " << endl;
				cout << "2. Seleccionar datos con condicion. " << endl;
				cout << "Ingrese la opcion: ";
				cin >> opcion;

				switch (opcion)
				{
				case 1:
				{
					system("cls");
					cout << "Lista de tablas: " << endl;
					dba.list_allTables(path);
					char tableName[20];
					cout << "\nIngrese el nombre de la tabla donde se seleccionaran registros: ";
					cin >> tableName;
					int tableIndex = get_tableIndex(path, tableName);

					if (tableIndex == -1) {
						cout << "La tabla no fue encontrada." << endl;
						system("pause");
						break;
					}

					if (has_noRegisters(path, tableIndex)) {
						cout << "Esta tabla no contiene registros." << endl;
						system("Pause");
						break;
					}

					dba.select_allRegisters(path, tableIndex);

				}
				break;
				case 2:
				{
					system("cls");
					cout << "Lista de tablas: " << endl;
					dba.list_allTables(path);
					cout << "\n";
					char tableName[20];
					cout << "Ingrese el nombre de la tabla donde se eliminaran registros: ";
					cin >> tableName;
					dba.select_register(path, tableName);
				}
				break;
				default:
					break;
				}

			}
				break;
			case  7:
				break;
			default:
				break;
			}
			}

			break;
		}
		case 2:
		{
			
			system("cls");
			cout << "----------CREACION DE BASE DE DATOS-----------" << endl;;
			cout << "\n";
			char name[20];
			int bloques = 0;
			cout << "Ingrese el nombre de la base de datos: ";
			cin >> name;
			cout << "Ingrese el numero de megabytes que ocupara la base de datos: ";

			cin >> bloques;
			dba.create_database(name, bloques);
		}
		break;

		case 3:
		{
			cout << "----------CREACION DE BASE DE DATOS-----------" << endl;;
			cout << "\n";
			string databse = "";
			cout << "Ingrese el nombre de la base de datos a borrar: ";
			cin >> databse;
			dba.drop_database(databse);
		}
		break;
		case 4:
			break;
		default:
			break;
		}
	}
}

