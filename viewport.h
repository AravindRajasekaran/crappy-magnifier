/*expecting windows.h was initialized*/

typedef struct {
	HWND hwnd;
	HWND parent;
	int cw, ch;
	RECT rnc;
} t_ViewPort;

extern t_ViewPort ViewPort;

void ViewPort_Create();
