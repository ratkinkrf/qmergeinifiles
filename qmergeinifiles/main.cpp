
#include <QCoreApplication>
#include <QStringList>
#include <QFile>
#include <QSettings>

static bool o_override = true;

void mergeSettingsKeys(QSettings &out, QSettings &in)
{
    QStringList in_keys = in.childKeys();
    QStringListIterator it(in_keys);
    while( it.hasNext() )
    {
	QString key = it.next();
	out.setValue(key, in.value(key));
    }
}

void mergeSettingsGroups(QSettings &out, QSettings &in)
{
    QStringList in_groups = in.childGroups();
    QStringListIterator it(in_groups);
    while( it.hasNext() )
    {
	QString group = it.next();
	out.beginGroup(group); in.beginGroup(group);
	mergeSettingsKeys(out, in);
	mergeSettingsGroups(out, in);
	out.endGroup(); in.endGroup();
    }
}

void mergeSettings(QSettings &out, QSettings &in)
{
    out.beginGroup(""); in.beginGroup("");
    mergeSettingsKeys(out, in);
    mergeSettingsGroups(out, in);
    out.endGroup(); in.endGroup();
    out.sync();
}

int main(int argc, char** argv)
{
    QStringList files;

    QCoreApplication app = QCoreApplication(argc, argv);
    QStringList arguments = app.arguments();
    QStringListIterator it(arguments);
    while( it.hasNext() )
    {
	QString argument = it.next();
	if(argument == "-n" && argument == "--no-override" )
	{
	    o_override = false;
	}
	else
	{
	    files.append(argument);
	}
    }
    if( files.size() > 2 )
    {
	files.takeAt(0);
	QString out_filename(files.takeAt(0));
	QSettings out(out_filename, QSettings::IniFormat);
	QStringListIterator fit(files);
	while( fit.hasNext() )
	{
	    QString in_filename = fit.next();
	    QFile in_file(in_filename);
	    if( in_file.exists() )
	    {
		QSettings in(in_filename, QSettings::IniFormat);
		mergeSettings(out, in);
	    }
	    else
	    {
		qDebug("File %s not exist. Skipping.", qPrintable(in_filename));
	    }
	}
    }
    else
	qFatal("Too few arguments.");
}
