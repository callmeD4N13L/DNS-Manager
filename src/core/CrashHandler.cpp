#include <QtGlobal>

#if defined(Q_OS_WIN)

#include "core/CrashHandler.hpp"

#include "core/Logging.hpp"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>

#include <windows.h>
#include <dbghelp.h>

namespace {

LONG WINAPI handler(EXCEPTION_POINTERS* info)
{
    // Minidump path + log entry are written as a best-effort; nothing in the
    // handler should allocate or lock in a way that could hang a crashed
    // process, so keep it minimal.
    const QString dir = Logging::defaultLogDir();
    QDir().mkpath(QFileInfo(dir).absolutePath());

    const QString dumpPath = dir + QLatin1String("/crash-")
        + QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd-hhmmss"))
        + QStringLiteral(".dmp");

    const HANDLE hFile = CreateFileW(reinterpret_cast<LPCWSTR>(dumpPath.utf16()),
                                     GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS,
                                     FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile != INVALID_HANDLE_VALUE) {
        MINIDUMP_EXCEPTION_INFORMATION exceptionInfo {};
        exceptionInfo.ThreadId = GetCurrentThreadId();
        exceptionInfo.ExceptionPointers = info;
        exceptionInfo.ClientPointers = FALSE;
        MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile,
                          MiniDumpNormal, info != nullptr ? &exceptionInfo : nullptr,
                          nullptr, nullptr);
        CloseHandle(hFile);
    }

    const unsigned long code = info != nullptr ? info->ExceptionRecord->ExceptionCode : 0;
    void* address = info != nullptr ? info->ExceptionRecord->ExceptionAddress : nullptr;

    const QString line = QStringLiteral("%1 FATAL Unhandled exception 0x%2 at %3 (minidump: %4)\n")
        .arg(QDateTime::currentDateTime().toString(Qt::ISODateWithMs))
        .arg(code, 8, 16, QLatin1Char('0'))
        .arg(reinterpret_cast<quintptr>(address), 0, 16)
        .arg(dumpPath);

    QFile log(Logging::defaultLogFilePath());
    if (log.open(QIODevice::WriteOnly | QIODevice::Append)) {
        log.write(line.toUtf8());
        log.flush();
    }

    return EXCEPTION_CONTINUE_SEARCH;
}

} // namespace

void CrashHandler::install()
{
    SetUnhandledExceptionFilter(handler);
}

#endif // Q_OS_WIN