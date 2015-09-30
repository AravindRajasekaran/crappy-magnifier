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

/* expecting windows.h was initialized */

typedef struct {
	int w, h;
	BITMAPINFO *bi;
	HBITMAP bmp;
	unsigned long *buf;
} t_dibsect;

typedef struct {
	HWND hwnd;
	unsigned long *buf;
	int cw, ch, tw, th;
	int resize_alg;
} t_MainWND;

extern t_MainWND MainWND;

void MainWND_Create();
