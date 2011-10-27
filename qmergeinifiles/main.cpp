
#include <QCoreApplication>
#include <QStringList>
#include <QFile>
#include <QHash>

extern const char *__progname;

static bool o_override = true;

typedef QHash<QByteArray,QByteArray> SettingsData;

void usage()
{
    qDebug("Usage:");
    qDebug("  %s [options] out.ini in1.ini [in2.ini in3.ini ...]", __progname);
    qDebug("Options:");
    qDebug("  -n, --no-override    Don't override contents of previous file.\n");
}

void readIniFile(const QString &path, SettingsData &settings_data)
{
    QByteArray current_group("General");
    QFile f(path);
    if( f.open(QIODevice::ReadOnly) )
    {
	while( !f.atEnd() )
	{
	    QByteArray ln = f.readLine().trimmed();
	    if( ln.startsWith('[') )
	    {
		int idx_end = ln.indexOf(']');
		if( idx_end > 0 )
		    current_group = ln.mid(1,idx_end - 1).trimmed();
		else
		    continue;
	    }
	    else
	    {
		int idx_delim = ln.indexOf('=');
		if( idx_delim > 0 )
		{
		    QByteArray key = ln.mid(0,idx_delim).trimmed();
		    key.prepend('/');
		    key.prepend(current_group);
		    QByteArray value = ln.mid(idx_delim + 1).trimmed();
		    if( o_override || !settings_data.contains(key) )
			settings_data[key] = value;
		}
		else
		    continue;
	    }
	}
	f.close();
    }
}

void writeIniFile(const QString &path, const SettingsData &settings_data)
{
    QFile f(path);
    if( f.open(QIODevice::WriteOnly|QIODevice::Truncate) )
    {
	f.write("[General]\n");
	for(int state = 0; state <= 1; state++)
	{
		QByteArray current_group;
		QHashIterator<QByteArray,QByteArray> i_general(settings_data);
		while( i_general.hasNext() )
		{
		    i_general.next();
		    QByteArray key_data = i_general.key();
		    int idx_delim = key_data.indexOf('/');
		    if( idx_delim > 0 )
		    {
			QByteArray group = key_data.mid(0,idx_delim);
			QByteArray key = key_data.mid(idx_delim+1);
			QByteArray value = i_general.value();
			QByteArray newline;
			newline.append(key);
			newline.append('=');
			newline.append(value);
			newline.append('\n');
			if( state == 0 )
			{
			    if( group == "General" )
				f.write(newline);
			}
			else
			{
			    if( group == "General" )
				continue;
			    if( current_group != group )
			    {
				current_group = group;
				QByteArray groupline;
				groupline.append("\n[");
				groupline.append(current_group);
				groupline.append("]\n");
				f.write(groupline);
			    }
			    f.write(newline);
			}
		    }
		    else
			continue;
		}
	}
	f.close();
    }
    else
	qDebug("Unable to open file \"%s\" for writing.", qPrintable(path));
}

int main(int argc, char** argv)
{
    QStringList files;

    QCoreApplication app(argc, argv);
    QStringList arguments = app.arguments();
    arguments.takeAt(0);
    QStringListIterator it(arguments);
    while( it.hasNext() )
    {
	QString argument = it.next();
	if(argument == "-n" || argument == "--no-override" )
	{
	    o_override = false;
	}
	else
	{
	    files.append(argument);
	}
    }
    if( files.size() > 1 )
    {
	SettingsData settings;
	QString out_filename(files.takeAt(0));
	readIniFile(out_filename, settings);
	QStringListIterator fit(files);
	while( fit.hasNext() )
	{
	    QString in_filename = fit.next();
	    QFile in_file(in_filename);
	    readIniFile(in_filename, settings);
	}
	writeIniFile(out_filename, settings);
    }
    else
    {
	usage();
	qFatal("Too few arguments.");
    }
}
