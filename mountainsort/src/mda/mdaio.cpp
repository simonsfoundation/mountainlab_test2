#include "mdaio.h"
#include "usagetracking.h"

long mda_read_header(struct MDAIO_HEADER *HH,FILE *input_file) {
	long num_read=0;
	long i;
	long totsize;

	//initialize
	HH->data_type=0;
	HH->num_bytes_per_entry=0;
	HH->num_dims=0;
	for (i=0; i<MDAIO_MAX_DIMS; i++) HH->dims[i]=1;
	HH->header_size=0;

	if (!input_file) return 0;

	//data type
	num_read=jfread(&HH->data_type,4,1,input_file);
	if (num_read<1) {
        printf("mda_read_header: Problem reading input file.\n");
		return 0;
	}

	if ((HH->data_type<-7)||(HH->data_type>=0)) {
		printf("mda_read_header: Problem with data type:  %d.\n",HH->data_type);
		return 0;
	}

	//number of bytes per entry
	num_read=jfread(&HH->num_bytes_per_entry,4,1,input_file);
	if (num_read<1) return 0;

	if ((HH->num_bytes_per_entry<=0)||(HH->num_bytes_per_entry>8)) return 0;

	//number of dimensions
	num_read=jfread(&HH->num_dims,4,1,input_file);
	if (num_read<1) return 0;

	if ((HH->num_dims<=0)||(HH->num_dims>MDAIO_MAX_DIMS)) return 0;

	//the dimensions
	totsize=1;
	for (i=0; i<HH->num_dims; i++) {
		num_read=jfread(&HH->dims[i],4,1,input_file);
		if (num_read<1) return 0;
		totsize*=HH->dims[i];
	}

	if ((totsize<=0)||(totsize>MDAIO_MAX_SIZE)) {
		printf("mda_read_header: Problem with total size: %ld\n",totsize);
		return 0;
	}

	HH->header_size=(3+HH->num_dims)*4;

	//we're done!
	return 1;
}

long mda_write_header(struct MDAIO_HEADER *X,FILE *output_file) {
	long num_bytes;
	long i;

	//make sure we have the right number of bytes per entry in case the programmer forgot to set this!
	if (X->data_type==MDAIO_TYPE_BYTE) X->num_bytes_per_entry=1;
	else if (X->data_type==MDAIO_TYPE_COMPLEX) X->num_bytes_per_entry=8;
	else if (X->data_type==MDAIO_TYPE_FLOAT32) X->num_bytes_per_entry=4;
	else if (X->data_type==MDAIO_TYPE_INT16) X->num_bytes_per_entry=2;
	else if (X->data_type==MDAIO_TYPE_INT32) X->num_bytes_per_entry=4;
	else if (X->data_type==MDAIO_TYPE_UINT16) X->num_bytes_per_entry=2;
    else if (X->data_type==MDAIO_TYPE_FLOAT64) X->num_bytes_per_entry=8;

	if ((X->num_dims<=0)||(X->num_dims>MDAIO_MAX_DIMS)) {
		printf("mda_write_header: Problem with num dims: %d\n",X->num_dims);
		return 0;
	}

	//data type
	num_bytes=fwrite(&X->data_type,4,1,output_file);
	if (num_bytes<1) return 0;

	//number of bytes per entry
	num_bytes=fwrite(&X->num_bytes_per_entry,4,1,output_file);
	if (num_bytes<1) return 0;

	//number of dimensions
	num_bytes=fwrite(&X->num_dims,4,1,output_file);
	if (num_bytes<1) return 0;

	//the dimensions
	for (i=0; i<X->num_dims; i++) {
		num_bytes=fwrite(&X->dims[i],4,1,output_file);
		if (num_bytes<1) return 0;
	}

	X->header_size=12+4*X->num_dims;

	//we're done!
	return 1;
}

#define MDA_READ_MACRO(type1,type2) \
type2 *tmp=(type2 *)malloc(sizeof(type2)*n); \
long ret=jfread(tmp,sizeof(type2),n,input_file); \
for (i=0; i<n; i++) data[i]=(type1)tmp[i]; \
free(tmp); \
return ret;
#define MDA_READ_MACRO_SAME(type1,type2) \
return jfread(data,sizeof(type1),n,input_file);

long mda_read_byte(unsigned char *data,struct MDAIO_HEADER *H,long n,FILE *input_file) {
	long i;
	if (H->data_type==MDAIO_TYPE_BYTE) {
		MDA_READ_MACRO_SAME(unsigned char,unsigned char)
	}
	else if (H->data_type==MDAIO_TYPE_FLOAT32) {
		MDA_READ_MACRO(unsigned char,float)
	}
	else if (H->data_type==MDAIO_TYPE_INT16) {
		MDA_READ_MACRO(unsigned char,int16_t)
	}
	else if (H->data_type==MDAIO_TYPE_INT32) {
		MDA_READ_MACRO(unsigned char,int32_t)
	}
	else if (H->data_type==MDAIO_TYPE_UINT16) {
		MDA_READ_MACRO(unsigned char,uint16_t)
	}
    else if (H->data_type==MDAIO_TYPE_FLOAT64) {
        MDA_READ_MACRO(unsigned char,double)
    }
	else return 0;
}

long mda_read_float32(float *data,struct MDAIO_HEADER *H,long n,FILE *input_file) {
	long i;
	if (H->data_type==MDAIO_TYPE_BYTE) {
		MDA_READ_MACRO(float,unsigned char)
	}
	else if (H->data_type==MDAIO_TYPE_FLOAT32) {
		MDA_READ_MACRO_SAME(float,float)
	}
	else if (H->data_type==MDAIO_TYPE_INT16) {
		MDA_READ_MACRO(float,int16_t)
	}
	else if (H->data_type==MDAIO_TYPE_INT32) {
		MDA_READ_MACRO(float,int32_t)
	}
	else if (H->data_type==MDAIO_TYPE_UINT16) {
		MDA_READ_MACRO(float,uint16_t)
	}
    else if (H->data_type==MDAIO_TYPE_FLOAT64) {
        MDA_READ_MACRO(float,double)
    }
	else return 0;
}

long mda_read_float64(double *data,struct MDAIO_HEADER *H,long n,FILE *input_file) {
	long i;
    if (H->data_type==MDAIO_TYPE_BYTE) {
        MDA_READ_MACRO(double,unsigned char)
    }
    else if (H->data_type==MDAIO_TYPE_FLOAT32) {
        MDA_READ_MACRO(double,float)
    }
    else if (H->data_type==MDAIO_TYPE_INT16) {
        MDA_READ_MACRO(double,int16_t)
    }
    else if (H->data_type==MDAIO_TYPE_INT32) {
        MDA_READ_MACRO(double,int32_t)
    }
    else if (H->data_type==MDAIO_TYPE_UINT16) {
        MDA_READ_MACRO(double,uint16_t)
    }
    else if (H->data_type==MDAIO_TYPE_FLOAT64) {
        MDA_READ_MACRO_SAME(double,double)
    }
    else return 0;
}

long mda_read_int16(int16_t *data,struct MDAIO_HEADER *H,long n,FILE *input_file) {
	long i;
	if (H->data_type==MDAIO_TYPE_BYTE) {
		MDA_READ_MACRO(int16_t,unsigned char)
	}
	else if (H->data_type==MDAIO_TYPE_FLOAT32) {
		MDA_READ_MACRO(int16_t,float)
	}
	else if (H->data_type==MDAIO_TYPE_INT16) {
		MDA_READ_MACRO_SAME(int16_t,int16_t)
	}
	else if (H->data_type==MDAIO_TYPE_INT32) {
		MDA_READ_MACRO(int16_t,int32_t)
	}
	else if (H->data_type==MDAIO_TYPE_UINT16) {
		MDA_READ_MACRO(int16_t,uint16_t)
	}
    else if (H->data_type==MDAIO_TYPE_FLOAT64) {
        MDA_READ_MACRO(int16_t,double)
    }
	else return 0;
}

long mda_read_int32(int32_t *data,struct MDAIO_HEADER *H,long n,FILE *input_file) {
	long i;
	if (H->data_type==MDAIO_TYPE_BYTE) {
		MDA_READ_MACRO(int32_t,unsigned char)
	}
	else if (H->data_type==MDAIO_TYPE_FLOAT32) {
		MDA_READ_MACRO(int32_t,float)
	}
	else if (H->data_type==MDAIO_TYPE_INT16) {
		MDA_READ_MACRO(int32_t,int16_t)
	}
	else if (H->data_type==MDAIO_TYPE_INT32) {
		MDA_READ_MACRO_SAME(int32_t,int32_t)
	}
	else if (H->data_type==MDAIO_TYPE_UINT16) {
		MDA_READ_MACRO(int32_t,uint16_t)
	}
    else if (H->data_type==MDAIO_TYPE_FLOAT64) {
        MDA_READ_MACRO(int32_t,double)
    }
	else return 0;
}

long mda_read_uint16(uint16_t *data,struct MDAIO_HEADER *H,long n,FILE *input_file) {
	long i;
	if (H->data_type==MDAIO_TYPE_BYTE) {
		MDA_READ_MACRO(uint16_t,unsigned char)
	}
	else if (H->data_type==MDAIO_TYPE_FLOAT32) {
		MDA_READ_MACRO(uint16_t,float)
	}
	else if (H->data_type==MDAIO_TYPE_INT16) {
		MDA_READ_MACRO(uint16_t,int16_t)
	}
	else if (H->data_type==MDAIO_TYPE_INT32) {
		MDA_READ_MACRO(uint16_t,int32_t)
	}
	else if (H->data_type==MDAIO_TYPE_UINT16) {
		MDA_READ_MACRO_SAME(uint16_t,uint16_t)
	}
    else if (H->data_type==MDAIO_TYPE_FLOAT64) {
        MDA_READ_MACRO(uint16_t,double)
    }
	else return 0;
}

#define MDA_WRITE_MACRO(type1,type2) \
type2 *tmp=(type2 *)malloc(sizeof(type2)*n); \
for (i=0; i<n; i++) tmp[i]=(type2)data[i]; \
long ret=fwrite(tmp,sizeof(type2),n,output_file); \
free(tmp); \
return ret;
#define MDA_WRITE_MACRO_SAME(type1,type2) \
return fwrite(data,sizeof(type1),n,output_file);

long mda_write_byte(unsigned char *data,struct MDAIO_HEADER *H,long n,FILE *output_file) {
	long i;
	if (H->data_type==MDAIO_TYPE_BYTE) {
		MDA_WRITE_MACRO_SAME(unsigned char,unsigned char)
	}
	else if (H->data_type==MDAIO_TYPE_FLOAT32) {
		MDA_WRITE_MACRO(unsigned char,float)
	}
	else if (H->data_type==MDAIO_TYPE_INT16) {
		MDA_WRITE_MACRO(unsigned char,int16_t)
	}
	else if (H->data_type==MDAIO_TYPE_INT32) {
		MDA_WRITE_MACRO(unsigned char,int32_t)
	}
	else if (H->data_type==MDAIO_TYPE_UINT16) {
		MDA_WRITE_MACRO(unsigned char,uint16_t)
	}
    else if (H->data_type==MDAIO_TYPE_FLOAT64) {
        MDA_WRITE_MACRO(unsigned char,double)
    }
	else return 0;
}

long mda_write_float32(float *data,struct MDAIO_HEADER *H,long n,FILE *output_file) {
	long i;
	if (H->data_type==MDAIO_TYPE_BYTE) {
		MDA_WRITE_MACRO(float,unsigned char)
	}
	else if (H->data_type==MDAIO_TYPE_FLOAT32) {
		MDA_WRITE_MACRO_SAME(float,float)
	}
	else if (H->data_type==MDAIO_TYPE_INT16) {
		MDA_WRITE_MACRO(float,int16_t)
	}
	else if (H->data_type==MDAIO_TYPE_INT32) {
		MDA_WRITE_MACRO(float,int32_t)
	}
	else if (H->data_type==MDAIO_TYPE_UINT16) {
		MDA_WRITE_MACRO(float,uint16_t)
	}
    else if (H->data_type==MDAIO_TYPE_FLOAT64) {
        MDA_WRITE_MACRO(float,double)
    }
	else return 0;
}

long mda_write_int16(int16_t *data,struct MDAIO_HEADER *H,long n,FILE *output_file) {
	long i;
	if (H->data_type==MDAIO_TYPE_BYTE) {
		MDA_WRITE_MACRO(int16_t,unsigned char)
	}
	else if (H->data_type==MDAIO_TYPE_FLOAT32) {
		MDA_WRITE_MACRO(int16_t,float)
	}
	else if (H->data_type==MDAIO_TYPE_INT16) {
		MDA_WRITE_MACRO_SAME(int16_t,int16_t)
	}
	else if (H->data_type==MDAIO_TYPE_INT32) {
		MDA_WRITE_MACRO(int16_t,int32_t)
	}
	else if (H->data_type==MDAIO_TYPE_UINT16) {
		MDA_WRITE_MACRO(int16_t,uint16_t)
	}
    else if (H->data_type==MDAIO_TYPE_FLOAT64) {
        MDA_WRITE_MACRO(int16_t,double)
    }
	else return 0;
}

long mda_write_int32(int32_t *data,struct MDAIO_HEADER *H,long n,FILE *output_file) {
	long i;
	if (H->data_type==MDAIO_TYPE_BYTE) {
		MDA_WRITE_MACRO(int32_t,unsigned char)
	}
	else if (H->data_type==MDAIO_TYPE_FLOAT32) {
		MDA_WRITE_MACRO(int32_t,float)
	}
	else if (H->data_type==MDAIO_TYPE_INT16) {
		MDA_WRITE_MACRO(int32_t,int16_t)
	}
	else if (H->data_type==MDAIO_TYPE_INT32) {
		MDA_WRITE_MACRO_SAME(int32_t,int32_t)
	}
	else if (H->data_type==MDAIO_TYPE_UINT16) {
		MDA_WRITE_MACRO(int32_t,uint16_t)
	}
    else if (H->data_type==MDAIO_TYPE_FLOAT64) {
        MDA_WRITE_MACRO(int32_t,double)
    }
	else return 0;
}

long mda_write_uint16(uint16_t *data,struct MDAIO_HEADER *H,long n,FILE *output_file) {
	long i;
	if (H->data_type==MDAIO_TYPE_BYTE) {
		MDA_WRITE_MACRO(uint16_t,unsigned char)
	}
	else if (H->data_type==MDAIO_TYPE_FLOAT32) {
		MDA_WRITE_MACRO(uint16_t,float)
	}
	else if (H->data_type==MDAIO_TYPE_INT16) {
		MDA_WRITE_MACRO(uint16_t,int16_t)
	}
	else if (H->data_type==MDAIO_TYPE_INT32) {
		MDA_WRITE_MACRO(uint16_t,int32_t)
	}
	else if (H->data_type==MDAIO_TYPE_UINT16) {
		MDA_WRITE_MACRO_SAME(uint16_t,uint16_t)
	}
    else if (H->data_type==MDAIO_TYPE_FLOAT64) {
        MDA_WRITE_MACRO(uint16_t,double)
    }
	else return 0;
}

long mda_write_float64(double *data,struct MDAIO_HEADER *H,long n,FILE *output_file) {
	long i;
	if (H->data_type==MDAIO_TYPE_BYTE) {
        MDA_WRITE_MACRO(double,unsigned char)
	}
	else if (H->data_type==MDAIO_TYPE_FLOAT32) {
        MDA_WRITE_MACRO(double,float)
	}
	else if (H->data_type==MDAIO_TYPE_INT16) {
        MDA_WRITE_MACRO(double,int16_t)
	}
	else if (H->data_type==MDAIO_TYPE_INT32) {
        MDA_WRITE_MACRO(double,int32_t)
	}
	else if (H->data_type==MDAIO_TYPE_UINT16) {
        MDA_WRITE_MACRO(double,uint16_t)
	}
    else if (H->data_type==MDAIO_TYPE_FLOAT64) {
        MDA_WRITE_MACRO_SAME(double,double)
    }
	else return 0;
}


void mda_copy_header(struct MDAIO_HEADER *ret,struct MDAIO_HEADER *X)
{
	long i;
	ret->data_type=X->data_type;
	ret->header_size=X->header_size;
	ret->num_bytes_per_entry=X->num_bytes_per_entry;
	ret->num_dims=X->num_dims;
	for (i=0; i<MDAIO_MAX_DIMS; i++) ret->dims[i]=X->dims[i];
}

void transpose_array(char *infile_path,char *outfile_path) {
	//open the files for reading/writing and declare some variables
	FILE *infile=fopen(infile_path,"rb");
	FILE *outfile=fopen(outfile_path,"wb");
	struct MDAIO_HEADER H;
	long M,N;
	long i,j;
	float *data_in,*data_out;

	if (!infile) return;
	if (!outfile) {fclose(infile); return;}

	//read the header
	mda_read_header(&H,infile);

	//if the data type is zero then there was a problem reading
	if (!H.data_type) {
		fclose(infile);
		fclose(outfile);
		return;
	}

	//get the dimensions and allocate the in/out arrays
	M=H.dims[0];
	N=H.dims[1];
	data_in=(float *)malloc(sizeof(float)*M*N);
	data_out=(float *)malloc(sizeof(float)*M*N);

	//Read the data -- note that we don't care what the actual type is.
	//This is a great trick!
	//See top of file for more info
	//Note that we could have decided to read only portions of the file if
	//N*M is too large for memory
	mda_read_float32(data_in,&H,M*N,infile);

	//Perform the transpose
	for (j=0; j<N; j++) {
		for (i=0; i<M; i++) {
			data_out[i+N*j]=data_in[j+M*i];
		}
	}

	//Swap the dimensions and write the output data
	H.dims[0]=N;
	H.dims[1]=M;
	mda_write_float32(data_out,&H,M*N,outfile);

	//clean up and we're done
	free(data_in);
	free(data_out);
	fclose(infile);
	fclose(outfile);
}
