#include <QString>
#include <QtTest>
#include "mlcommon.h"
#include <thread>
#include <future>

using VD = QVector<double>;

class MLComputeTest : public QObject {
    Q_OBJECT

public:
    MLComputeTest();

private Q_SLOTS:
    void min();
    void min_data();
    void min_benchmark();

    void sum();
    void sum_data();
    void sum_benchmark();
    void sum_benchmark_accumulate();

    void dotProduct();
    void dotProduct_data();
    void dotProduct_benchmark_vector();
    void dotProduct_benchmark_arrays();
    void dotProduct_benchmark_inner_product();

    void mean();
    void mean_data();
    void mean_benchmark();
    void mean_benchmark_alternative();

    void stdev();
    void stdev_data();
    void stdev_benchmark();
    void stdev_benchmark_alternative();

    //    void correlation();
    //    void correlation_data();
    void correlation_benchmark();
    void correlation_benchmark_alternative();
    void correlation_benchmark_parallel();
};

MLComputeTest::MLComputeTest()
{
}

void MLComputeTest::min()
{
    QFETCH(QVector<double>, data);
    QFETCH(double, result);
    double computed = MLCompute::min(data.size(), data.data());
    QCOMPARE(computed, result);
}

void MLComputeTest::min_data()
{
    QTest::addColumn<QVector<double> >("data");
    QTest::addColumn<double>("result");

    QTest::newRow("empty") << VD() << 0.0;
    QTest::newRow("single") << VD({ 42 }) << 42.0;
    QTest::newRow("negative") << VD({ -42 }) << -42.0;
    QTest::newRow("multi-different 1") << VD({ 24, 42 }) << 24.0;
    QTest::newRow("multi-different 2") << VD({ 42, 24 }) << 24.0;
    QTest::newRow("multi-same") << VD({ 42, 42 }) << 42.0;
}

void MLComputeTest::min_benchmark()
{
    VD data;
    data.reserve(10000);
    for (int i = 0; i < 10000; ++i) {
        data.append(-10000 + qrand());
    }
    QBENCHMARK
    {
        MLCompute::min(data.size(), data.data());
    }
}

void MLComputeTest::sum()
{
    QFETCH(QVector<double>, data);
    QFETCH(double, result);
    double computed = MLCompute::sum(data);
    QCOMPARE(computed, result);
}

void MLComputeTest::sum_data()
{
    QTest::addColumn<QVector<double> >("data");
    QTest::addColumn<double>("result");

    QTest::newRow("empty") << VD() << 0.0;
    QTest::newRow("single") << VD({ 42 }) << 42.0;
    QTest::newRow("negative") << VD({ -42 }) << -42.0;
    QTest::newRow("multiple") << VD({ 24, 42 }) << 66.0;
    QTest::newRow("inverse") << VD({ 24, -24 }) << 0.0;
    QTest::newRow("progression") << VD({ 1, -2, 3, -4, 5, -6, 7, -8 }) << -4.0;
}

void MLComputeTest::sum_benchmark()
{
    VD data;
    data.resize(10000);
    std::iota(data.begin(), data.end(), 1);
    QBENCHMARK
    {
        MLCompute::sum(data);
    }
}

void MLComputeTest::sum_benchmark_accumulate()
{
    VD data;
    data.resize(10000);
    std::iota(data.begin(), data.end(), 1);
    QBENCHMARK
    {
        std::accumulate(data.constBegin(), data.constEnd(), 0);
    }
}

void MLComputeTest::dotProduct()
{
    QFETCH(QVector<double>, data1);
    QFETCH(QVector<double>, data2);
    QFETCH(double, result);
    double computed = MLCompute::dotProduct(data1, data2);
    QCOMPARE(computed, result);
}

void MLComputeTest::dotProduct_data()
{
    QTest::addColumn<QVector<double> >("data1");
    QTest::addColumn<QVector<double> >("data2");
    QTest::addColumn<double>("result");

    QTest::newRow("empty") << VD() << VD() << 0.0;
    QTest::newRow("first-empty") << VD() << VD({ 0 }) << 0.0;
    QTest::newRow("second-empty") << VD({ 0 }) << VD() << 0.0;
    QTest::newRow("diff-size") << VD({ 1 }) << VD({ 1, 2, 3, 4 }) << 0.0;
    QTest::newRow("zero") << VD({ 0 }) << VD({ 0 }) << 0.0;
    QTest::newRow("first-zero") << VD({ 0, 0, 0 }) << VD({ 1, 2, 3 }) << 0.0;
    QTest::newRow("square") << VD({ 1, 1, 1 }) << VD({ 1, 1, 1 }) << 3.0;
}

void MLComputeTest::dotProduct_benchmark_vector()
{
    VD data;
    data.resize(10000);
    std::iota(data.begin(), data.end(), 1);
    QBENCHMARK
    {
        MLCompute::dotProduct(data, data);
    }
}

void MLComputeTest::dotProduct_benchmark_arrays()
{
    VD data;
    data.resize(10000);
    std::iota(data.begin(), data.end(), 1);
    QBENCHMARK
    {
        MLCompute::dotProduct(data.size(), data.data(), data.data());
    }
}

void MLComputeTest::dotProduct_benchmark_inner_product()
{
    VD data;
    data.resize(10000);
    std::iota(data.begin(), data.end(), 1);
    QBENCHMARK
    {
        std::inner_product(data.constBegin(), data.constEnd(), data.constBegin(), 0);
    }
}

void MLComputeTest::mean()
{
    QFETCH(VD, data);
    QFETCH(double, result);
    double computed = MLCompute::mean(data);
    QCOMPARE(computed, result);
}

void MLComputeTest::mean_data()
{
    QTest::addColumn<VD>("data");
    QTest::addColumn<double>("result");
    QTest::newRow("empty") << VD() << 0.0;
    QTest::newRow("single-zero") << VD( { 0 } ) << 0.0;
    QTest::newRow("single-non-zero") << VD( { 1 } ) << 1.0;
    QTest::newRow("same values") << VD( { 1, 1, 1, 1 } ) << 1.0;
    QTest::newRow("opposite values") << VD( { 2, -2, 2, -2, 2, -2 } ) << 0.0;
    QTest::newRow("integer") << VD( { 2, 4, 4, 4, 5, 5, 5, 7, 9 } ) << 5.0;
    QTest::newRow("real") << VD( { 2.2, 4.7, 3.5, 4.1, 4.7, -5.2, 2.1, 0, -1, 0.9 } ) << 1.6;
}

void MLComputeTest::mean_benchmark()
{
    VD data;
    data.resize(10000);
    std::iota(data.begin(), data.end(), 1);
    QBENCHMARK
    {
        MLCompute::mean(data);
    }
}

void MLComputeTest::mean_benchmark_alternative()
{
    VD data;
    data.resize(10000);
    std::iota(data.begin(), data.end(), 1);
    QBENCHMARK
    {
        (void)(std::accumulate(data.constBegin(), data.constEnd(), 0) / data.size());
    }
}

void MLComputeTest::stdev()
{
    QFETCH(VD, data);
    QFETCH(double, result);
    double computed = MLCompute::stdev(data);
    QCOMPARE(computed, result);
}

void MLComputeTest::stdev_data()
{
    QTest::addColumn<VD>("data");
    QTest::addColumn<double>("result");
    QTest::newRow("empty") << VD() << 0.0;
    QTest::newRow("single-zero") << VD( { 0 } ) << 0.0;
    QTest::newRow("single-non-zero") << VD( { 1 } ) << 0.0;
    QTest::newRow("integer") << VD( { 2, 4, 4, 4, 5, 5, 5, 7, 9 } ) << 2.0;
}

void MLComputeTest::stdev_benchmark()
{
    VD data;
    data.resize(10000);
    std::iota(data.begin(), data.end(), 1);
    QBENCHMARK
    {
        MLCompute::stdev(data);
    }
}

double stdev_helper(const VD& data)
{
    double sumsqr = std::inner_product(data.constBegin(), data.constEnd(), data.constBegin(), 0);
    double sum = std::accumulate(data.constBegin(), data.constEnd(), 0);
    const int ct = data.size();
    return sqrt((sumsqr - sum * sum / ct) / (ct - 1));
}

void MLComputeTest::stdev_benchmark_alternative()
{
    VD data;
    data.resize(10000);
    std::iota(data.begin(), data.end(), 1);
    QBENCHMARK
    {
        stdev_helper(data);
    }
}

void MLComputeTest::correlation_benchmark()
{
    VD data;
    data.resize(10000);
    std::iota(data.begin(), data.end(), 1);
    QBENCHMARK
    {
        MLCompute::correlation(data, data);
    }
}

double correlation_helper(const VD& data1, const VD& data2)
{
    if (data1.size() != data2.size())
        return 0;
    const size_t N = data1.size();
    const double sum1 = std::accumulate(data1.constBegin(), data1.constEnd(), 0);
    const double mean1 = sum1 / N;
    const double sumsqr1 = std::inner_product(data1.constBegin(), data1.constEnd(), data1.constBegin(), 0);
    const double stdev1 = N > 1 ? sqrt((sumsqr1 - sum1 * sum1 / N) / (N - 1)) : 0;

    const double sum2 = std::accumulate(data2.constBegin(), data2.constEnd(), 0);
    const double mean2 = sum2 / N;
    const double sumsqr2 = std::inner_product(data2.constBegin(), data2.constEnd(), data2.constBegin(), 0);
    const double stdev2 = sqrt((sumsqr2 - sum2 * sum2 / N) / (N - 1));
    if (!stdev1 || !stdev2)
        return 0;
    VD Y1;
    Y1.reserve(N);
    std::transform(data1.constBegin(), data1.constEnd(), Y1.begin(), [mean1, stdev1](double val) { return (val - mean1)/stdev1; });
    VD Y2;
    Y2.reserve(N);
    std::transform(data2.constBegin(), data2.constEnd(), Y2.begin(), [mean2, stdev2](double val) { return (val - mean2)/stdev2; });
    return std::inner_product(Y1.constBegin(), Y1.constEnd(), Y2.constBegin(), 0);
}

struct corr_intermediate {
    double sum;
    double mean;
    double sumsqr;
    double stdev;
    VD Y;
};

void correlation_parallel_helper(VD::const_iterator first,
    VD::const_iterator last,
    std::promise<corr_intermediate> accumulate_promise)
{
    const size_t N = last - first;
    corr_intermediate result;
    result.sum = std::accumulate(first, last, 0);
    result.mean = result.sum / N;
    result.sumsqr = std::inner_product(first, last, first, 0);
    result.stdev = N > 1 ? sqrt((result.sumsqr - result.sum * result.sum / N) / (N - 1)) : 0;
    if (result.stdev) {
        result.Y.reserve(N);
        std::transform(first, last, result.Y.begin(), [result](double val) { return (val - result.mean)/result.stdev; });
    }
    accumulate_promise.set_value(result); // Notify future
}

double correlation_parallel(const VD& data1, const VD& data2)
{
    if (data1.size() != data2.size())
        return 0;
    std::promise<corr_intermediate> sum1_promise;
    std::future<corr_intermediate> sum1_future = sum1_promise.get_future();
    std::thread sum1_thread(
        correlation_parallel_helper,
        data1.begin(), data1.end(),
        std::move(sum1_promise));
    std::promise<corr_intermediate> sum2_promise;
    std::future<corr_intermediate> sum2_future = sum2_promise.get_future();
    std::thread sum2_thread(
        correlation_parallel_helper,
        data2.begin(), data2.end(),
        std::move(sum2_promise));

    corr_intermediate itm1 = std::move(sum1_future.get());
    corr_intermediate itm2 = std::move(sum2_future.get());
    sum1_thread.join();
    sum2_thread.join();
    if (!itm1.stdev || !itm2.stdev)
        return 0;
    return std::inner_product(itm1.Y.constBegin(), itm1.Y.constEnd(), itm2.Y.constBegin(), 0);
}

void MLComputeTest::correlation_benchmark_alternative()
{
    VD data;
    data.resize(10000);
    std::iota(data.begin(), data.end(), 1);
    QBENCHMARK
    {
        correlation_helper(data, data);
    }
}

void MLComputeTest::correlation_benchmark_parallel()
{
    VD data;
    data.resize(10000);
    std::iota(data.begin(), data.end(), 1);
    QBENCHMARK
    {
        correlation_parallel(data, data);
    }
}

QTEST_APPLESS_MAIN(MLComputeTest)

#include "tst_mlcomputetest.moc"
