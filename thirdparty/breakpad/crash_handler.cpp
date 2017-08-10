//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2017 Nikolaos Hatzopoulos (nickhatz@csu.fullerton.edu)
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#include "crash_handler.h"

namespace Breakpad {


    /************************************************************************/
    /* CrashHandlerPrivate                                                  */
    /************************************************************************/
    class CrashHandlerPrivate
    {
    public:
        CrashHandlerPrivate()
        {
            pHandler = NULL;
        }

        ~CrashHandlerPrivate()
        {
            delete pHandler;
        }

        void InitCrashHandler(wstring dumpPath);

        static google_breakpad::ExceptionHandler* pHandler;

        static bool bReportCrashesToSystem;

    };

    google_breakpad::ExceptionHandler* CrashHandlerPrivate::pHandler = NULL;
    bool CrashHandlerPrivate::bReportCrashesToSystem = false;

    //QHash<QString, QString> crashTable;
    std::map<string,string> crashTable;
    QMutex mymutex;

    int AnnotateCrashReport(string aKey, string aData){
        mymutex.lock();
        //crashTable.insert(aKey,aData);
        crashTable[aKey] = aData;
        mymutex.unlock();
        return 0;
    }

    int PrintMyCrashReport(){
        for (std::map<string,string>::iterator it=crashTable.begin(); it!=crashTable.end(); ++it){
            std::cout << it->first << " " << it->second << std::endl;
        }

        return 0;
    }

    void writeMyCrashReport(wstring mypath){
        string my_strpath = std::string(mypath.begin(),mypath.end());
        std::ofstream myfile;
        myfile.open(my_strpath);
        for (std::map<string,string>::iterator it=crashTable.begin(); it!=crashTable.end(); ++it){
            myfile << it->first << "," << it->second << std::endl;
        }
        myfile.close();

    }

    /************************************************************************/
    /* DumpCallback                                                         */
    /************************************************************************/
#if defined(Q_OS_WIN32)
    bool DumpCallback(const wchar_t* _dump_dir,const wchar_t* _minidump_id,void* context,EXCEPTION_POINTERS* exinfo,MDRawAssertionInfo* assertion,bool success)
#endif
    {
        Q_UNUSED(context);
#if defined(Q_OS_WIN32)
        Q_UNUSED(_dump_dir);
        Q_UNUSED(_minidump_id);
        Q_UNUSED(assertion);
        Q_UNUSED(exinfo);
#endif

        qDebug("BreakpadQt crash");

        //PrintMyCrashReport();
#if defined(Q_OS_WIN32)
        wstring minidump_path;
        wstring metadata_path;
        wstring program_path;

        // Minidump Path
        minidump_path =  wstring(_dump_dir) + L"/" + wstring(_minidump_id) + L".dmp";
        metadata_path =  wstring(_dump_dir) + L"/" + wstring(_minidump_id) + L".txt";

        qDebug("minidump path: %s\n", std::string(minidump_path.begin(),minidump_path.end()).c_str());

        writeMyCrashReport(metadata_path);

        // How are we going to define the path of the crashReporter???
        program_path = L"C:/Users/nickhatz/MuseScore/build.release/thirdparty/breakpad/musescore_crashreporter.exe";
        launcher(program_path, minidump_path, metadata_path);

#endif

        /*
        NO STACK USE, NO HEAP USE THERE !!!
        Creating QString's, using qDebug, etc. - everything is crash-unfriendly.
        */
        return CrashHandlerPrivate::bReportCrashesToSystem ? success : true;
    }

    void CrashHandlerPrivate::InitCrashHandler(wstring dumpPath)
    {
        if ( pHandler != NULL )
            return;
 #if defined(Q_OS_WIN32)
        //std::wstring pathAsStr = string2wstring(dumpPath);
        pHandler = new google_breakpad::ExceptionHandler(
            dumpPath,
            /*FilterCallback*/ 0,
            DumpCallback,
            /*context*/
            0,
            true
            );
 #endif



    }

    /************************************************************************/
    /* CrashHandler                                                         */
    /************************************************************************/
    CrashHandler* CrashHandler::instance()
    {
        static CrashHandler globalHandler;
        return &globalHandler;
    }

    CrashHandler::CrashHandler()
    {
        d = new CrashHandlerPrivate();
    }

    CrashHandler::~CrashHandler()
    {
        delete d;
    }

    void CrashHandler::setReportCrashesToSystem(bool report)
    {
        d->bReportCrashesToSystem = report;
    }

    bool CrashHandler::writeMinidump()
    {
        bool res = d->pHandler->WriteMinidump();
        if (res) {
            qDebug("BreakpadQt: writeMinidump() success.");
        } else {
            qWarning("BreakpadQt: writeMinidump() failed.");
        }

        return res;
    }

    void CrashHandler::Init( wstring reportPath )
    {
        d->InitCrashHandler(reportPath);

    }

    bool launcher(wstring program, wstring minidump_path, wstring metadata_path){

        STARTUPINFO si;
        PROCESS_INFORMATION pi;
        wstring mycmd;

        //example command: c:\...\crashReporter.exe minidump_path
        mycmd = program + L" " + minidump_path + L" " + metadata_path;

        ZeroMemory( &si, sizeof(si) );
        si.cb = sizeof(si);
        ZeroMemory( &pi, sizeof(pi) );


        std::wcout << "CrashReporter: "  << program
                   << "Dmppath: "        << minidump_path;

        // Open a Windows Process equivelant to fork() for Linux
        if( !CreateProcess( NULL,   // No module name (use command line)
                (WCHAR *)mycmd.c_str(),          // Command line
                NULL,           // Process handle not inheritable
                NULL,           // Thread handle not inheritable
                FALSE,          // Set handle inheritance to FALSE
                0,              // No creation flags
                NULL,           // Use parent's environment block
                NULL,           // Use parent's starting directory
                &si,            // Pointer to STARTUPINFO structure
                &pi )           // Pointer to PROCESS_INFORMATION structure
            )
            {
                printf( "CreateProcess failed (%lu).\n", GetLastError() );
                return true;
            }

        // Wait until child process exits.
        //WaitForSingleObject( pi.hProcess, INFINITE );

        // Close process and thread handles.
        CloseHandle( pi.hProcess );
        CloseHandle( pi.hThread );


        Q_UNUSED(program);
        return false;

    }



}
