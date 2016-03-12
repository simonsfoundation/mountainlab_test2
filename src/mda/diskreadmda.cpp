#include "diskreadmda.h"
#include <stdio.h>
#include "mdaio.h"

#define MAX_PATH_LEN 10000
#define DEFAULT_CHUNK_SIZE 10e6

class DiskReadMdaPrivate
{
public:
	DiskReadMda *q;
	FILE *m_file;
	bool m_file_open_failed;
	MDAIO_HEADER m_header;
	Mda m_internal_chunk;
	long m_current_internal_chunk_index;

	char m_path[MAX_PATH_LEN];
	void do_construct();
	bool open_file_if_needed();
};

DiskReadMda::DiskReadMda() {
	d=new DiskReadMdaPrivate;
	d->q=this;
	d->do_construct();
}

DiskReadMda::DiskReadMda(const DiskReadMda &other)
{
	d=new DiskReadMdaPrivate;
	d->q=this;
	d->do_construct();
	setPath(other.d->m_path);
}

DiskReadMda::~DiskReadMda() {
	if (d->m_file) fclose(d->m_file);
	delete d;
}

void DiskReadMda::setPath(const QString &file_path)
{
	setPath(file_path.toLatin1().data());
}

void DiskReadMda::setPath(const char *file_path)
{
	strcpy(d->m_path,file_path);
	if (d->m_file) {
		fclose(d->m_file);
		d->m_file=0;
	}
}

long DiskReadMda::N1() const
{
	if (!d->open_file_if_needed()) return 0;
	return d->m_header.dims[0];
}

long DiskReadMda::N2() const
{
	if (!d->open_file_if_needed()) return 0;
	return d->m_header.dims[1];
}

long DiskReadMda::N3() const
{
	if (!d->open_file_if_needed()) return 0;
	return d->m_header.dims[2];
}

long DiskReadMda::N4() const
{
	if (!d->open_file_if_needed()) return 0;
	return d->m_header.dims[3];
}

long DiskReadMda::N5() const
{
	if (!d->open_file_if_needed()) return 0;
	return d->m_header.dims[4];
}

long DiskReadMda::N6() const
{
	if (!d->open_file_if_needed()) return 0;
	return d->m_header.dims[5];
}

bool DiskReadMda::readChunk(Mda &X, long i, long size) const
{
	if (!d->open_file_if_needed()) return false;
	X.allocate(1,size);
	fseek(d->m_file,d->m_header.header_size+d->m_header.num_bytes_per_entry*i,SEEK_SET);
	long bytes_read=mda_read_float64(X.dataPtr(),&d->m_header,size,d->m_file);
	if (bytes_read!=size) {
		printf("Warning: Problem reading 1d chunk in diskreadmda: %ld<>%ld\n",bytes_read,size);
		return false;
	}
	return true;
}

bool DiskReadMda::readChunk(Mda &X, long i1, long i2, long size1, long size2) const
{
	if (!d->open_file_if_needed()) return false;
	if (size1==N1()) {
		//easy case
		X.allocate(size1,size2);
		fseek(d->m_file,d->m_header.header_size+d->m_header.num_bytes_per_entry*(i1+N1()*i2),SEEK_SET);
		long bytes_read=mda_read_float64(X.dataPtr(),&d->m_header,size1*size2,d->m_file);
		if (bytes_read!=size1*size2) {
			printf("Warning problem reading 2d chunk in diskreadmda: %ld<>%ld\n",bytes_read,size1*size2);
			return false;
		}
		return true;
	}
	else {
		printf("Warning: This case not yet supported (diskreadmda::readchunk 2d).\n");
		return false;
	}
}

bool DiskReadMda::readChunk(Mda &X, long i1, long i2, long i3, long size1, long size2, long size3) const
{
	if (!d->open_file_if_needed()) return false;
	if ((size1==N1())&&(size2==N2())) {
		//easy case
		X.allocate(size1,size2,size3);
		fseek(d->m_file,d->m_header.header_size+d->m_header.num_bytes_per_entry*(i1+N1()*i2+N1()*N2()*i3),SEEK_SET);
		long bytes_read=mda_read_float64(X.dataPtr(),&d->m_header,size1*size2*size3,d->m_file);
		if (bytes_read!=size1*size2*size3) {
			printf("Warning problem reading 3d chunk in diskreadmda: %ld<>%ld\n",bytes_read,size1*size2*size3);
			return false;
		}
		return true;
	}
	else {
		printf("Warning: This case not yet supported (diskreadmda::readchunk 3d).\n");
		return false;
	}
}

double DiskReadMda::value(long i) const
{
	if ((i<0)||(i>=N1())) return 0;
	long chunk_index=i/DEFAULT_CHUNK_SIZE;
	long offset=i-DEFAULT_CHUNK_SIZE*chunk_index;
	if (d->m_current_internal_chunk_index!=chunk_index) {
		this->readChunk(d->m_internal_chunk,chunk_index,DEFAULT_CHUNK_SIZE);
		d->m_current_internal_chunk_index=chunk_index;
	}
	return d->m_internal_chunk.value(offset);
}

double DiskReadMda::value(long i1, long i2) const
{
	if ((i1<0)||(i1>=N1())) return 0;
	if ((i2<0)||(i2>=N2())) return 0;
	return value(i1+N1()*i2);
}

double DiskReadMda::value(long i1, long i2, long i3) const
{
	if ((i1<0)||(i1>=N1())) return 0;
	if ((i2<0)||(i2>=N2())) return 0;
	if ((i3<0)||(i3>=N3())) return 0;
	return value(i1+N1()*i2+N1()*N2()*i3);
}

void DiskReadMdaPrivate::do_construct()
{
	strcpy(m_path,"");
	m_file_open_failed=false;
	m_file=0;
	m_current_internal_chunk_index=-1;
}

bool DiskReadMdaPrivate::open_file_if_needed()
{
	if (m_file) return true;
	if (m_file_open_failed) return false;
	m_file=fopen(m_path,"rb");
	if (m_file) {
		mda_read_header(&m_header,m_file);
	}
	else {
		printf("Failed to open diskreadmda file: %s\n",m_path);
		m_file_open_failed=true;
		return false;
	}
	return true;
}
