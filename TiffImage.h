#ifndef _TIFFIMAGE
#define _TIFFIMAGE

#include "IFD.h"

#define IFD_SIZE 12

//Subfile Types
#define FILETYPE_REDUCED 1
#define FILETYPE_PAGE 2
#define FILETYPE_MASK 4
//Compression
#define COMPRESSION_NONE 1
#define COMPRESSION_CCITTRLE 2
#define COMPRESSION_PACKBITS 32773
//Photometric Interpretation
#define PHOTOMETRIC_MINISWHITE 0
#define PHOTOMETRIC_MINISBLACK 1
#define PHOTOMETRIC_RGB 2
#define PHOTOMETRIC_PALETTE 3
#define PHOTOMETRIC_MASK 4

class TiffImage
{
public:


	int ReadTiffHeader(const char* fileName);
	void ReadImage(const char* fileName, unsigned char* imageBuffer);
	void addIFD(char* ifd, int index);
	void applyIFD(IFD ifd);
	bool Endian();


private:
	FILE *fp;

	long width;
	long height;
	bool endian; // 0 for little endian, 1 for big endian.
	short int directory_num;

	//IFD information
	long subfile_type;
	short compression;
	short PM_Interpretation;
	short *BitsPerSample;
	long  *StripOffsets;

	IFD *IFDs; // One-to-many relationship.

	char header[8];
};


#endif