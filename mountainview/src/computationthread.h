/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 3/31/2016
*******************************************************/

#ifndef COMPUTATIONTHREAD
#define COMPUTATIONTHREAD

#include <QThread>
#include <QString>

class ComputationThreadPrivate;
class ComputationThread : public QThread {
    Q_OBJECT
public:
    friend class ComputationThreadPrivate;
    ComputationThread();
    virtual ~ComputationThread();

    virtual void compute() = 0;

    void setDeleteOnComplete(bool val);
    void startComputation(); //will stop existing computation
    void stopComputation(); //will wait for stop before returning
    bool isComputing();
    bool isFinished();
    bool hasError();
    QString errorMessage();

signals:
    void computationFinished();

protected:
    bool stopRequested();
    void setErrorMessage(const QString& error);

private:
    void run();
private slots:
    void slot_start();

private:
    ComputationThreadPrivate* d;
};

#endif // COMPUTATIONTHREAD
