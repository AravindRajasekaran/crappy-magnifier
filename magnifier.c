/*
   Copyright 2005 Odbayar Nyamtseren

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#include <windows.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdarg.h>
#include "common.h"
#include "mainwnd.h"
#include "viewport.h"

HINSTANCE hinst;

t_cfg cfg = {
	{1, 5, 10, 15, 20, 25, 30, 35, 40, 45},
	{0.25f, 0.5f, 0.75f, 1.0f, 1.5f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f},
	6, 5,
	10, 10,
	30, 30, 64, 64
};

void SaveConfig()
{
	FILE *fout;

	fout = fopen("magnifier.cfg", "wb");
	if (!fout)
		return;
	fwrite(&cfg, sizeof(cfg), 1, fout);
	fclose(fout);
}

void LoadConfig()
{
	FILE *fin;
	
	fin = fopen("magnifier.cfg", "rb");
	if (!fin)
		return;
	fread(&cfg, sizeof(cfg), 1, fin);
	fclose(fin);
}

int x_Die(char *name, int line, int type, ...)
{
	char errmsg1[256];
	char errmsg2[256];
	va_list args;
	char *fmt;
	
	va_start(args, type);
	switch (type)
	{
		case DT_WINAPI:
			{
				char *lpMsgBuf;
				FormatMessage(
    					FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
						NULL,
						GetLastError(),
						MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
						(LPTSTR) &lpMsgBuf,
						0,
						NULL
					);
				sprintf(errmsg1, "%s: %d: %s", name, line, lpMsgBuf);
				LocalFree(lpMsgBuf);
			}
			break;
		case DT_TEXT:
			fmt = va_arg(args, char *);
			vsprintf(errmsg2, fmt, args);
			sprintf(errmsg1, "file: %s, line: %d\n%s", name, line, errmsg2);
	}
	MessageBox(NULL, errmsg1, "Spoon and Fork", MB_OK|MB_ICONINFORMATION);
	va_end(args);

	ExitProcess(1);
}

void CreateConsole()
{
	AllocConsole();
	*stdout = *_fdopen(_open_osfhandle((long)GetStdHandle(STD_OUTPUT_HANDLE), _O_TEXT), "w");
	setvbuf(stdout, NULL, _IONBF, 0);
}

DWORD WINAPI Magnifier(LPVOID param)
{
	int now, next = 0;

	while (!MainWND.buf)
		Sleep(10);
	while (1)
	{
		now = GetTickCount();
		if (now >= next)
		{
			next = now + 1000 / cfg.fps[cfg.fps_i];
			InvalidateRect(MainWND.hwnd, NULL, FALSE);
		}
		Sleep(1);
	}
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    MSG	msg;
	DWORD id;

#ifndef NDEBUG
	CreateConsole();
#endif

	hinst = hInstance;

	LoadConfig();

	MainWND_Create();

	CreateThread(&(SECURITY_ATTRIBUTES){sizeof(SECURITY_ATTRIBUTES), FALSE, 0}, 0, Magnifier, NULL, 0, &id);

	while (GetMessage(&msg, NULL, 0, 0)) 
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	SaveConfig();

	return msg.wParam; 
}
