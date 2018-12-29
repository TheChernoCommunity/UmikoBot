#include "InterruptHandler.h"

#include <QtWidgets/QApplication>

#if defined(Q_OS_WIN32)
#include <windows.h>
#elif defined(Q_OS_UNIX)
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#endif

#if defined(Q_OS_WIN32)
static BOOL WINAPI HandlerRoutine(DWORD sig)
{
	QApplication::quit();
	return TRUE;
}
#elif defined(Q_OS_UNIX)
static void HandlerRoutine(int sig)
{
	QApplication::quit();
}
#endif

void InterruptHandler::Init()
{
#if defined(Q_OS_WIN32)
	SetConsoleCtrlHandler(&HandlerRoutine, TRUE);
#elif defined(Q_OS_UNIX)
	struct sigaction sigIntHandler{};
	sigIntHandler.sa_handler = &HandlerRoutine;
	sigemptyset(&sigIntHandler.sa_mask);
	sigaction(SIGINT, &sigIntHandler, NULL);
#endif
}
