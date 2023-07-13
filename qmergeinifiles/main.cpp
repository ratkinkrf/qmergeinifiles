/****************************************************************************************
 * Copyright (c) 2007-2017 Sergey V Turchin <zerg@altlinux.org>                             *
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

#include <unordered_map>
#include <vector>
#include <iterator>
#include <string>
#include <fstream>
#include <algorithm>

extern const char *__progname;

static bool o_override = true;

typedef std::unordered_map<std::string,std::string> SettingsData;
void usage()
{
    printf("%s -- Utility to merge INI-format files\n", __progname);
    printf("(C) 2007-2019, Sergey V Turchin <zerg@altlinux.org>\n");
    printf("Usage:\n");
    printf("  %s [options] out.ini in1.ini [in2.ini in3.ini ...]\n", __progname);
    printf("Options:\n");
    printf("  -n, --no-override    Don't override contents of previous file.\n");
}

// trim from start (in place)
static inline void ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
}

// trim from end (in place)
static inline void rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}

// trim from both ends (in place)
static inline void trim(std::string &s) {
    rtrim(s);
    ltrim(s);
}

// trim from both ends (copying)
static inline std::string trim_copy(std::string s) {
    trim(s);
    return s;
}

void readIniFile(const std::string &path, SettingsData &settings_data)
{
    std::string current_group("General");
    std::fstream f;
    f.open(path,std::ios_base::in);
    if(f.is_open())
    {
        while( !f.eof() )
        {
            std::string ln;
            std::getline(f,ln);
            trim(ln);
            if( *ln.begin() == ('[') )
            {
                int idx_end = ln.find(']');
                if( idx_end > 0 )
                    current_group = trim_copy(ln.substr(1,idx_end - 1));
                else
                    continue;
            }
            else
            {
                int idx_delim = ln.find('=');
                if( idx_delim > 0 )
                {
                    std::string key = trim_copy(ln.substr(0,idx_delim));
                    key.insert(0,1,'/');
                    key.insert(0,current_group);
                    std::string value = trim_copy(ln.substr(idx_delim + 1));
                    if( o_override || !settings_data.count(key) )
                        settings_data[key] = value;
                }
                else
                    continue;
            }
        }
        f.close();
    }
}

void writeIniFile(const std::string &path, SettingsData &settings_data)
{
    std::fstream f;
    f.open(path,std::ios_base::out|std::ios_base::trunc);
    if( f.is_open() )
    {
        // collect General
        std::unordered_map<std::string, std::string> general_settings;
        auto i_settings(settings_data.begin());
        while( i_settings != settings_data.end() )
        {
            std::string key_data = i_settings->first;
            int idx_delim = key_data.find('/');
            if( idx_delim > 0 )
            {
                std::string group = key_data.substr(0,idx_delim);
                std::string key = key_data.substr(idx_delim+1);
                std::string value = i_settings->second;
                std::string newline;
                newline.append(key);
                newline.append(1,'=');
                newline.append(value);
                newline.append(1,'\n');
                if( group == "General" )
                {
                    general_settings[key] = value;
                    ++i_settings;
                    settings_data.erase(key_data);
                    continue;
                }
            }
            else
                continue;
            ++i_settings;
        }
        // write General
        if( general_settings.size() > 0 )
        {
            f.write("[General]\n",10);
            auto i_general(general_settings.begin());
            while( i_general != general_settings.end() )
            {
                std::string newline;
                newline.append(i_general->first);
                newline.append(1,'=');
                newline.append(i_general->second);
                newline.append(1,'\n');
                f.write(newline.data(),newline.size());
                ++i_general;
            }
        }
        // write other settings
        if( settings_data.size() > 0 )
        {
            std::string current_group;
            auto i_settings2(settings_data.begin());
            while( i_settings2 != settings_data.end() )
            {
                std::string key_data = i_settings2->first;
                int idx_delim = key_data.find('/');
                if( idx_delim > 0 )
                {
                    std::string newline;
                    std::string group = key_data.substr(0,idx_delim);
                    std::string key = key_data.substr(idx_delim+1);
                    std::string value = i_settings2->second;
                    if( current_group != group )
                    {
                        current_group = group;
                        std::string groupline;
                        groupline.append("\n[");
                        groupline.append(current_group);
                        groupline.append("]\n");
                        f.write(groupline.data(),groupline.size());
                    }
                    newline.append(key);
                    newline.append(1,'=');
                    newline.append(value);
                    newline.append(1,'\n');
                    f.write(newline.data(),newline.size());
                }
                ++i_settings2;
            }
        }
        f.close();
    }
    else
        printf("Unable to open file  %s for writing.", path.data());
}

int main(int argc, char** argv)
{
    std::vector<std::string> files;

    std::vector<std::string> arguments(argv + 1, argv + argc);
    auto it(arguments.begin());
    while( it != arguments.end() )
    {
        std::string argument = *it;
        if(argument == "-n" || argument == "--no-override" )
        {
            o_override = false;
        }
        else
        {
            files.push_back(argument);
        }
        ++it;
    }
    if( files.size() > 1 )
    {
        SettingsData settings;
        std::string out_filename(files.at(0));
        readIniFile(out_filename, settings);
        auto fit(files.begin());
        while( fit != files.end() )
        {
            std::string in_filename = *fit;
            readIniFile(in_filename, settings);
            ++fit;
        }
        writeIniFile(out_filename, settings);
    }
    else
    {
        usage();
        printf("Too few arguments.");
    }
}
