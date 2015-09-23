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
			long int offset = (long int) ifd.getOffset();
			fseek(fp, offset, SEEK_SET);

			char *temp = new char[ifd.getFieldSize()];
			for(int i=0;i<ifd.getCount();i++){
				StripOffsets[i] = 0;
				if(endian){
					if(fgets(temp,ifd.getFieldSize(),fp)!=NULL){
						for(int j=0;j<ifd.getFieldSize();j++){
							StripOffsets[i] = StripOffsets[i] | ( temp[j] << ( (ifd.getFieldSize() - j -1) *8));
						}
					}
					else{
						perror("Can not read IFD Strip Offsets");
						exit(EXIT_FAILURE);
					}
				}
				else{
					if(fgets(temp,ifd.getFieldSize(),fp)!=NULL){
						for(int j=0;j<ifd.getFieldSize();j++){
							StripOffsets[i] = StripOffsets[i] | ( temp[ifd.getFieldSize() - j -1] << ( (ifd.getFieldSize() - j -1) *8));
						}
					}
					else{
						perror("Can not read IFD Strip Offsets");
						exit(EXIT_FAILURE);
					}
				}
			}
		}

	}
	else if(ifd.getTag() == 274) { // Orientation
		orientation = (ifd.getValue() & ORIENTATION_BOTLEFT) | (ifd.getValue() & ORIENTATION_BOTRIGHT) | (ifd.getValue() & ORIENTATION_LEFTBOT) |  (ifd.getValue() & ORIENTATION_TOPRIGHT) |
			(ifd.getValue() & ORIENTATION_LEFTTOP) | (ifd.getValue() & ORIENTATION_RIGHTBOT) | (ifd.getValue() & ORIENTATION_RIGHTTOP) | (ifd.getValue() & ORIENTATION_TOPLEFT);
	}
	else if(ifd.getTag() == 277) { // SamplesPerPixel
		SamplesPerPixel = (short) ifd.getValue();	
	}
	else if(ifd.getTag() == 278) { // RowsPerStrip
		RowsPerStrip = (long) ifd.getValue();
	}
	else if(ifd.getTag() == 279) { // StripByteCounts
		StripByteCounts = new long[ifd.getCount()];
		if(ifd.getCount() * ifd.getFieldSize() <= 4){
			StripByteCounts[0] = (long) ifd.getValue();
		}
		else{

			long int offset = (long int) ifd.getOffset();
			fseek(fp, offset, SEEK_SET);

			char *temp = new char[ifd.getFieldSize()];
			for(int i=0;i<ifd.getCount();i++){
				StripByteCounts[i] = 0;
				if(endian){
					if(fgets(temp,ifd.getFieldSize(),fp)!=NULL){
						for(int j=0;j<ifd.getFieldSize();j++){
							StripByteCounts[i] = StripByteCounts[i] | ( temp[j] << ( (ifd.getFieldSize() - j -1) *8));
						}
					}
					else{
						perror("Can not read IFD Strip Byte Counts");
						exit(EXIT_FAILURE);
					}
				}
				else{
					if(fgets(temp,ifd.getFieldSize(),fp)!=NULL){
						for(int j=0;j<ifd.getFieldSize();j++){
							StripByteCounts[i] = StripByteCounts[i] | ( temp[ifd.getFieldSize() - j -1] << ( (ifd.getFieldSize() - j -1) *8));
						}
					}
					else{
						perror("Can not read IFD Strip Byte Counts");
						exit(EXIT_FAILURE);
					}
				}
			}
		}
	}
	else if(ifd.getTag() == 282){ //XResolution. 1-Ustteki 2-Alttaki

		XResolution = new long[2];

		long int offset = (long int) ifd.getOffset();
		fseek(fp, offset, SEEK_SET);

		char *temp = new char(sizeof(long));
		for(int i=0;i<2;i++){
			XResolution[i] = 0;
			if(fgets(temp,sizeof(long),fp)!=NULL){
				if(endian){
					XResolution[i] = XResolution[i] & (temp[0] << 24);
					XResolution[i] = XResolution[i] & (temp[1] << 16);
					XResolution[i] = XResolution[i] & (temp[2] << 8 );
					XResolution[i] = XResolution[i] & (temp[3] << 0 );
				}

				else{
					XResolution[i] = XResolution[i] & (temp[0] << 0 );
					XResolution[i] = XResolution[i] & (temp[1] << 8 );
					XResolution[i] = XResolution[i] & (temp[2] << 16);
					XResolution[i] = XResolution[i] & (temp[3] << 24);
				}
			}
			else{
				perror("Error reading IFD Xresolution");
				exit(EXIT_FAILURE);
			}

		}
	}
	else if(ifd.getTag() == 283){ //YResolution. 1-Ustteki 2-Alttaki

		YResolution = new long[2];

		long int offset = (long int) ifd.getOffset();
		fseek(fp, offset, SEEK_SET);

		char *temp = new char(sizeof(long));
		for(int i=0;i<2;i++){
			YResolution[i] = 0;
			if(fgets(temp,sizeof(long),fp)!=NULL){
				if(endian){
					YResolution[i] = YResolution[i] & (temp[0] << 24);
					YResolution[i] = YResolution[i] & (temp[1] << 16);
					YResolution[i] = YResolution[i] & (temp[2] << 8 );
					YResolution[i] = YResolution[i] & (temp[3] << 0 );
				}

				else{
					YResolution[i] = YResolution[i] & (temp[0] << 0 );
					YResolution[i] = YResolution[i] & (temp[1] << 8 );
					YResolution[i] = YResolution[i] & (temp[2] << 16);
					YResolution[i] = YResolution[i] & (temp[3] << 24);
				}
			}
			else{
				perror("Error reading IFD Yresolution");
				exit(EXIT_FAILURE);
			}

		}
	}
}

void TiffImage::printMetaInformation(){

	printf("----------------------------------\n");
	if(endian)
		printf("Byte Order: Big Endian\n");
	else
		printf("Byte Order: Little Endian\n");
	printf("Number of directories:%hd\n",directory_num);
	printf("Width:%ld\n",width);
	printf("Height:%ld\n",height);

	if(subfile_type == FILETYPE_PAGE)
		printf("Subfile Type: Page\n");
	else if(subfile_type == FILETYPE_MASK)
		printf("Subfile Type: Mask\n");
	else if(subfile_type == FILETYPE_REDUCED)
		printf("Subfile Type: Reduced\n");


	if(compression == COMPRESSION_NONE)
		printf("Compression Method: None\n");
	else if(compression == COMPRESSION_PACKBITS)	
		printf("Compression Method: Pack Bits\n");
	else if(compression == COMPRESSION_CCITTRLE)
		printf("Compression Method: CCITT RLE\n");
	else
		printf("Compression Method: Not specified\n");

	if(PM_Interpretation == PHOTOMETRIC_MINISWHITE)
		printf("Photometric Interpretation: 0 is white.\n");
	else if(PM_Interpretation == PHOTOMETRIC_MINISBLACK)
		printf("Photometric Interpretation: 0 is black.\n");
	else if(PM_Interpretation == PHOTOMETRIC_RGB )
		printf("Photometric Interpretation: RGB.\n");
	else if(PM_Interpretation == PHOTOMETRIC_PALETTE )
		printf("Photometric Interpretation: Pallette.\n");
	else if(PM_Interpretation == PHOTOMETRIC_MASK )
		printf("Photometric Interpretation: Transparency Mask.\n");
	else
		printf("Photometric Interpretation: Not specified\n");

	if(orientation == ORIENTATION_BOTLEFT)
		printf("Orientation: Bot-Left\n");
	else if(orientation == ORIENTATION_BOTRIGHT)
		printf("Orientation: Bot-Right\n");
	else if(orientation == ORIENTATION_LEFTBOT)
		printf("Orientation: Left-Bot\n");
	else if(orientation == ORIENTATION_LEFTTOP)
		printf("Orientation: Left-Top\n");
	else if(orientation == ORIENTATION_RIGHTBOT )
		printf("Orientation: Right-Bot\n");
	else if(orientation == ORIENTATION_RIGHTTOP)
		printf("Orientation: Right-Top\n");
	else if(orientation == ORIENTATION_TOPLEFT)
		printf("Orientation: Top-Left\n");
	else if(orientation == ORIENTATION_TOPRIGHT)
		printf("Orientation: Top-Right\n");
	else
		printf("Orientation: Not specified\n");

	printf("Bits Per Sample: ");
	if(PM_Interpretation == PHOTOMETRIC_RGB){
		for(int i=0;i<3;i++){
			printf("%hd\t\n",BitsPerSample[i]);
		}
	}
	else
		printf("%hd\n",BitsPerSample[0]);

	printf("Samples Per Pixel: %hd\n",SamplesPerPixel);

	printf("X Resolution: %ld/%ld\n",XResolution[0],XResolution[1]);
	printf("Y Resolution: %ld/%ld\n",YResolution[0],YResolution[1]);

	printf("Rows Per Strip: %ld\n",RowsPerStrip);


	printf("----------------------------------\n");

}