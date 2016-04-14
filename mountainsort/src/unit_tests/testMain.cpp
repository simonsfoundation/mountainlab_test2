#include "testMda.h"

int main(int argc, char** argv) {
	TestMda mda;
	QTest::qExec(&mda, argc, argv);
	return 0;
}
