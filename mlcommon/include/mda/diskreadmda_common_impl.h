/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 8/4/2016
*******************************************************/

#ifndef DISKREADMDA_COMMON_IMPL_H
#define DISKREADMDA_COMMON_IMPL_H

#include <stdio.h>
#include "mdaio.h"
#include "mda.h"
#include "remotereadmda.h"

class DiskReadMda_Common_Impl {
public:
    FILE* m_file;
    bool m_file_open_failed;
    bool m_header_read;
    MDAIO_HEADER m_header;
    bool m_reshaped;
    long m_mda_header_total_size;
    Mda32 m_internal_chunk_32;
    Mda m_internal_chunk_64;
    long m_current_internal_chunk_index;
    Mda32 m_memory_mda_32;
    Mda m_memory_mda_64;
    bool m_use_memory_mda_32 = false;
    bool m_use_memory_mda_64 = false;

#ifdef USE_REMOTE_READ_MDA
    bool m_use_remote_mda;
    RemoteReadMda m_remote_mda;
#endif

    QString m_path;

    void copy_from(DiskReadMda_Common_Impl* other);
    void construct_and_clear();
    bool read_header_if_needed();
    bool open_file_if_needed();
    long total_size();
    void set_remote_data_type(QString dtype);
    void set_download_chunk_size(long size);
    long download_chunk_size();
    QString make_path();
    long dim(int num);
    bool reshape(int N1b, int N2b, int N3b = 1, int N4b = 1, int N5b = 1, int N6b = 1);
    double value64(long i);
    bool read_chunk_64(Mda& X, long i, long size);
    bool read_chunk_64(Mda& X, long i1, long i2, long size1, long size2);
    bool read_chunk_64(Mda& X, long i1, long i2, long i3, long size1, long size2, long size3);
};

#endif // DISKREADMDA_COMMON_IMPL_H
