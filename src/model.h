#ifndef MODEL_H
#define MODEL_H

#include <QObject>
#include <QVector>
#include <QFile>
#include <QDir>
#include <QMessageBox>
#include <QTextStream>
#include "liblinear-ovo-1.93/linear.h"
#include "author.h"

class author_detector;

class Model: public QObject
{
	Q_OBJECT

public:
	Model();
	~Model();

	bool saveModel();
	bool loadModel();
	void sampleAndTrain();
	void classifyDirectory();
	bool estimateQuality();
	bool predictAuthor(QString &sres);

	void setTrueLocationFileName(const QString &fileName);
	void setTextFileName(const QString &fileName);
	void setUserLocationFileName(const QString &fileName);
	void setModelFileName(const QString &fileName);
	void setDirName(const QString &dirName);
	void setParent(author_detector *p);
	void setTripletCount(int c);

	QString getTrueLocationFileName();
	QString getTextFileName();
	QString getUserLocationFileName();
	QString getModelFileName();    
	QString getDirName();
	double getPrecision();
	double getRecall();
	double getFScore();
	int getTripletCount();

private:
	void sample();
	void trainModel();
	int detect(const QVector<double> &params);
	QVector<double> getParams(const QVector<QPair<Triplet, double> > &c);

	QVector<Triplet> triplets;
	int tr_count;
	QVector<QString> author_names;

	QVector< QVector<double> > m_features;
	QVector<int> m_labels;

	int m_featuresNumber;
	int m_instancesNumber;
	int m_classNumber;
	struct model* m_classifier;
	double m_paramC;

	double m_precision;
	double m_recall;
	double m_fScore;
	QString m_trueLocationFileName;
	QString m_textFileName;
	QString m_userLocationFileName;
	QString m_modelFileName;
	QString m_dirName;
	author_detector *parent;

signals:
	void finished();
	
};


#endif // MODEL_H