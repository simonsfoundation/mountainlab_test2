/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 8/5/2016
*******************************************************/

#include "synthesize1.h"
#include "mda.h"
#include <random>

#include <mda32.h>

double rand_uniform(double a, double b);
long rand_int(long a, long b);
void generate_randn(size_t N, float *X);

bool synthesize1(const QString& waveforms_in_path, const QString& info_in_path, const QString& timeseries_out_path, const synthesize1_opts& opts)
{
    long N = opts.N;
    double noise_level = opts.noise_level;

    Mda W(waveforms_in_path);
    Mda info(info_in_path);
    int M = W.N1();
    int T = W.N2();
    int K = W.N3();

    Mda32 X(M, N);
    generate_randn(X.totalSize(),X.dataPtr());
    for (long n = 0; n < N; n++) {
        for (long m = 0; m < M; m++) {
            double val=X.get(m,n);
            val=val*noise_level;
            X.set(val, m, n);
        }
    }

    for (int k = 0; k < K; k++) {
        long pop = (long)info.value(0, k);
        printf("k=%d, pop=%ld\n", k, pop);
        for (long a = 0; a < pop; a++) {
            long t0 = rand_int(0, N - T - 1);
            for (int t = 0; t < T; t++) {
                for (int m = 0; m < M; m++) {
                    X.setValue(X.value(m, t0 + t) + W.value(m, t, k), m, t0 + t);
                }
            }
        }
    }

    return X.write32(timeseries_out_path);
}

long rand_int(long a, long b)
{
    if (b <= a)
        return a;
    return a + (qrand() % (b - a + 1));
}

double rand_uniform(double a, double b)
{
    double val = (qrand() + 0.5) / RAND_MAX;
    return a + val * (b - a);
}

void generate_randn(size_t N, float *X) {
    std::random_device rd; //to generate a non-deterministic integer (I believe)
    std::mt19937 e2(rd()); //the random number generator, seeded by rd() (I believe)
    std::normal_distribution<> dist(0, 1); //to generate random numbers from normal distribution
    for (size_t n=0; n<N; n++) {
        X[n]=dist(e2);
    }
}
