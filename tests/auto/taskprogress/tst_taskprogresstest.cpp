#include <QString>
#include <QtTest>
#include "taskprogress/taskprogress.h"

class TaskProgressTest : public QObject {
    Q_OBJECT

public:
    TaskProgressTest();

private Q_SLOTS:
    void model_empty();
    void model_rowCount();

    void testCase1_data();
    void testCase1();
};

TaskProgressTest::TaskProgressTest()
{
}

void TaskProgressTest::model_empty()
{
    TaskManager::TaskProgressModel model;
    QVERIFY2(model.rowCount() == 0, "empty model should have no rows");
    QVERIFY2(model.index(0, 0) == QModelIndex(), "empty model should not return valid index");
}

void TaskProgressTest::model_rowCount()
{
    TaskManager::TaskProgressMonitor* monitor = TaskManager::TaskProgressMonitor::globalInstance();
}

void TaskProgressTest::testCase1_data()
{
    QTest::addColumn<QString>("data");
    QTest::newRow("0") << QString();
}

void TaskProgressTest::testCase1()
{
    QFETCH(QString, data);
    QVERIFY2(true, "Failure");
}

QTEST_MAIN(TaskProgressTest)

#include "tst_taskprogresstest.moc"
