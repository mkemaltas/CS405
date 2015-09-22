#ifndef _IFD
#define _IFD
#include "TiffImage.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

enum fieldType{
	BYTE, ASCII, SHORT, LONG, RATIONAL
};

class IFD{

public:

	IFD();
	IFD(TiffImage *image, char* ifd);

	short getTag();
	fieldType getType();
	short getFieldSize();
	unsigned int getCount();
	unsigned int getValue();
	unsigned int getOffset();


private:

	//many IFD to one Image relationship.
	TiffImage Image;

	short tag;
	fieldType type;
	short fieldSize;
	unsigned int count;
	unsigned int value;	// either value or offset will be used. They don't exist together.	
	unsigned int offset;
};
#endif