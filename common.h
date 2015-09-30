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
