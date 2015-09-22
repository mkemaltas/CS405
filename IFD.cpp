#include "IFD.h"

IFD::IFD(){

}
IFD::IFD(TiffImage *image, char* ifd){

	bool endian = image->Endian();
	
	///////////////TAG///////////////
	tag = 0;
	if(endian){
		tag = tag | (ifd[0] << 8 );
		tag = tag | (ifd[1] << 0 );
	}
	else{
		tag = tag | (ifd[0] << 0 );
		tag = tag | (ifd[1] << 8 );
	}
#ifdef DEBUG
	printf("Tag is %hd \n",tag);
#endif

	//////////FIELD TYPE/////////////
	short ftype = 0;
	if(endian){
		ftype = ftype | (ifd[2] << 8 );
		ftype = ftype | (ifd[3] << 0 );
	}
	else{
		ftype = ftype | (ifd[2] << 0 );
		ftype = ftype | (ifd[3] << 8 );
	}

	switch(ftype){
	case 1 :
		this->type = BYTE;
		this->fieldSize = 1;
#ifdef DEBUG
		printf("Field type is BYTE\n");
#endif
		break;
	case 2 :
		this->type = ASCII;
		this->fieldSize = sizeof(char);
#ifdef DEBUG
		printf("Field type is ASCII\n");
#endif
		break;
	case 3 :
		this->type = SHORT;
		this->fieldSize = sizeof(short);
#ifdef DEBUG
		printf("Field type is SHORT\n");
#endif
		break;

	case 4 :
		this->type = LONG;
		this->fieldSize = sizeof(long);
#ifdef DEBUG
		printf("Field type is LONG\n");
#endif
		break;
	case 5 : 
		this->type = RATIONAL;
		this->fieldSize = 2*sizeof(long);
#ifdef DEBUG
		printf("Field type is RATIONAL\n");
#endif
		break;
	default : 
		printf("Field type is %hd\n",ftype);
		perror("Field Type is undefined");
		exit(EXIT_FAILURE);
	}
	//////////////COUNT//////////////
	this->count = 0;
	if(endian){
		this->count = this->count | ( ifd[4] << 24 );
		this->count = this->count | ( ifd[5] << 16 );
		this->count = this->count | ( ifd[6] << 8  );
		this->count = this->count | ( ifd[7] << 0  );
	}
	else{
		this->count = this->count | ( ifd[4] << 0  );
		this->count = this->count | ( ifd[5] << 8  );
		this->count = this->count | ( ifd[6] << 16 );
		this->count = this->count | ( ifd[7] << 24 );
	}
#ifdef DEBUG
	printf("Count is %u\n", this->count);
#endif


	////////////VALUE/OFFSET/////////
	if(this->count * this->fieldSize <= 4){ // If we can fit the value into 4 bytes, we just write the value. Otherwise, we write the offset.
		this->value  = 0;
		this->offset = NULL;
		if(endian){
			this->value = this->value | (ifd[8]  << 24 );
			this->value = this->value | (ifd[9]  << 16 );
			this->value = this->value | (ifd[10] << 8  );
			this->value = this->value | (ifd[11] << 0  );
		}
		else{
			this->value = this->value | (ifd[8]  << 0  );
			this->value = this->value | (ifd[9]  << 8  );
			this->value = this->value | (ifd[10] << 16 );
			this->value = this->value | (ifd[11] << 24 );
		}
#ifdef DEBUG
	printf("Value fits 4 bytes. Value is %u\n",this->value);
#endif
	}
	else{
		this->offset = 0;
		this->value  = NULL;
		if(endian){
			this->offset = this->offset | (ifd[8]  << 24 );
			this->offset = this->offset | (ifd[9]  << 16 );
			this->offset = this->offset | (ifd[10] << 8  );
			this->offset = this->offset | (ifd[11] << 0  );
		}
		else{
			this->offset = this->offset | (ifd[8]  << 0  );
			this->offset = this->offset | (ifd[9]  << 8  );
			this->offset = this->offset | (ifd[10] << 16 );
			this->offset = this->offset | (ifd[11] << 24 );
		}
#ifdef DEBUG
		printf("Value doesn't fit 4 bytes. Offset is %u\n",this->offset);
#endif

	}

}

short IFD::getTag(){
	return tag;
}
fieldType IFD::getType(){
	return type;
}
short IFD::getFieldSize(){
	return fieldSize;
}
unsigned int IFD::getCount(){
	return count;
}
unsigned int IFD::getValue(){
	return value;
}
unsigned int IFD::getOffset(){
	return offset;
}