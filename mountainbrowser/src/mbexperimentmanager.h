/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 4/6/2016
*******************************************************/

#ifndef MBEXPERIMENTMANAGER_H
#define MBEXPERIMENTMANAGER_H

#include <QJsonObject>
#include <QStringList>


struct MBExperiment {
  QString id;
  QJsonObject json;
};

class MBExperimentManagerPrivate;
class MBExperimentManager
{
public:
  friend class MBExperimentManagerPrivate;
  MBExperimentManager();
  virtual ~MBExperimentManager();
  void loadExperiments(const QString &json_str);
  void addExperiment(const MBExperiment &E);
  QStringList allExperimentIds() const;
  MBExperiment experiment(const QString &id);
private:
  MBExperimentManagerPrivate *d;
};

#endif // MBEXPERIMENTMANAGER_H

