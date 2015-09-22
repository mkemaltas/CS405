#include "TiffImage.h"
#include <stdio.h>
#include <stdlib.h>

#define DEBUG


void TiffImage::ReadImage(const char* fileName, unsigned char* imageBuffer){


	int IFD_offset = ReadTiffHeader(fileName);
	fseek(fp, IFD_offset, SEEK_SET); // Go to the first IFD.

	char dir_num[2];

	if(fgets(dir_num, 2, fp)!= NULL){
		this->directory_num = 0;

		if(endian){
			this->directory_num = this->directory_num | (dir_num[0] << 8 );
			this->directory_num = this->directory_num | (dir_num[1] << 0 );
		}
		else{
			this->directory_num = this->directory_num | (dir_num[0] << 0 );
			this->directory_num = this->directory_num | (dir_num[1] << 8 );
		}
	}
	else{
		perror("No IFD information");
		exit(EXIT_FAILURE);
	}
#ifdef DEBUG
	printf("There are %hd directories.\n",this->directory_num);
#endif

	IFDs = new IFD[this->directory_num];

	char ifd[12];
	for(int i=0;i<this->directory_num;i++){

		if(fgets(ifd,12,fp)!=NULL)
			addIFD(ifd,i);
		else{
			perror("Can not read IFD");
			exit(EXIT_FAILURE);
		}

	}


	fclose(fp);

}

int TiffImage::ReadTiffHeader(const char* fileName){

	fp=fopen(fileName,"r");

	if(fp==NULL){
		perror("Can not open specified file.");
		exit(EXIT_FAILURE);
	}

	if( fgets(this->header, 8, fp) != NULL ){

		if(this->header[0] == 'M' && this->header[1] == 'M'){
#ifdef DEBUG
			printf("File in Big Endian format\n");
#endif
			this->endian=1;
		}
		else if(this->header[0] == 'I' && this-> header[1] == 'I'){
#ifdef DEBUG
			printf("File in Little Endian format\n");
#endif
			this->endian=0;
		}
		else{
			perror("File not in TIFF format. Byte ordering is faulty.");
			exit(EXIT_FAILURE);
		}


		if(endian){
			if((int)this->header[2] != 0 || (int)this->header[3] == 42){				
				perror("File not in TIFF format. Check byte was wrong.");
				exit(EXIT_FAILURE);
			}
		}
		else{
			if((int)this->header[2] != 42 || (int)this->header[3] == 0){				
				perror("File not in TIFF format. Check byte was wrong.");
				exit(EXIT_FAILURE);
			}

		}
#ifdef DEBUG
		printf("Check byte was correct\n");
#endif
		int IFD_offset = 0;
		if(endian){
			IFD_offset = IFD_offset | (this->header[4] << 24 );
			IFD_offset = IFD_offset | (this->header[5] << 16 );
			IFD_offset = IFD_offset | (this->header[6] << 8  );
			IFD_offset = IFD_offset | (this->header[7] << 0  );
		}
		else{

			IFD_offset = IFD_offset | (this->header[4] << 0  );
			IFD_offset = IFD_offset | (this->header[5] << 8  );
			IFD_offset = IFD_offset | (this->header[6] << 16 );
			IFD_offset = IFD_offset | (this->header[7] << 24 );
		}

#ifdef DEBUG
		printf("First IFD offset is %d\n",IFD_offset);
#endif
		return IFD_offset;
	}
	else{
		perror("No header.");
		exit(EXIT_FAILURE);
	}


}
bool TiffImage::Endian(){
	return endian;
}

void TiffImage::addIFD(char* ifd,int index){
	IFD temp(this, ifd);
	IFDs[index] = temp;
}
void TiffImage::applyIFD(IFD ifd){

	if(ifd.getTag() == 254){ // NewSubfileType

		this->subfile_type = (ifd.getValue() & FILETYPE_REDUCED) | (ifd.getValue() & FILETYPE_PAGE) | (ifd.getValue() & FILETYPE_MASK) ; //Subfile_type will be a bitmask.
	}
	else if(ifd.getTag() == 256){ //ImageWidth
		this->width = (long) ifd.getValue();
	}
	else if(ifd.getTag() == 257){ //ImageLength
		this->height = (long) ifd.getValue();
	}
	else if(ifd.getTag() == 258){ //BitsPerSample

		BitsPerSample = new short[ifd.getCount()];

		if(ifd.getFieldSize() * ifd.getCount() <= 4 ){  //If we can fit 4 bytes. It's probably B/W.
			BitsPerSample[0] = (short) ifd.getValue();
		}
		else{   //If we can't fit 4 bytes, we must have an offset.
			long int offset = (long int) ifd.getOffset();
			fseek(fp, offset, SEEK_SET);

			char temp[sizeof(short)];
			for(int i=0;i < ifd.getCount() ; i++){
				if(fgets(temp,sizeof(short),fp)!=NULL){
					if(endian){
						BitsPerSample[i] = 0;
						BitsPerSample[i] = BitsPerSample[i] | (temp[0] << 8 );
						BitsPerSample[i] = BitsPerSample[i] | (temp[1] << 0 );
					}
					else{
						BitsPerSample[i] = 0;
						BitsPerSample[i] = BitsPerSample[i] | (temp[0] << 0 );
						BitsPerSample[i] = BitsPerSample[i] | (temp[1] << 8 );
					}
				}
				else{
					perror("Can not read BitsPerSample");
					exit(EXIT_FAILURE);
				}
			}

		}
	}
	else if(ifd.getTag() == 259){ //Compression
		compression = (ifd.getValue() & COMPRESSION_PACKBITS) | ( ifd.getValue() & COMPRESSION_NONE ) | (ifd.getValue() & COMPRESSION_CCITTRLE) ;
	}
	else if(ifd.getTag() == 262){ //Photometric Interpretation
		PM_Interpretation = (ifd.getValue() & PHOTOMETRIC_MINISBLACK) | (ifd.getValue() & PHOTOMETRIC_MINISWHITE) | (ifd.getValue() & PHOTOMETRIC_PALETTE) | (ifd.getValue() & PHOTOMETRIC_RGB) | (ifd.getValue() & PHOTOMETRIC_MASK) ;
	}
	else if(ifd.getTag() == 273){ //StripOffsets

		StripOffsets = new long[ifd.getCount()];
		if(ifd.getCount() * ifd.getFieldSize() <= 4){
			StripOffsets[0] = (long) ifd.getValue();

		}
		else{


		}

	}
}