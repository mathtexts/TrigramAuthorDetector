#include <model.h>
#include <time.h>
#include <cstdlib>
#include <iostream>
#include <algorithm>
#include "author_detector.h"

using namespace std;

#define Malloc(type,n) (type *)malloc((n)*sizeof(type))

Model::Model()
{
	tr_count = 0;
    m_featuresNumber = 0;
    m_instancesNumber = 0;
    m_precision = 0.0;
    m_recall = 0.0;
    m_fScore = 0.0;
	m_paramC = 0.1; 
    m_dirName = "trainingSet/";
    m_trueLocationFileName = "";
    m_userLocationFileName = "predicted.txt";
    m_modelFileName = "model.txt";
    m_classifier = NULL;
}

Model::~Model()
{
    if (m_classifier)
        free_and_destroy_model(&m_classifier);
}

void Model::setTrueLocationFileName(const QString& fileName)
{
    m_trueLocationFileName = fileName;
}

void Model::setTextFileName(const QString& fileName)
{
	m_textFileName = fileName;
}

void Model::setUserLocationFileName(const QString& fileName)
{
    m_userLocationFileName = fileName;
}

void Model::setModelFileName(const QString& fileName)
{
    m_modelFileName = fileName;
}

void Model::setDirName(const QString& dirName)
{
    m_dirName = dirName;
}

QString Model::getTrueLocationFileName()
{
    return m_trueLocationFileName;
}

QString Model::getTextFileName()
{
	return m_textFileName;
}

QString Model::getUserLocationFileName()
{
    return m_userLocationFileName;
}

QString Model::getModelFileName()
{
    return m_modelFileName;
}

QString Model::getDirName()
{
    return m_dirName;
}

double Model::getPrecision()
{
    return m_precision;
}

double Model::getRecall()
{
    return m_recall;
}

double Model::getFScore()
{
    return m_fScore;
}

void Model::setParent(author_detector *p)
{
	parent = p;
}

int Model::getTripletCount()
{
	return tr_count;
}

void Model::setTripletCount(int c)
{
	tr_count = c;
}


bool Model::saveModel()
{
    // Save model
	if (!m_classifier)
		return false;
	if (parent->getExitFlag())
		return true;
    QByteArray ba = m_modelFileName.toLocal8Bit();
    if (save_model(ba.data(), m_classifier))
        return false;
    else
	{
		QFile file(m_modelFileName);
		if (!file.open(QIODevice::Append | QIODevice::Text))
			return false;
		QTextStream out(&file);
		out << "triplets" << endl;
		out << triplets.size() << endl;
		for (int i = 0; i < triplets.size(); i++)
			out << triplets[i].get(0) << ' ' << triplets[i].get(1) << ' ' << triplets[i].get(2) << endl;
		for (int i = 0; i < m_classifier->nr_class; i++)
			out << author_names[i] << endl;
		file.close();
        return true;
	}
}

bool Model::loadModel()
{
    // Load model
    QByteArray ba = m_modelFileName.toLocal8Bit();
    if ((m_classifier = load_model(ba.data())) == 0)
        return false;

    // Read number of features from model file
    QFile file(m_modelFileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;
    QTextStream in(&file);
    QString str;
	do
		in >> str;
	while (str.compare("nr_class"));
	in >> m_classNumber;
    do
        in >> str;
    while (str.compare("nr_feature"));
    in >> m_featuresNumber;
    
	do
		in >> str;
	while (str.compare("triplets"));

	int m;
	in >> m;
	triplets.clear();
	for (int i = 0; i < m; i++)
	{
		int c1, c2, c3;
		in >> c1 >> c2 >> c3;
		triplets.push_back(Triplet(c1, c2, c3));
	}
	for (int i = 0; i < m_classNumber; i++)
	{
		QString s;
		in >> s;
		author_names.push_back(s);
	}
	file.close();

	return true;
}

QVector<double> Model::getParams(const QVector<QPair<Triplet, double> > &c)
{
	QVector<double> res;

	for (int j = 0; j < triplets.size(); j++)
	{
		int l = 0, r = c.size();
		while (r > l + 1)
		{
			int m = (l + r) / 2;
			if (triplets[j] < c[m].first)
				r = m;
			else
				l = m;
		}
		int c1 = 0;
		if (c.size())
			if (triplets[j] == c[l].first)
				c1 = c[l].second;
		res.push_back(c1);
	}
	return res;
}

void Model::sample()
{
	QDir dir(m_dirName);
	dir.setFilter(QDir::Dirs | QDir::Hidden | QDir::NoSymLinks);
	QFileInfoList list = dir.entryInfoList();
	int k = 0;
	Spec specs;
	double step = 100 / (list.size() - 2);
	double proc = 0;
	QVector<QVector<QPair<Triplet, double> > > text_triplets;
	triplets.clear();
	for (QFileInfoList::iterator j = list.begin(); j != list.end(); ++j) 
	{
		proc = k * step;
		parent->setProgressDialogValue(proc);
		QFileInfo fileInfo = *j;

		if (fileInfo.fileName() == "." || fileInfo.fileName() == "..")
			continue;
		QDir dir1(m_dirName + fileInfo.fileName());
		author_names.push_back(fileInfo.fileName());
		dir1.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
		QFileInfoList list = dir1.entryInfoList();
		double step1 = step / (list.size());
		int k1 = 0;
		QVector<Triplet> cur_author_triplets;
		QVector<QPair<Triplet, double> > cur_text_triplets;
		for (QFileInfoList::iterator j1 = list.begin(); j1 != list.end(); ++j1) 
		{
			QFileInfo fileInfo = *j1;
			QFile file(fileInfo.filePath());
			file.open(QIODevice::ReadOnly | QIODevice::Text);
			QTextStream in(&file);
			QString s = "";
			while (!in.atEnd())
			{
				QString s1 = in.readLine();
				if (!s1.length())
					s1 = '.';
				s += s1 + " ";
			}
			AText cur_text = AText(specs, s);
			text_triplets.push_back(cur_text.getCountTriplets());
			QVector<Triplet> cur_tr = cur_text.getBestTriplets(2 * tr_count);
//			for (int i = 0; i < cur_tr.size(); i++)
//				triplets.push_back(cur_tr[i]);
			for (int i = 0; i < cur_tr.size(); i++)
			{
				cur_author_triplets.push_back(cur_tr[i]);
				cur_text_triplets.push_back(text_triplets[text_triplets.size() - 1][i]);
			}

			m_features.push_back(cur_text.getParams());


			m_labels.push_back(k);
			file.close();
			k1++;
			proc += step1;
			if (parent->getExitFlag())
				return;
			parent->setProgressDialogValue(min(proc, 99.0));
//			if (k1 == 3)
//				break;
		}

		sort(cur_text_triplets.begin(), cur_text_triplets.end(), AText::fcmp1);

		sort(cur_author_triplets.begin(), cur_author_triplets.end());
		double s = 1;//cur_text_triplets[0].second;
		QVector<QPair<Triplet, double> > cur_author_ctr;
		for (int i = 1; i < cur_text_triplets.size(); i++)
			if (!(cur_text_triplets[i].first == cur_text_triplets[i - 1].first))
			{
				cur_author_ctr.push_back(QPair<Triplet, double>(cur_author_triplets[i - 1], s));
				s = 1;//cur_text_triplets[i].second;
			} else
				s += 1;//cur_text_triplets[i].second;
		cur_author_ctr.push_back(QPair<Triplet, double>(cur_author_triplets[cur_author_triplets.size() - 1], s));
		sort(cur_author_ctr.begin(), cur_author_ctr.end(), AText::fcmp);
		for (int i = 0; i < min(tr_count, cur_author_ctr.size()); i++)
		{
			triplets.push_back(cur_author_ctr[i].first);
		}


		k++;
//		if (k == 3)
//			break;
		if (parent->getExitFlag())
			return;
	}

	sort(triplets.begin(), triplets.end());
	int m = 1;
	if (!triplets.size())
		m = 0;
	for (int i = 1; i < triplets.size(); i++)
		if (!(triplets[i] == triplets[m - 1]))
		{
			triplets[m] = triplets[i];
			m++;
		}
	triplets.resize(m);
	for (int i = 0; i < m_labels.size(); i++)
	{
		QVector<QPair<Triplet, double> > cur_tr = text_triplets[i];
		sort(cur_tr.begin(), cur_tr.end(), AText::fcmp1);		
		m_features[i] += getParams(cur_tr);
//		m_features.push_back(getParams(cur_tr));
	}
}

void Model::trainModel()
{
	if (!m_features.size())
		return;

	m_featuresNumber = m_features[0].size();
	m_instancesNumber = m_features.size();

	struct problem prob;
	prob.l = m_instancesNumber;
	prob.bias = -1;
	prob.n = m_featuresNumber;
	prob.y = Malloc(double, m_instancesNumber);
	prob.x = Malloc(struct feature_node *, m_instancesNumber);

	for (int i = 0; i < m_instancesNumber; i++)
	{
		prob.x[i] = Malloc(struct feature_node, (this->m_featuresNumber)+1);
		prob.x[i][m_featuresNumber].index = -1;
		for (int j = 0; j < m_featuresNumber; j++)
		{
			prob.x[i][j].index = j+1;
			prob.x[i][j].value = m_features[i][j];
		}
		prob.y[i] = m_labels[i];
	}

	struct parameter param;
	param.solver_type = MCSVM_CS;
	param.ovo = 0; // one vs one
	param.C = 1;      // try to vary it
	param.eps = 1e-4;
	param.nr_weight = 0;
	param.weight_label = NULL;
	param.weight = NULL;

	m_classifier = train(&prob, &param);
	destroy_param(&param);
	free(prob.y);
	for (int i = 0; i < m_instancesNumber; i++)
		free(prob.x[i]);
	free(prob.x);
	parent->setProgressDialogValue(100);
}

void Model::sampleAndTrain()
{
    m_features.clear();
    m_labels.clear();
    sample();
    trainModel();
}

int Model::detect(const QVector<double> &params)
{
    struct feature_node* x = Malloc(struct feature_node, this->m_featuresNumber + 1);
    x[m_featuresNumber].index = -1;
    
	for (int j = 0; j < m_featuresNumber; j++)
	{
		x[j].index = j+1;
		x[j].value = params[j];
	}
	
	double *prob_estimates = new double[m_classNumber * (m_classNumber - 1) / 2];  // level of confidence
	int predicted_label = predict_values(m_classifier, x, prob_estimates);
	free(x);
	delete prob_estimates;
    return predicted_label;
}

void Model::classifyDirectory()
{
	if (!m_classifier)
	{
		parent->setProgressDialogValue(100);
		return;
	}
	QDir dir(m_dirName);
	dir.setFilter(QDir::Dirs | QDir::Hidden | QDir::NoSymLinks);
	QFileInfoList list = dir.entryInfoList();
	int k = 0;
	Spec specs;
	double step = 100 / (list.size() - 2);
	double proc = 0;
	QFile file(getUserLocationFileName());
	QTextStream out(&file);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		parent->setProgressDialogValue(100);
		return;
	}
	for (QFileInfoList::iterator j = list.begin(); j != list.end(); ++j) 
	{
		proc = k * step;
		parent->setProgressDialogValue(proc);
		QFileInfo fileInfo = *j;

		if (fileInfo.fileName() == "." || fileInfo.fileName() == "..")
			continue;
		QDir dir1(m_dirName + fileInfo.fileName());
		//authors.push_back(Author(fileInfo.fileName()));
		dir1.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
		QFileInfoList list = dir1.entryInfoList();
		double step1 = step / (list.size());
		int k1 = 0;
		for (QFileInfoList::iterator j1 = list.begin(); j1 != list.end(); ++j1) 
		{
			QFileInfo fileInfo = *j1;
			QFile file(fileInfo.filePath());
			file.open(QIODevice::ReadOnly | QIODevice::Text);
			QTextStream in(&file);
			QString s = "";
			while (!in.atEnd())
			{
				QString s1 = in.readLine();
				if (!s1.length())
					s1 = '.';
				s += s1 + " ";
			}
			AText cur_text = AText(specs, s);
			QVector<QPair<Triplet, double> > cur_triplets = cur_text.getCountTriplets();

			QVector<QPair<Triplet, double> > cur_tr = cur_triplets;
			sort(cur_tr.begin(), cur_tr.end(), AText::fcmp1);
			int res = detect(cur_text.getParams() + getParams(cur_tr));

//			int res = detect(cur_text.getParams());
			out << fileInfo.baseName() << " " << author_names[res] << "\n";
			k1++;
			proc += step1;
			if (parent->getExitFlag())
			{
				file.close();
				return;
			}
			parent->setProgressDialogValue(proc);
		}
		k++;
		if (parent->getExitFlag())
		{
			file.close();
			return;
		}
	}
	file.close();
	parent->setProgressDialogValue(100);
}

bool Model::estimateQuality()
{
	QFile file(getTrueLocationFileName());
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		QMessageBox::critical(0, "Error", "Can't open answer file");
		return false;
	}
	QTextStream in(&file);
	
	QVector<QPair<QString, QString> > TrueLocs;
	while (!in.atEnd())
	{
		QString res;
		QString s;
		in >> s >> res;
		if (s.size())
			TrueLocs.push_back(QPair<QString, QString>(s, res));
	}

	file.close();

	file.setFileName(getUserLocationFileName());
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		QMessageBox::critical(0, "Error", "Can't open prediction file");
		return false;
	}
	in.setDevice(&file);
	
	QVector<QPair<QString, QString> > UserLocs;
	while (!in.atEnd())
	{
		QString res;
		QString s;
		in >> s >> res;
		if (s.size())
			UserLocs.push_back(QPair<QString, QString>(s, res));
	}

	int TP1 = 0;
	for (int i = 0; i < TrueLocs.size(); i++)
	{
		for (int j = 0; j < UserLocs.size(); j++)
		{
			if (TrueLocs[i].first == UserLocs[j].first && TrueLocs[i].second == UserLocs[j].second)
			{
				TP1++;
				break;
			}
		}
	}
	m_precision = 1.0 * TP1 / TrueLocs.size();
    return true;
}

bool Model::predictAuthor(QString &sres)
{
	QFile file(m_textFileName);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
		return false;
	QTextStream in(&file);
	QString s = "";
	while (!in.atEnd())
	{
		QString s1 = in.readLine();
		if (!s1.length())
			s1 = '.';
		s += s1 + " ";
	}
	AText cur_text = AText(Spec(), s);
	QVector<QPair<Triplet, double> > cur_triplets = cur_text.getCountTriplets();

	QVector<QPair<Triplet, double> > cur_tr = cur_triplets;
	sort(cur_tr.begin(), cur_tr.end(), AText::fcmp1);
	int res = detect(getParams(cur_tr));

	file.close();
	sres = author_names[res];
	return true;
}