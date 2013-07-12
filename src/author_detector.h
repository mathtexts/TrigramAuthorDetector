#ifndef AUTHOR_DETECTOR_H
#define AUTHOR_DETECTOR_H

#include <QtGui/QMainWindow>
#include <QThread>
#include <QProgressDialog>
#include "ui_author_detector.h"
#include "model.h"

enum ProcessName{TRAIN, CLASSIFY};

class QProcessThread : public QThread
{
	Q_OBJECT

		friend class author_detector;
		Model* model;

protected:
	void run();

signals:
	void enableView(bool fl);
	void saveModel();
	void stop();

private slots:
	void onChangeProgressDialog(int c, QProgressDialog *p);
	void finish();

public:
	ProcessName pr_name;
	QProcessThread(Model *in_model) : model(in_model) {};
};

class author_detector : public QMainWindow
{
	Q_OBJECT

public:
	author_detector(QWidget *parent = 0, Qt::WFlags flags = 0);
	~author_detector();
	void setProgressDialogValue(int procent);
	bool getExitFlag();

private:
	Ui::author_detectorClass ui;
	Model *model;
	bool fl_ex;
	QProcessThread processThread;
	QProgressDialog *progressDialog;
	void closeEvent(QCloseEvent *event);

signals:
	void changeProgressDialog(int, QProgressDialog *p);

private slots:
	void onDirBrowse();
	void onModelFileBrowse();
	void onTrueFileBrowse();
	void onPredictedFileBrowse();
	void onTrain();
	void onClassify();
	void onEstimate();
	void onSave();
	void enable(bool fl);
	void ProgressDialogCancel();
	void onStop();
	
	void onModelFileBrowse2();
	void onTextFileBrowse();
	void onPredict();

};

#endif // AUTHOR_DETECTOR_H
