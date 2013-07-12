#ifndef AUTHOR_H
#define AUTHOR_H

#include <QDir>
#include <QTextStream>
#include <QVector>
#include "spec.h"

using namespace std;

class Triplet
{
	int c[3];
public:
	Triplet(int c1 = 0, int c2 = 0, int c3 = 0)
	{
		c[0] = c1;
		c[1] = c2;
		c[2] = c3;
	}
	int get(int i)
	{
		return c[i];
	}
	bool operator ==(const Triplet &a)
	{
		return (c[0] == a.c[0] && c[1] == a.c[1] && c[2] == a.c[2]);
	}
	bool operator <(const Triplet &a) const
	{
		return (c[0] < a.c[0] || c[0] == a.c[0] && c[1] < a.c[1] || c[0] == a.c[0] && c[1] == a.c[1] && c[2] < a.c[2]);
	}
};

class Sentence
{
	QString s;
	QVector<QString> lexems;
	int word_count;
	bool isSep(QCharRef c)
	{
		return (c == ',' || c == '.' || c == '!' || c == '?' || c == ';' || c == ':' || c == '<' || c == '>' || c == '"'
				|| c == '«' || c == '»');
	}
public: static int getHash(QString s)
	{
		int res = 0;
		int p = 1;
		s = s.toLower();
		int j = 0;
		int fl = 0;
		for (int i = 0; i < s.length(); i++)
		{
			if (s[i].isLetter())
			{
				fl = 1;
				int c = -1;
				int num = s[i].unicode();
				if (num >= 1072 && num <= 1103)
					c = num - 1072;
				else if (num == 1107)
					c = 32;
				if (c == -1)
				{
					if (num >= 97 && num <= 122)
						return -2;
					fl = 0;
					break;
				}
				res += c * p;
				p *= 33;
				j++;
				if (j == 6)
					break;
			}
		}
		if (!fl)
			res = -1;

		return res;
	}
public:
	Sentence(QString _s = "") : s(_s)
	{
		QString cur_lex = "";
		int num = 0;
		word_count = 0;
		for (int i = 0; i < s.length(); i++)
		{
			if (!num)
			{
				if (!s[i].isSpace())
				{
					if (isSep(s[i]))
					{
						num = 2;
						cur_lex = s[i];
					} else
					{
						num = 1;
						cur_lex = s[i];
					}
				}
			} else if (num == 1)
			{
				if (!isSep(s[i]) && !s[i].isSpace())
				{
					cur_lex += s[i];
				} else
				{
					lexems.push_back(cur_lex);
					word_count++;
					cur_lex = "";
					if (s[i].isSpace())
						num = 0;
					else
					{
						num = 2;
						cur_lex = s[i];
					}
				}
			} else if (num == 2)
			{
				if (isSep(s[i]))
				{
					cur_lex += s[i];
				} else
				{
					lexems.push_back(cur_lex);
					cur_lex = "";
					if (s[i].isSpace())
						num = 0;
					else
					{
						num = 1;
						cur_lex = s[i];
					}
				}
			}
		}
		if (num)
			lexems.push_back(cur_lex);
	}
	int getWordCount()
	{
		return word_count;
	}
	void get_triplets(QVector<Triplet> &triplets)
	{
		if (lexems.size() < 3)
			return;
		int c1 = getHash(lexems[0]);
		int c2 = getHash(lexems[1]);
		for (int i = 2; i < lexems.size(); i++)
		{
			int c3 = getHash(lexems[i]);
			if (c1 >= 0 && c2 >= 0 && c3 >= 0)
				triplets.push_back(Triplet(c1, c2, c3));
			c1 = c2;
			c2 = c3;
		}
	}
	void add_words_num(QVector<int> &words_num)
	{
		for (int i = 0; i < lexems.size(); i++)
		{
			if (lexems[i].length() >= 5)
			{
				words_num.push_back(getHash(lexems[i]));
			}
		}
	}
	void count_spec(const Spec &specs, QVector<int> &c_spec)
	{
		QVector<int> spec_hash;
		for (int i = 0; i < specs.spec.size(); i++)
			spec_hash.push_back(getHash(specs.spec[i]));

		for (int i = 0; i < lexems.size(); i++)
		{
			int fl = 0;
			if (lexems[i].length() <= 6)
			{
				int h = getHash(lexems[i]);
				for (int j = 0; j < specs.spec.size(); j++)
				{
					if (specs.spec[j][0] != 'E' && specs.spec[j][0] != 'P')
						if (h == spec_hash[j])
							{
								c_spec[j]++;
								fl = 1;
								break;
							}
				}
			}
			if (!fl)
			{
				for (int j = 0; j < specs.spec.size(); j++)
				{
					if (specs.spec[j][0] == 'E')
					{
						int ls = lexems[i].length();
						int lp = specs.spec[j].length();
						if (ls >= lp - 1)
						{
							fl = 1;
							for (int k = 1; k < lp; k++)
								if (lexems[i][ls - lp + k] != specs.spec[j][k])
								{
									fl = 0;
									break;
								}
							if (fl)
							{
								c_spec[j]++;
								break;
							}
						}
					} else if (specs.spec[j][0] == 'P')
					{
						int ls = lexems[i].length();
						int lp = specs.spec[j].length();
						if (ls >= lp - 1)
						{
							fl = 1;
							for (int k = 1; k < lp; k++)
								if (lexems[i][k - 1] != specs.spec[j][k] && lexems[i][k - 1] != specs.spec[j][k].toLower())
								{
									fl = 0;
									break;
								}
								if (fl)
								{
									c_spec[j]++;
									break;
								}
						}
					} else
						break;
				}
			}
		}

	}
};

class ATextParams
{
protected:
	QVector<double> params;
public:
	ATextParams() : params(0){}
	ATextParams(QVector<double> _params) : params(_params) {}
	double getParams(int i)
	{
		return params[i];
	}
	QVector<double> getParams()
	{
		return QVector<double>(0);
		return params;
	}
	int getParamsSize()
	{
		return params.size();
	}
};

class AText : public ATextParams
{
	QVector<Sentence> sentences;
	QVector<int> words_num;
	QVector<Triplet> triplets;
	QVector<QPair<Triplet, double> > ctr;

public:

static int fcmp(const QPair<Triplet, double> &a, const QPair<Triplet, double> &b)
{
	return a.second > b.second;
}

static int fcmp1(const QPair<Triplet, double> &a, const QPair<Triplet, double> &b)
{
	return a.first < b.first;
}

static int fcmp2(const QPair<Triplet, int> &a, const QPair<Triplet, int> &b)
{
	return a.second > b.second;
}

public:
	AText(ATextParams params) : ATextParams(params){}
	AText(Spec specs = Spec(), QString s = "") : ATextParams(), sentences(0), words_num(0), triplets(0), ctr(0)
	{
		QString cur_s = "";
		for (int i = 0; i < s.length(); i++)
		{
			if (s[i].isUpper())
			{
				int fl = 0;
				for (int j = i - 1; j >= 0; j--)
				{
					if (s[j] == '.' || s[j] == '!' || s[j] == '?')
					{
						fl = 1;
						break;
					}
					if (!s[j].isSpace())
						break;
				}
				if (fl)
				{
					sentences.push_back(Sentence(cur_s));
					cur_s = "";
				}
			}
			cur_s += s[i];
		}
		sentences.push_back(Sentence(cur_s));
		int sum_word_count = 0;
		QVector<int> sum_spec;
		sum_spec.resize(specs.spec.size());
		sum_spec.fill(0);
		for (int i = 0; i < sentences.size(); i++)
		{
			sum_word_count += sentences[i].getWordCount();
			sentences[i].get_triplets(triplets);
			sentences[i].add_words_num(words_num);
			sentences[i].count_spec(specs, sum_spec);
		}
		int all_spec = 0;
		for (int i = 0; i < specs.spec.size(); i++)
			all_spec += sum_spec[i];

		sort(triplets.begin(), triplets.end());
		
		int kt = 1;
		for (int i = 1; i < triplets.size(); i++)
		{
			if (!(triplets[i] == triplets[i - 1]))
			{
				ctr.push_back(QPair<Triplet, double>(triplets[i - 1], kt));
				kt = 1;
			} else
				kt++;
		}
		if (triplets.size())
			ctr.push_back(QPair<Triplet, double>(triplets[triplets.size() - 1], kt));
		sort(ctr.begin(), ctr.end(), fcmp);
		for (int i = 0; i < ctr.size(); i++)
			ctr[i].second = ctr[i].second * 100000 / triplets.size();//words_num.size();

		sort(words_num.begin(), words_num.end());
		int c_uniq = 1;
		int c_uniq1 = 0;
		QVector<int> c_spec;
		int cm2 = 0, cm1 = 0;
		for (int i = 0; i < words_num.size() - 1; i++)
			if (words_num[i] == -2)
				cm2++;
			else
				break;
		for (int i = 0; i < words_num.size() - 1; i++)
			if (words_num[i] == -1)
				cm1++;
			else
				break;

		for (int i = 1; i < words_num.size() - 1; i++)
		{
			if (words_num[i] != words_num[i - 1])
				c_uniq++;
			if (words_num[i] != words_num[i - 1] && words_num[i] != words_num[i + 1])
				c_uniq1++;
		}

		params.resize(specs.spec.size());
		for (int i = 0; i < params.size(); i++)
			params[i] = 1.0 * sum_spec[i] * 100 / words_num.size();
//		params[specs.spec.size()] = 1.0 * cm2 / words_num.size();
//		params[specs.spec.size() + 1] = 1.0 * cm1 / words_num.size();
//		params[specs.spec.size() + 2] = 1.0 * c_uniq / words_num.size();
//		params[specs.spec.size() + 3] = 1.0 * c_uniq1 / words_num.size();

	}
	QVector<Triplet> getBestTriplets(int n)
	{
		if (n > ctr.size())
			n = ctr.size();
		QVector<Triplet> res;
		for (int i = 0; i < n; i++)
			res.push_back(ctr[i].first);
		return res;
	}
	QVector<QPair<Triplet, double> > getCountTriplets()
	{
		return ctr;
	}
};

class Author
{
	QString name;
	QVector<AText> texts;
public:
	Author(QString _name = "") : name(_name), texts(0){}
	void addText(Spec specs, QString s)
	{
		texts.push_back(AText(specs, s));
	}
	void addTextParams(QVector<double> params)
	{
		texts.push_back(AText(ATextParams(params)));
	}
	QString getName()
	{
		return name;
	}
	QVector<AText> getTexts()
	{
		return texts;
	}
	int getParamsCount()
	{
		return texts[0].getParamsSize();
	}
};


#endif // AUHTOR_H