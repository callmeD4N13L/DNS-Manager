#pragma once

#if defined(Q_OS_WIN)

// Installs an unhandled-exception filter that writes a minidump (dbghelp)
// into the log directory and a log line before letting the system handler run.
namespace CrashHandler {
void install();
} // namespace CrashHandler

#endif // Q_OS_WIN