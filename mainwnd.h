/*expecting windows.h was initialized*/

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
