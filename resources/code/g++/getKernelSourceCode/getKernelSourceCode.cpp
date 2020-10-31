#include <unistd.h>

#include <cstdio>
#include <fstream>
#include <set>
#include <string>
#include <regex>

using namespace std;


regex re_1("^\\s*CC\\s*(.*)");
regex re_2("^.*#include\\s*<(([^/]*)/.*)>"); 
regex re_3("^.*#include\\s*\"(.*)\"");
regex re_4("(^.*/)[^/]+\\.c");
regex re_5("^.*\\s+(.*\\.h)");

bool saveSourceFileToSet(const string& logFile, set<string>& sSavedSrcFile);
void printStringSet(const set<string>& targetSet);
bool saveSetToFile(const set<string>& targetSet, const string& fileName);
bool fileExisted(const string& oneFileName);
bool findIncludes(const string& oneSrcFile, set<string>& container);
bool getRawFile_1(const smatch& match, string& rawFile);
string getPrefix(const string& srcFile);
bool getRawFile_2(const smatch& match, string& rawFile, const string& srcFile);

string strArchName = "arm";
string strMachName = "sunxi";

set<string> KeyWords = 
{
    "acpi", "asm-generic", "clocksource", "config", "crypto",
    "drm", "dt-bindings","generated", "keys", "linux", "math-emu", "media", "misc",
    "mtd", "net", "pcmcia", "rdma", "rxrpc", "scsi", "sound", "target",
    "trace", "uapi", "video", "xen"
};


int main(int argc, char* argv[])
{
    if (argc < 5) 
    {
        printf("usage: %s arch machine input output\n", argv[0]);
        return 1;
    }

    set<string> setSrcFile;
    set<string> setTotalFiles;

    strArchName = argv[1];
    strMachName = argv[2];

    saveSourceFileToSet(argv[3], setSrcFile);
    setTotalFiles = setSrcFile;

    set<string>::const_iterator it = setSrcFile.begin();
    for (; it != setSrcFile.end(); ++it)
    {
        if (findIncludes((*it), setTotalFiles) == false)
        {
            break;
        }

    }

    saveSetToFile(setTotalFiles, argv[4]);

    return 0;
}



bool findIncludes(const string& oneSrcFile, set<string>& container)
{
    ifstream in(oneSrcFile);
    if (!in)
    {
        fprintf(stderr, "Error: open %s fail.\n", oneSrcFile.c_str());
        return false;
    }


    string oneline, rawFile;
    smatch results;

    while (getline(in, oneline))
    {
        regex_search(oneline, results, re_2); // <XX>

        if (getRawFile_1(results, rawFile))
        {
            if (container.find(rawFile) == container.end()) // new file
            {
                
                container.insert(rawFile);
                findIncludes(rawFile, container);
            }
        }
        else
        {
            regex_search(oneline, results, re_3); // "XX"
            if (getRawFile_2(results, rawFile, oneline))
            {
                if (container.find(rawFile) == container.end()) // new file
                {
                    container.insert(rawFile);
                    findIncludes(rawFile, container);
                }
                
            }
        }
    }

    in.close();

    return true;
}


string getPrefix(const string& srcFile)
{
    smatch match;
    regex_search(srcFile, match, re_4);
    if (match.empty() == false)
    {
        return match.str(1); 
    }

    return "";
}


// if @rawFile exist and @match is not empty, return true
bool getRawFile_2(const smatch& match, string& rawFile, const string& srcFile)
{

    if (match.empty() == false)
    {
        string pre = getPrefix(srcFile);
        rawFile = pre + match.str(1);
        if (fileExisted(rawFile))
            return true;
    }

    return false;
}


// if @rawFile exist and @match is not empty, return true
bool getRawFile_1(const smatch& match, string& rawFile)
{
    string prefixFolder;

    if (match.empty() == false)
    {
        if (match.size() == 3)
        {
            prefixFolder = match.str(2);

            if (KeyWords.find(prefixFolder) != KeyWords.end())
            {
                rawFile = "include/" + match.str(1);
                if (fileExisted(rawFile))
                    return true;
                else
                {
                    if (prefixFolder == "linux")
                    {
                        rawFile = "include/uapi/" + match.str(1);
                        if (fileExisted(rawFile))
                            return true;
                    }
                }
            }
            else
            {
                if (prefixFolder == "asm")
                {
                    rawFile = "arch/" + strArchName + "/include/" + match.str(1);
                }
                else if (prefixFolder == "mach")
                {
                    rawFile = "arch/" + strArchName + "/mach-" + strMachName + "/include/" + match.str(1);
                }

                if (fileExisted(rawFile))
                    return true;
            }
        }

    }

    return false;
}


bool saveSourceFileToSet(const string& logFile, set<string>& sSavedSrcFile)
{
    ifstream in(logFile);

    if (!in)
    {
        fprintf(stderr, "Error: open %s fail.\n", logFile.c_str());
        return false;
    }

    string oneline, rawFile;
    smatch results;

    string::size_type pos = 0;

    while (getline(in, oneline))
    {
        regex_search(oneline, results, re_1);
        if (results.empty() == false)
        {
            rawFile = results.str(1);
            if (fileExisted(rawFile) == false)
                continue;

            pos = rawFile.find(".o");
            if (pos != string::npos) 
                sSavedSrcFile.insert(rawFile.substr(0, pos) + ".c");
            else
                sSavedSrcFile.insert(results.str(1));
        }
        else
        {
            regex_search(oneline, results, re_5);
            rawFile = results.str(1);
            if (fileExisted(rawFile) == false)
                continue;

            sSavedSrcFile.insert(rawFile);

        }
    }


    in.close();

    return true;
}


void printStringSet(const set<string>& targetSet)
{
    set<string>::const_iterator it = targetSet.begin();
    for (; it != targetSet.end(); ++it)
    {
        printf("%s\n", (*it).c_str());
    }

    printf("\n");;
}

bool saveSetToFile(const set<string>& targetSet, const string& fileName)
{
    ofstream out(fileName);
    if (!out)
    {
        printf("Error: cannot open/create %s\n", fileName.c_str());
        return false;
    }

    set<string>::const_iterator it = targetSet.cbegin();

    for (; it != targetSet.cend(); ++it)
        out << (*it) << "\r\n";

    out.close();

    return true;
}

bool fileExisted(const string& oneFileName)
{
    if (access(oneFileName.c_str(), F_OK) == 0)
        return true;
    else
        return false;
}

