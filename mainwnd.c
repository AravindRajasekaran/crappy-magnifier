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
#include <gl/gl.h>
#include <gl/glext.h>
#include "common.h"
#include "mainwnd.h"
#include "viewport.h"

t_MainWND MainWND = {};
t_dibsect vp_mdib = {};

void CreateMemDIB(t_dibsect *fb, int w, int h)
{
    HDC dc;

    if (fb->bmp)
        DeleteObject(fb->bmp);
    if (!fb->bi)
        fb->bi = calloc(1, sizeof(BITMAPINFOHEADER) + sizeof(void *));
    fb->w = w;
    fb->h = h;
    fb->bi->bmiHeader = (BITMAPINFOHEADER){
            .biSize             = sizeof(BITMAPINFOHEADER),
            .biWidth            = w,
            .biHeight           = -h,
            .biPlanes           = 1,
            .biBitCount         = 32,
            .biCompression      = BI_RGB,
            .biSizeImage        = 0,
            .biXPelsPerMeter    = 2834,
            .biYPelsPerMeter    = 2834,
            .biClrUsed          = 0,
            .biClrImportant     = 0
        };
    dc = GetDC(HWND_DESKTOP);
    fb->bmp = CreateDIBSection(dc, fb->bi, DIB_RGB_COLORS, (void **)&fb->buf, NULL, 0);
    ReleaseDC(HWND_DESKTOP, dc);
}

void MainWND_LoadConfig(HWND wnd)
{
    MENUITEMINFO mi;
    HMENU menu;
    char st[64];
    int i;

    menu = GetMenu(wnd);

    mi.cbSize           = sizeof(MENUITEMINFO);
    mi.fMask            = MIIM_STATE | MIIM_TYPE;
    mi.fType            = MFT_STRING;
    for (i = 0; i < 10; ++i)
    {
        // zoom sub-menu
        if (i == cfg.zoom_i)
            mi.fState = MFS_CHECKED;
        else
            mi.fState = MFS_UNCHECKED;
        mi.dwTypeData   = st;
        mi.cch          = sprintf(st, "&%d.    x%.2f", (i + 1) % 10, cfg.zoom[i]);
        SetMenuItemInfo(menu, IDM_ZOOM + i, FALSE, &mi);

        // fps sub-menu
        if (i == cfg.fps_i)
            mi.fState = MFS_CHECKED;
        else
            mi.fState = MFS_UNCHECKED;
        mi.dwTypeData   = st;
        mi.cch          = sprintf(st, "&%d.    %d fps", (i + 1) % 10, cfg.fps[i]);
        SetMenuItemInfo(menu, IDM_FPS + i, FALSE, &mi);

    }

    MainWND.resize_alg = 0;
    CheckMenuItem(menu, IDM_ALG_LINEAR, MF_BYCOMMAND | MF_CHECKED);
}

void MainWND_About(HWND hwnd)
{
    char content[100];
    sprintf(content, "Crappy Magnifier v%s\n\nThe crappiest magnifier in the world!", VERSION);
    MessageBox(hwnd, content, "About Crappy Magnifier", MB_OK);
}

LRESULT APIENTRY MainWND_Proc(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam)
{
    int i;

    switch (umsg)
    {
        case WM_CREATE:
            {
                HDC     dc;
                HGLRC   glrc;
                int pfd_idx;

                PIXELFORMATDESCRIPTOR *pfd = &(PIXELFORMATDESCRIPTOR) {
                        .nSize = sizeof(PIXELFORMATDESCRIPTOR),
                        .nVersion = 1,
                        .dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
                        .iPixelType = PFD_TYPE_RGBA,
                        .cColorBits = 24,
                        .cDepthBits = 16,
                        .iLayerType = PFD_MAIN_PLANE
                    };

                dc = GetDC(hwnd);
                pfd_idx = ChoosePixelFormat(dc, pfd);
                if (!SetPixelFormat(dc, pfd_idx, pfd))
                    Die(DT_TEXT, "can't attach this fucking pixel format to ...");
                glrc = wglCreateContext(dc);
                if (!wglMakeCurrent(dc, glrc))
                    Die(DT_TEXT, "wglMakeCurrent failed");

                glEnable(GL_TEXTURE_2D);
                glShadeModel(GL_FLAT);
            }
            MainWND_LoadConfig(hwnd);
            ViewPort_Create(hwnd);
            break;
        case WM_DESTROY:
            {
                HGLRC   glrc;
                HDC     dc;

                glrc = wglGetCurrentContext();
                if (glrc)
                {
                    dc = wglGetCurrentDC();
                    wglMakeCurrent(NULL, NULL);
                    ReleaseDC(hwnd, dc);
                    wglDeleteContext(glrc);
                }
                PostQuitMessage(0);
            }
            break;
        case WM_PAINT:
            {
                HDC dc, cdc;
                HBITMAP oldbmp;
                RECT clt;
                POINT pt;
                GLenum rs_alg;

                if (!MainWND.buf)
                    return DefWindowProc(hwnd, umsg, wparam, lparam);

                pt.x = 0; pt.y = 0;
                ClientToScreen(ViewPort.hwnd, &pt);
                GetClientRect(ViewPort.hwnd, &clt);

                dc = GetDC(HWND_DESKTOP);
                cdc = CreateCompatibleDC(dc);
                oldbmp = SelectObject(cdc, vp_mdib.bmp);
                BitBlt(cdc, 0, 0, clt.right, clt.bottom, dc, pt.x, pt.y, SRCCOPY);
                SelectObject(cdc, oldbmp);
                DeleteDC(cdc);
                ReleaseDC(HWND_DESKTOP, dc);
                for (i = 0; i < vp_mdib.h; i++)
                    memcpy(MainWND.buf + i * MainWND.tw, vp_mdib.buf + i * vp_mdib.w, vp_mdib.w * 4);

                dc = wglGetCurrentDC();

                glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
                glTexImage2D(GL_TEXTURE_2D, 0, 4, MainWND.tw, MainWND.th, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, MainWND.buf);
                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
                switch (MainWND.resize_alg)
                {
                    case 0:
                        rs_alg = GL_LINEAR;
                        break;
                    case 1:
                        rs_alg = GL_NEAREST;
                        break;
                }
                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, rs_alg);
                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, rs_alg);
                glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, 0);

                glBegin(GL_QUADS);
                glTexCoord2f(0.0f, 0.0f);
                glVertex2f(-1.0f,  1.0f);
                glTexCoord2f(1.0f * vp_mdib.w / MainWND.tw, 0.0f);
                glVertex2f( 1.0f,  1.0f);
                glTexCoord2f(1.0f * vp_mdib.w / MainWND.tw, 1.0f * vp_mdib.h / MainWND.th);
                glVertex2f( 1.0f, -1.0f);
                glTexCoord2f(0.0f, 1.0f * vp_mdib.h / MainWND.th);
                glVertex2f(-1.0f, -1.0f);
                glEnd();

                glFlush();
                SwapBuffers(dc);
                ValidateRect(hwnd, NULL);
            }
            return 0;
        case WM_COMMAND:
            switch (HIWORD(wparam))
            {
                // menu
                case 0:
                    {
                        HMENU menu;

                        menu = GetMenu(hwnd);
                        if (LOWORD(wparam) >= IDM_ZOOM && LOWORD(wparam) <= IDM_ZOOM + 9)
                        {
                            cfg.zoom_i = LOWORD(wparam) - IDM_ZOOM;
                            for (i = IDM_ZOOM; i < IDM_ZOOM + 10; ++i)
                                if (i == LOWORD(wparam))
                                    CheckMenuItem(menu, i, MF_BYCOMMAND | MF_CHECKED);
                                else
                                    CheckMenuItem(menu, i, MF_BYCOMMAND | MF_UNCHECKED);
                            SendMessage(hwnd, UM_RESIZE, RS_CLIENT, ViewPort.cw * cfg.zoom[cfg.zoom_i] + ((long)(ViewPort.ch * cfg.zoom[cfg.zoom_i]) << 16));
                            break;
                        }
                        if (LOWORD(wparam) >= IDM_FPS && LOWORD(wparam) <= IDM_FPS + 9)
                        {
                            cfg.fps_i = LOWORD(wparam) - IDM_FPS;
                            for (i = IDM_FPS; i < IDM_FPS + 10; ++i)
                                if (i == LOWORD(wparam))
                                    CheckMenuItem(menu, i, MF_BYCOMMAND | MF_CHECKED);
                                else
                                    CheckMenuItem(menu, i, MF_BYCOMMAND | MF_UNCHECKED);
                            break;
                        }
                        switch (LOWORD(wparam))
                        {
                            case IDM_ALG_LINEAR:
                                MainWND.resize_alg = 0;
                                CheckMenuItem(menu, IDM_ALG_NEAREST, MF_BYCOMMAND | MF_UNCHECKED);
                                CheckMenuItem(menu, IDM_ALG_LINEAR, MF_BYCOMMAND | MF_CHECKED);
                                break;
                            case IDM_ALG_NEAREST:
                                MainWND.resize_alg = 1;
                                CheckMenuItem(menu, IDM_ALG_LINEAR, MF_BYCOMMAND | MF_UNCHECKED);
                                CheckMenuItem(menu, IDM_ALG_NEAREST, MF_BYCOMMAND | MF_CHECKED);
                                break;
                            case IDM_ABOUT:
                                MainWND_About(hwnd);
                                break;
                        }
                    }
            }
            break;
        case WM_SIZE:
            CreateMemDIB(&vp_mdib, ViewPort.cw, ViewPort.ch);

            MainWND.cw = LOWORD(lparam);
            MainWND.ch = HIWORD(lparam);
            MainWND.tw = MainWND.th = 1;
            while (MainWND.tw < vp_mdib.w) MainWND.tw <<= 1;
            while (MainWND.th < vp_mdib.h) MainWND.th <<= 1;
            if (MainWND.buf)
                free(MainWND.buf);
            MainWND.buf = malloc(MainWND.tw * MainWND.th * 4);

            glViewport(0, 0, MainWND.cw, MainWND.ch);
            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            glOrtho(0, 0, MainWND.cw, MainWND.ch, -1, 1);
            break;
        case UM_RESIZE:
            {
                if (wparam == RS_CLIENT)
                {
                    RECT rw, rc;

                    GetWindowRect(hwnd, &rw);
                    GetClientRect(hwnd, &rc);
                    SetWindowPos(
                            hwnd, NULL, 0, 0,
                            rw.right - rw.left - rc.right + LOWORD(lparam),
                            rw.bottom - rw.top - rc.bottom + HIWORD(lparam),
                            SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE
                        );
                }
                else if (wparam == RS_WINDOW)
                    SetWindowPos(
                            hwnd, NULL, 0, 0,
                            LOWORD(lparam),
                            HIWORD(lparam),
                            SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE
                        );
            }
            break;
        case WM_CLOSE:
            {
                RECT rw;

                GetWindowRect(MainWND.hwnd, &rw);
                cfg.mw_x = rw.left;
                cfg.mw_y = rw.top;
                GetWindowRect(ViewPort.hwnd, &rw);
                cfg.vp_x = rw.left;
                cfg.vp_y = rw.top;
                cfg.vp_w = rw.right - rw.left - ViewPort.rnc.left - ViewPort.rnc.right;
                cfg.vp_h = rw.bottom - rw.top - ViewPort.rnc.top - ViewPort.rnc.bottom;
            }
        default:
            return DefWindowProc(hwnd, umsg, wparam, lparam);
    }

    return 0;
}

// no need to split class and window creation section unless you want more than 1 window
void MainWND_Create()
{
    WNDCLASSEX  wcx = {
        .cbSize         = sizeof(WNDCLASSEX),
        .lpfnWndProc    = MainWND_Proc,
        .hInstance      = hinst,
        .hIcon          = LoadIcon(hinst, (LPCTSTR)IDI_MAIN),
        .hIconSm        = NULL,
        .hCursor        = LoadCursor(NULL, IDC_ARROW),
        .hbrBackground  = GetStockObject(NULL_BRUSH),
        .lpszMenuName   = MAKEINTRESOURCE(IDM_MAIN),
        .lpszClassName  = "MainWND_class"
    };

    RegisterClassEx(&wcx) || Die(DT_WINAPI);

    MainWND.hwnd = CreateWindowEx(
            WS_EX_TOPMOST, "MainWND_class", "Crappy Magnifier",
            WS_POPUP | WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX | WS_VISIBLE
            | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
            cfg.mw_x, cfg.mw_y, 0, 0,
            NULL, NULL, hinst, NULL
        );
}
