#pragma once

class DataBlock {
public:
	char data[4088];
	int next;
	int free;

	DataBlock();
};


