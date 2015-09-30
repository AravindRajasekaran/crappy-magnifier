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
#include <gl\gl.h>
#include "common.h"
#include "viewport.h"
#include "mainwnd.h"

t_ViewPort ViewPort = {};

LRESULT APIENTRY ViewPort_Proc(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam)
{
	switch (umsg)
	{
		case WM_CLOSE:
		case WM_DESTROY:
			break;
		case WM_CREATE:
			{
				RECT rc;
				
				GetClientRect(hwnd, &rc);
				ViewPort.parent = GetParent(hwnd);
				ViewPort.cw = rc.right;
				ViewPort.ch = rc.bottom;
				SendMessage(
						ViewPort.parent, UM_RESIZE, RS_CLIENT,
						ViewPort.cw * cfg.zoom[cfg.zoom_i] + ((long)(ViewPort.ch * cfg.zoom[cfg.zoom_i]) << 16)
					);
			}
			break;
		case WM_SIZE:
			{
				RECT *rnc = &ViewPort.rnc;
				HRGN rgn, rgno, rgni;
				int w, h;
				
				ViewPort.cw = LOWORD(lparam);
				ViewPort.ch = HIWORD(lparam);

				w = LOWORD(lparam) + rnc->left + rnc->right;
				h = HIWORD(lparam) + rnc->top + rnc->bottom;

				rgn = CreateRectRgn(0, 0, 1, 1);
				rgno = CreateRectRgn(0, 0, w + 1, h + 1);
				rgni = CreateRectRgn(rnc->left, rnc->top, w - rnc->right, h - rnc->bottom);
				CombineRgn(rgn, rgno, rgni, RGN_XOR);
				SetWindowRgn(hwnd, rgn, TRUE);
				DeleteObject(rgno);
				DeleteObject(rgni);

				SendMessage(
						ViewPort.parent, UM_RESIZE, RS_CLIENT,
						ViewPort.cw * cfg.zoom[cfg.zoom_i] + ((long)(ViewPort.ch * cfg.zoom[cfg.zoom_i]) << 16)
					);
			}
			break;
		case WM_GETMINMAXINFO:
			{
				RECT rc, rw;
				int i, dx, dy;
				POINT *p;

				GetWindowRect(hwnd, &rw);
				GetClientRect(hwnd, &rc);
				dx = rw.right - rw.left - rc.right;
				dy = rw.bottom - rw.top - rc.bottom;
				glGetIntegerv(GL_MAX_TEXTURE_SIZE, &i);

				p = &((MINMAXINFO *)lparam)->ptMaxTrackSize;
				p->x = i + dx;
				p->y = i + dy;

				p = &((MINMAXINFO *)lparam)->ptMinTrackSize;
				p->x = 16 + dx;
				p->y = 16 + dy;
			}
			break;
		default:
			return DefWindowProc(hwnd, umsg, wparam, lparam);
	}

	return 0;
} 


// MainWND_Create replica
void ViewPort_Create(HWND parent) 
{ 
	RECT *rnc = &ViewPort.rnc;
	
	rnc->left	= GetSystemMetrics(SM_CXSIZEFRAME) + GetSystemMetrics(SM_CXEDGE);
	rnc->bottom	= GetSystemMetrics(SM_CYSIZEFRAME) + GetSystemMetrics(SM_CYEDGE);
	rnc->top	= rnc->bottom + GetSystemMetrics(SM_CYSMCAPTION);
	rnc->right	= rnc->left;

	WNDCLASSEX	wcx = {
		.cbSize			= sizeof(WNDCLASSEX),
		.style			= 0,
		.lpfnWndProc	= ViewPort_Proc,
		.cbClsExtra		= 0,
		.cbWndExtra		= 0,
		.hInstance		= hinst,
		.hIcon			= NULL,
		.hIconSm		= NULL,
		.hCursor		= LoadCursor(NULL, IDC_ARROW),
		.hbrBackground	= CreateSolidBrush(0x00FF0000),
		.lpszMenuName	= NULL,
		.lpszClassName	= "ViewPort_class"
	};
	
	RegisterClassEx(&wcx) || Die(DT_WINAPI);

	ViewPort.hwnd = CreateWindowEx(
			WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_OVERLAPPEDWINDOW,
			"ViewPort_class", "ViewPort",
			WS_POPUP | WS_CAPTION | WS_SIZEBOX | WS_VISIBLE,
			cfg.vp_x, cfg.vp_y, cfg.vp_w + rnc->left + rnc->right, cfg.vp_h + rnc->top + rnc->bottom,
			parent, NULL, hinst, NULL
		);
}
