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

        void InitCrashHandler(const QString& dumpPath);
#if defined(Q_OS_WIN32)
        static google_breakpad::ExceptionHandler* pHandler;
#endif
        static bool bReportCrashesToSystem;

    };

    google_breakpad::ExceptionHandler* CrashHandlerPrivate::pHandler = NULL;
    bool CrashHandlerPrivate::bReportCrashesToSystem = false;

    QHash<QString, QString> crashTable;
    QMutex mymutex;

    int AnnotateCrashReport(const QString& aKey, const QString& aData){
        mymutex.lock();
        crashTable.insert(aKey,aData);
        mymutex.unlock();
        return 0;
    }

    int PrintMyCrashReport(){
        QHashIterator<QString, QString> i(crashTable);
        while (i.hasNext()) {
            i.next();
            qDebug("%s : %s", qUtf8Printable(i.key()),qUtf8Printable(i.value()) );
        }
        return 0;
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

        minidump_path =  wstring(_dump_dir) + L"/" + wstring(_minidump_id) + L".dmp";

        qDebug("minidump path: %s\n", std::string(minidump_path.begin(),minidump_path.end()).c_str());

        std::map<wstring, wstring> parameters;
        std::map<wstring, wstring> files;


        // Add any attributes to the parameters map.
        // Attributes such as uname.sysname, uname.version, cpu.count are
        // extracted from minidump files automatically.
        //parameters["product_name"] = "foo";
        parameters.insert(pair<wstring, wstring>(L"product_name", L"foo"));
        //parameters["version"] = "0.1.0";
        parameters.insert(pair<wstring, wstring>(L"version", L"0.1.0"));
        // parameters["uptime"] uptime_sec;

        // Pass Crash table data
        QHashIterator<QString, QString> i(crashTable);
        while (i.hasNext()) {
            i.next();
            //qDebug("%s : %s", qUtf8Printable(i.key()),qUtf8Printable(i.value()) );
            parameters.insert(pair<wstring, wstring>(i.key().toStdWString(),i.value().toStdWString()));
        }

        //files["upload_file_minidump"] = minidump_path.toStdString();
        files.insert(pair<wstring, wstring>(L"upload_file_minidump", minidump_path));

        wstring url = L"https://musescore.sp.backtrace.io:6098/post?format=minidump&token=00268871877ba102d69a23a8e713fff9700acf65999b1f043ec09c5c253b9c03";

        wstring response;

        int mytimeout, my_error;


        google_breakpad::HTTPUpload *test_upload;
        test_upload->SendRequest(url,
                                   parameters,
                                   files,
                                   &mytimeout,
                                   &response,
                                   &my_error);


        std::string s( response.begin(), response.end() );
        qDebug("%s\n",s.c_str());
#endif


        /*
        NO STACK USE, NO HEAP USE THERE !!!
        Creating QString's, using qDebug, etc. - everything is crash-unfriendly.
        */
        return CrashHandlerPrivate::bReportCrashesToSystem ? success : true;
    }

    void CrashHandlerPrivate::InitCrashHandler(const QString& dumpPath)
    {
        if ( pHandler != NULL )
            return;
 #if defined(Q_OS_WIN32)
        std::wstring pathAsStr = (const wchar_t*)dumpPath.utf16();
        pHandler = new google_breakpad::ExceptionHandler(
            pathAsStr,
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

    void CrashHandler::Init( const QString& reportPath )
    {
        d->InitCrashHandler(reportPath);
    }
}
