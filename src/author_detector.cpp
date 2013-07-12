#include "author_detector.h"
#include <QFileDialog>

author_detector::author_detector(QWidget *parent, Qt::WFlags flags)
	: QMainWindow(parent, flags), processThread(0)
{
	ui.setupUi(this);
	setFixedSize(size());
	model = new Model();
	model->setParent(this);
	processThread.model = model;

	progressDialog = new QProgressDialog("Operation in progress.", "Cancel", 0, 100, this, Qt::WindowCloseButtonHint);
	progressDialog->setWindowTitle("Please Wait");

	QObject::connect(&processThread, SIGNAL(enableView(bool)),
		this, SLOT(enable(bool)));
	QObject::connect(&processThread, SIGNAL(saveModel()),
		this, SLOT(onSave()));
	QObject::connect(&processThread, SIGNAL(loadModel()),
		this, SLOT(onLoad()));
//	QObject::connect(&processThread, SIGNAL(finished()), &processThread, SLOT(deleteLater()));
//	QObject::connect(model, SIGNAL(finished()), &processThread, SLOT(deleteLater()));
	QObject::connect(&processThread, SIGNAL(stop()), this, SLOT(onStop()));
	QObject::connect(this, SIGNAL(changeProgressDialog(int, QProgressDialog *)),
		&processThread, SLOT(onChangeProgressDialog(int, QProgressDialog *)));
	QObject::connect(progressDialog, SIGNAL(canceled()), this, SLOT(ProgressDialogCancel()));
}

void author_detector::setProgressDialogValue(int procent)
{
	changeProgressDialog(procent, progressDialog);
}

author_detector::~author_detector()
{
	delete model;
	delete progressDialog;
}

void author_detector::onSave()
{
	if (!model->saveModel())
	{
		QMessageBox::critical(0, "Error", "Can't save model");
	}
}

void author_detector::onDirBrowse()
{
	QString dirName = QFileDialog::getExistingDirectory(this, tr("Choose directory"), QDir::currentPath());
	if (!dirName.isNull())
	{
		ui.dirLineEdit->setText(dirName + "\\");
		model->setDirName(dirName);
	}
}

void author_detector::onModelFileBrowse()
{
	QString fileName = QFileDialog::getOpenFileName(this, tr("Choose model file"), QDir::currentPath());
	if (!fileName.isNull())
		ui.modelLineEdit->setText(fileName);
	model->setModelFileName(fileName);
}

void author_detector::onModelFileBrowse2()
{
	QString fileName = QFileDialog::getOpenFileName(this, tr("Choose model file"), QDir::currentPath());
	if (!fileName.isNull())
		ui.modelLineEdit_2->setText(fileName);
	model->setModelFileName(fileName);
}


void author_detector::onTrueFileBrowse()
{
	QString fileName = QFileDialog::getOpenFileName(this, tr("Choose true text's authors file"), QDir::currentPath());
	if (!fileName.isNull())
		ui.trueLineEdit->setText(fileName);
	model->setTrueLocationFileName(fileName);
}

void author_detector::onTextFileBrowse()
{
	QString fileName = QFileDialog::getOpenFileName(this, tr("Choose text file"), QDir::currentPath());
	if (!fileName.isNull())
		ui.textLineEdit->setText(fileName);
	model->setTextFileName(fileName);
}

void author_detector::onPredictedFileBrowse()
{
	QString fileName = QFileDialog::getOpenFileName(this, tr("Choose predicted text's authors file"), QDir::currentPath());
	if (!fileName.isNull())
		ui.predictedLineEdit->setText(fileName);
	model->setUserLocationFileName(fileName);
}

void author_detector::onTrain()
{
	if (processThread.isRunning())
		while(!processThread.isFinished()){}
	fl_ex = 0;

	model->setDirName(ui.dirLineEdit->text());
	model->setModelFileName(ui.modelLineEdit->text());

	QDir dir(model->getDirName());
	if (!dir.exists())
	{
		QMessageBox::critical(0, "Error", "Can't find train directory");
		return;
	}
	if (model->getModelFileName() == "")
	{
		QMessageBox::critical(0, "Error", "Wrong model file name");
		return;
	}
	
	model->setTripletCount(ui.spinBox->value());

	progressDialog->show();
	processThread.pr_name = TRAIN;
	processThread.start();
}

void author_detector::onClassify()
{	
	if (processThread.isRunning())
		while(!processThread.isFinished()){}
	fl_ex = 0;

	model->setDirName(ui.dirLineEdit->text());
	model->setModelFileName(ui.modelLineEdit->text());
	model->setUserLocationFileName(ui.predictedLineEdit->text());

	QDir dir(model->getDirName());
	if (!dir.exists())
	{
		QMessageBox::critical(0, "Error", "Can't find classify directory");
		return;
	}
	if (model->getUserLocationFileName() == "")
	{
		QMessageBox::critical(0, "Error", "Wrong file name for prediction");
		return;
	}

	if (!model->loadModel())
	{
		QMessageBox::critical(0, "Error", "Can't load model");
		return;
	}
	progressDialog->show();
	processThread.pr_name = CLASSIFY;
	processThread.start();
}

void author_detector::onPredict()
{	
	if (processThread.isRunning())
		while(!processThread.isFinished()){}
	fl_ex = 0;

	model->setModelFileName(ui.modelLineEdit_2->text());
	model->setTextFileName(ui.textLineEdit->text());

	if (!model->loadModel())
	{
		QMessageBox::critical(0, "Error", "Can't load model");
		return;
	}

	QFile file(model->getTextFileName());
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		QMessageBox::critical(0, "Error", "Prediction error");
		return;
	}
	QTextStream in(&file);
	QString s = in.readAll();
	ui.textEdit->setText(s);

	QString sres;
	if (model->predictAuthor(sres))
	{
		ui.label_11->setText(sres);
	} else
	{
		QMessageBox::critical(0, "Error", "Prediction error");
		return;
	}
}


void author_detector::onEstimate()
{	
	model->setUserLocationFileName(ui.predictedLineEdit->text());
	model->setTrueLocationFileName(ui.trueLineEdit->text());

	if (model->getUserLocationFileName() == "")
	{
		QMessageBox::critical(0, "Error", "Wrong prediction file name");
		return;
	}
	if (model->getTrueLocationFileName() == "")
	{
		QMessageBox::critical(0, "Error", "Wrong answer file name");
		return;
	}
	if (model->estimateQuality())
	{
		ui.label_6->setText(QString::number(model->getPrecision()));
	}
}

void author_detector::enable(bool fl)
{
	ui.tabWidget->setEnabled(fl);
}

void QProcessThread::onChangeProgressDialog(int c, QProgressDialog *p)
{
	p->setValue(c);
}

void QProcessThread::run()
{
	switch (pr_name)
	{
	case TRAIN:
		{
			enableView(false);
			model->sampleAndTrain();
			saveModel();
			enableView(true);
			break;
		}
	case CLASSIFY:
		{
			enableView(false);
			model->classifyDirectory();
			enableView(true);
			break;
		}
	}
}

void author_detector::onStop()
{
	fl_ex = 1;
}
bool author_detector::getExitFlag()
{
	return fl_ex;
}


void QProcessThread::finish()
{
	stop();
}

void author_detector::ProgressDialogCancel()
{
	ui.tabWidget->setEnabled(true);
	if (processThread.isRunning())
	{
		processThread.finish();// terminate();
	}
}

void author_detector::closeEvent(QCloseEvent *event)
{
	if (processThread.isRunning())
	{
		processThread.finish();// terminate();
		while(!processThread.isFinished()){}
	}
}
