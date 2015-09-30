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

#include "resource.h"

#define UM_RESIZE	WM_USER + 1

#define RS_CLIENT	0
#define RS_WINDOW	1

#define DT_WINAPI	1
#define DT_TEXT		2
#define Die(...)	x_Die(__FILE__, __LINE__, __VA_ARGS__)

#ifdef NDEBUG
#	define out(...) ((int)0)
#else
#	define out(...) printf(__VA_ARGS__)
#endif

typedef struct {
	int fps[10];
	float zoom[10];
	int fps_i;
	int zoom_i;
	int mw_x, mw_y;
	int vp_x, vp_y, vp_w, vp_h;
} t_cfg;

extern t_cfg cfg;
extern HINSTANCE hinst;

int x_Die(char *, int, int, ...);
