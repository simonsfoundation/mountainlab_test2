#include "testMda.h"
#include "testMdaIO.h"
#include "testBandpassFilter.h"

template <typename TestClass>
int runTest(int argc, char** argv)
{
    TestClass test;
    const int result = QTest::qExec(&test, argc, argv);
    if (result != 0)
        exit(result);
    return result;
}

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);
    runTest<TestMda>(argc, argv);
    runTest<TestMdaIO>(argc, argv);
    runTest<TestBandpassFilter>(argc, argv);
    return 0;
}
