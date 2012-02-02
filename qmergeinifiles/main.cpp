/****************************************************************************************
 * Copyright (c) 2007-2012 Sergey V Turchin <zerg@altlinux.org>                             *
 *                                                                                      *
 * This program is free software; you can redistribute it and/or modify it under        *
 * the terms of the GNU General Public License as published by the Free Software        *
 * Foundation; either version 2 of the License, or (at your option) any later           *
 * version.                                                                             *
 *                                                                                      *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY      *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A      *
 * PARTICULAR PURPOSE. See the GNU General Public License for more details.             *
 *                                                                                      *
 * You should have received a copy of the GNU General Public License along with         *
 * this program.  If not, see <http://www.gnu.org/licenses/>.                           *
 ****************************************************************************************/

#include <QCoreApplication>
#include <QStringList>
#include <QFile>
#include <QHash>

extern const char *__progname;

static bool o_override = true;

typedef QHash<QByteArray,QByteArray> SettingsData;

void usage()
{
    qDebug("");
    qDebug("%s -- Utility to merge INI-format files", __progname);
    qDebug("(C) 2007-2012, Sergey V Turchin <zerg@altlinux.org>");
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

void writeIniFile(const QString &path, SettingsData &settings_data)
{
    QFile f(path);
    if( f.open(QIODevice::WriteOnly|QIODevice::Truncate) )
    {

	// collect General
	QHash<QByteArray,QByteArray> general_settings;
	QHashIterator<QByteArray,QByteArray> i_settings(settings_data);
	while( i_settings.hasNext() )
	{
	    i_settings.next();
	    QByteArray key_data = i_settings.key();
	    int idx_delim = key_data.indexOf('/');
	    if( idx_delim > 0 )
	    {
		QByteArray group = key_data.mid(0,idx_delim);
		QByteArray key = key_data.mid(idx_delim+1);
		QByteArray value = i_settings.value();
		QByteArray newline;
		newline.append(key);
		newline.append('=');
		newline.append(value);
		newline.append('\n');
		if( group == "General" )
		{
		    general_settings[key] = value;
		    settings_data.remove(key_data);
		}
	    }
	    else
		continue;
	}
	// write General
	if( general_settings.size() > 0 )
	{
	    f.write("[General]\n");
	    QHashIterator<QByteArray,QByteArray> i_general(general_settings);
	    while( i_general.hasNext() )
	    {
		i_general.next();
		QByteArray newline;
		newline.append(i_general.key());
		newline.append('=');
		newline.append(i_general.value());
		newline.append('\n');
		f.write(newline);
	    }
	}
	// write other settings
	if( settings_data.size() > 0 )
	{
	    QByteArray current_group;
	    QHashIterator<QByteArray,QByteArray> i_settings(settings_data);
	    while( i_settings.hasNext() )
	    {
		i_settings.next();
		QByteArray key_data = i_settings.key();
		int idx_delim = key_data.indexOf('/');
		if( idx_delim > 0 )
		{
			QByteArray newline;
			QByteArray group = key_data.mid(0,idx_delim);
			QByteArray key = key_data.mid(idx_delim+1);
			QByteArray value = i_settings.value();
			if( current_group != group )
			{
			    current_group = group;
			    QByteArray groupline;
			    groupline.append("\n[");
			    groupline.append(current_group);
			    groupline.append("]\n");
			    f.write(groupline);
			}
			newline.append(key);
			newline.append('=');
			newline.append(value);
			newline.append('\n');
			f.write(newline);
		}
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
