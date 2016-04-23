#include "testMdaIO.h"
#include "mdaio.h"
#include <stdio.h>
#include <cstring>
#include <QTemporaryFile>

void TestMdaIO::testHeader() {
	struct MDAIO_HEADER header, headerCopy;
	header.data_type = MDAIO_TYPE_FLOAT32;
	const size_t dimCount = sizeof(header.dims)/sizeof(header.dims[0]);
	for (size_t i=0 ; i<dimCount ; ++i)
		header.dims[i] = 1 + i%5;
	header.num_dims = dimCount / 2;
	header.num_bytes_per_entry = 4;
	mda_copy_header(&headerCopy, &header);
	QVERIFY(std::memcmp(&header, &headerCopy, sizeof(header)) == 0);
	QTemporaryFile file;
	QVERIFY(file.open());
	FILE* handle = fdopen(file.handle(), "r+b");
	QVERIFY(handle);
	QVERIFY(mda_write_header(&headerCopy, handle));
	std::memset(&header, 0, sizeof(header));
	rewind(handle);
	QVERIFY(mda_read_header(&header, handle));
	QCOMPARE(header.data_type, headerCopy.data_type);
	QCOMPARE(header.num_dims, headerCopy.num_dims);
	QCOMPARE(header.num_bytes_per_entry, headerCopy.num_bytes_per_entry);
	QCOMPARE(header.header_size, headerCopy.header_size);
	for (int i=0 ; i<header.num_dims ; ++i)
		QCOMPARE(header.dims[i], headerCopy.dims[i]);
	file.close();
	fclose(handle);
}
