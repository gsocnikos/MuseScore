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

#ifndef CRASH_HANDLER_H
#define CRASH_HANDLER_H

#pragma once
#include <QtCore/QString>

#include <QtCore/QDir>
#include <QtCore/QProcess>
#include <QtCore/QCoreApplication>
#include <QMutex>


#include "client/windows/handler/exception_handler.h"
#include "common/windows/http_upload.h"


using std::wstring;
using std::pair;


namespace Breakpad {

    int AnnotateCrashReport(const QString& aKey, const QString& aData);
    int PrintMyCrashReport();



    class CrashHandlerPrivate;
    class CrashHandler
    {
    public:

        static CrashHandler* instance();

        void Init(const QString&  reportPath);
        void setReportCrashesToSystem(bool report);
        bool writeMinidump();




    private:
        CrashHandler();
        ~CrashHandler();
        Q_DISABLE_COPY(CrashHandler)
        CrashHandlerPrivate* d;
    };
}

#endif // CRASH_HANDLER_H
