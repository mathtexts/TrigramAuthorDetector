#ifndef SPEC_H
#define SPEC_H

class Spec
{
	QString specs;
	QVector<QString> spec;
	friend class AText;
	friend class Sentence;
public:
	Spec()
	{
		specs = QString::fromLocal8Bit("E: E; E... в без до из к на по о от перед при через");
		specs += QString::fromLocal8Bit(" с у за над об под про для и или как словно а что но однако чтобы когда бы б");
		specs += QString::fromLocal8Bit(" лишь едва точно будто если да притом же ж тоже либо самый весь очень крайне");
		specs += QString::fromLocal8Bit(" ли ль совсем хотя чтоб");

		QStringList l = specs.split(" ");
		spec = l.toVector();
	}
};

#endif // SPEC_H