#pragma once
#include "DataBase.h"
#include "Datablock.h"
#include "Structure.h"

#include <string>

using namespace std;

class FileManagement {
public:
	FileManagement();

	//funciones para la creacion de bases de datos
	void create_database(char[20], int); //crea una nueva base de datos

	//funciones para la eliminacion de bases de datos
	void drop_database(string path);

	//funciones para la creacion de tablas
	void create_table(Table table, string path); //crea una nueva tabla dentro del archivo

	//manejo de tablas
	void list_allTables(string path);

	//funciones para la eliminacion de tablas
	void delete_table(string path, int index, int sysIndex ,char table_name[20]);
	void delete_sysRegister(string path, char table_name[100], int index);
	

	//funciones para el ingreso  de registros
	void new_register(string path, int index, int tmpIndex);

	//funciones para la elimininacion de registros
	int delete_register(string path,int index, char indentifier[100], char filter[100],Table table);
	void delete_allRegisters(string path, int index);

	//funciones para la actualizacion de registros
	void update_register(string path);
	
	//funciones para la seleccion de registros
	void select_allRegisters(string path, int index);
	void select_register(string path, char name[20]);

	//funciones para uso general
	int get_tableIndex(string path, char name[20]);
	int search_tableIndex(string path, char name[20], int index);
	int get_metadataSize(string path, int index);
	bool block_isFull(string path, int index);
	double get_registerCount(string path, int index, int registerSize, int metadataSize);
	Table read_tableData(string path, int index);
	int reading_Size(Table table, char identifier[4000]);
	int get_identifierSize(Table table, char identifier[20]);

	//funciones para el manejo de registros
	int get_registerSize(Table table);
	int find_firstRegistry(string path, int index);
	int find_lastRegister(string path, int index); //busca el delimitador que representa el comienzo de los registros en una tabla
	int find_registerIndex(string path, int name, char[20], Table table, char value[100]);
	bool fills_dataBlock(string path, int index, int registerSize);
	int register_count(string path, int index);
	bool isLast_Register(string path, int index, char identifier[20], char value[4000], Table table);
	void split(char *data);

	//funciones utilizadas para el manejo de bloques
	int search_freeBlock(string path); //busca el primer bloque libre
	void make_blockBusy(string path, int index);
	bool hasNext(string path,int index);
	int assing_nextBlock(string path, int index);
	bool has_noRegisters(string path, int index);
	
	//funciones para el manejo de base de datos
	bool database_exists(string path);

	//crear tabla del sistema
	void create_systemTable(string path);
	void new_sysRegister(string path,Table tabla,int sysIndex, int tableIndex);

	//void main
	void run();
};