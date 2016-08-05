#include "normalize_channels.h"
#include "diskreadmda.h"
#include "diskwritemda.h"
#include "mda.h"
#include "msprefs.h"
#include <math.h>
#include <QDebug>
#include <QTime>
#include "get_sort_indices.h"
#include "eigenvalue_decomposition.h"
#include <QVector>

Mda get_normalize_channelsing_matrix(Mda& COV);

bool normalize_channels(const QString& input, const QString& output)
{
    DiskReadMda X(input);
    long M = X.N1();
    long N = X.N2();
    if (!N) {
        qWarning() << "Input file does not exist: " + input;
        return false;
    }

    QVector<double> sumsqrs(M);
    for (int m = 0; m < M; m++)
        sumsqrs[m] = 0;
    long chunk_size = PROCESSING_CHUNK_SIZE;
    if ((N) && (N < PROCESSING_CHUNK_SIZE)) {
        chunk_size = N;
    }

    {
        QTime timer;
        timer.start();
        long num_timepoints_handled = 0;
#pragma omp parallel for
        for (long timepoint = 0; timepoint < N; timepoint += chunk_size) {
            Mda chunk;
#pragma omp critical(lock1)
            {
                X.readChunk(chunk, 0, timepoint, M, qMin(chunk_size, N - timepoint));
            }
            double* chunkptr = chunk.dataPtr();

            QVector<double> sumsqrs0(M);
            for (int m = 0; m < M; m++)
                sumsqrs0[m] = 0;

            long aa = 0;
            for (long i = 0; i < chunk.N2(); i++) {
                for (int m = 0; m < M; m++) {
                    sumsqrs0[m] += chunkptr[aa] * chunkptr[aa];
                    aa++;
                }
            }
#pragma omp critical(lock2)
            {
                for (int m = 0; m < M; m++) {
                    sumsqrs[m] += sumsqrs0[m];
                }
                num_timepoints_handled += qMin(chunk_size, N - timepoint);
                if ((timer.elapsed() > 5000) || (num_timepoints_handled == N)) {
                    printf("%ld/%ld (%d%%)\n", num_timepoints_handled, N, (int)(num_timepoints_handled * 1.0 / N * 100));
                    timer.restart();
                }
            }
        }
    }
    QVector<double> stdevs(M);
    for (int m = 0; m < M; m++) {
        double val = sumsqrs[m];
        if (N > 1)
            val /= N - 1;
        val = sqrt(val);
        if (!val)
            val = 1;
        stdevs[m] = val;
    }

    DiskWriteMda Y;
    Y.open(MDAIO_TYPE_FLOAT32, output, M, N);
    {
        QTime timer;
        timer.start();
        long num_timepoints_handled = 0;
#pragma omp parallel for
        for (long timepoint = 0; timepoint < N; timepoint += chunk_size) {
            Mda chunk_in;
#pragma omp critical(lock1)
            {
                X.readChunk(chunk_in, 0, timepoint, M, qMin(chunk_size, N - timepoint));
            }
            double* chunk_in_ptr = chunk_in.dataPtr();
            Mda chunk_out(M, chunk_in.N2());
            double* chunk_out_ptr = chunk_out.dataPtr();
            for (long i = 0; i < chunk_in.N2(); i++) {
                long aa = M * i;
                for (int m = 0; m < M; m++) {
                    chunk_out_ptr[aa + m] = chunk_in_ptr[aa + m] / stdevs[m];
                }
            }
#pragma omp critical(lock2)
            {
                Y.writeChunk(chunk_out, 0, timepoint);
                num_timepoints_handled += qMin(chunk_size, N - timepoint);
                if ((timer.elapsed() > 5000) || (num_timepoints_handled == N)) {
                    printf("%ld/%ld (%d%%)\n", num_timepoints_handled, N, (int)(num_timepoints_handled * 1.0 / N * 100));
                    timer.restart();
                }
            }
        }
    }
    Y.close();

    return true;
}

/*
    ComputerManager XX;

    // non open-mp implementation
    for (long timepoint=0; timepoint < N; timepoint += chunk_size) {
        Mda chunk;
        X.readChunk(chunk, 0, timepoint, M, qMin(chunk_size, N - timepoint));
        NC_Computer *C=new NC_Computer1;
        C->chunk=chunk;
        XX.addComputer(C);
    }

    for (int i=0; i<XX.computerCount(); i++) {
        NC_Computer *C=XX.computer(i);
        C->wait();
        for (int m=0; m<M; m++) {
            sumsqrs[m]+=C->sumsqrs[m];
        }
    }

    XX.clearComputers();

    DiskWriteMda Y;
    Y.open(MDAIO_TYPE_FLOAT32, output, M, N);

    for (long timepoint=0; timepoint < N; timepoint += chunk_size) {
        Mda chunk;
        X.readChunk(chunk, 0, timepoint, M, qMin(chunk_size, N - timepoint));
        NC_Computer *C=new NC_Computer2;
        C->chunk=chunk;
        XX.addComputer(C);
    }

    for (int i=0; i<XX.computerCount(); i++) {
        NC_Computer *C=XX.computer(i);
        C->wait();
        Y.writeChunk(C->chunk, 0, timepoint);
    }
*/
