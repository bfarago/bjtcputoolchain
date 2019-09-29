#include "Std_Types.h"
#include "VideoDrv_fb.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

// #define VCIO_ENABLED
#ifdef VCIO_ENABLED
#include "vcio.h"
#endif

#include "Det.h"
#define VIDEODRV_UNUSED_ENABLE 0
#define UNUSED(x) x=x

#define FBDEF_PATH ("/dev/fb0")

int fb_fd = 0; // framebuffer filedescriptor
int fb_mboxfd = 0; //videocore mailbox fd
struct fb_var_screeninfo fb_vinfo;
struct fb_var_screeninfo fb_orig_vinfo;
struct fb_fix_screeninfo fb_finfo;
char *fb_p = 0;
long int fb_screensize = 0;
unsigned short fb_palold_r[256];
unsigned short fb_palold_g[256];
unsigned short fb_palold_b[256];
unsigned short fb_palold_a[256];
struct fb_cmap fb_palold;
unsigned short fb_palnew_r[256];
unsigned short fb_palnew_g[256];
unsigned short fb_palnew_b[256];
unsigned short fb_palnew_a[256];
struct fb_cmap fb_palnew;
uint8 fb_pal_changed=0;

char VideoDrv_Fb_font8x8_basic[128][8] = {
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0000 (nul)
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0001
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0002
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0003
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0004
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0005
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0006
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0007
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0008
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0009
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+000A
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+000B
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+000C
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+000D
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+000E
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+000F
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0010
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0011
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0012
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0013
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0014
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0015
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0016
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0017
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0018
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0019
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+001A
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+001B
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+001C
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+001D
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+001E
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+001F
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0020 (space)
    { 0x18, 0x3C, 0x3C, 0x18, 0x18, 0x00, 0x18, 0x00},   // U+0021 (!)
    { 0x36, 0x36, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0022 (")
    { 0x36, 0x36, 0x7F, 0x36, 0x7F, 0x36, 0x36, 0x00},   // U+0023 (#)
    { 0x0C, 0x3E, 0x03, 0x1E, 0x30, 0x1F, 0x0C, 0x00},   // U+0024 ($)
    { 0x00, 0x63, 0x33, 0x18, 0x0C, 0x66, 0x63, 0x00},   // U+0025 (%)
    { 0x1C, 0x36, 0x1C, 0x6E, 0x3B, 0x33, 0x6E, 0x00},   // U+0026 (&)
    { 0x06, 0x06, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0027 (')
    { 0x18, 0x0C, 0x06, 0x06, 0x06, 0x0C, 0x18, 0x00},   // U+0028 (()
    { 0x06, 0x0C, 0x18, 0x18, 0x18, 0x0C, 0x06, 0x00},   // U+0029 ())
    { 0x00, 0x66, 0x3C, 0xFF, 0x3C, 0x66, 0x00, 0x00},   // U+002A (*)
    { 0x00, 0x0C, 0x0C, 0x3F, 0x0C, 0x0C, 0x00, 0x00},   // U+002B (+)
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x0C, 0x06},   // U+002C (,)
    { 0x00, 0x00, 0x00, 0x3F, 0x00, 0x00, 0x00, 0x00},   // U+002D (-)
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x0C, 0x00},   // U+002E (.)
    { 0x60, 0x30, 0x18, 0x0C, 0x06, 0x03, 0x01, 0x00},   // U+002F (/)
    { 0x3E, 0x63, 0x73, 0x7B, 0x6F, 0x67, 0x3E, 0x00},   // U+0030 (0)
    { 0x0C, 0x0E, 0x0C, 0x0C, 0x0C, 0x0C, 0x3F, 0x00},   // U+0031 (1)
    { 0x1E, 0x33, 0x30, 0x1C, 0x06, 0x33, 0x3F, 0x00},   // U+0032 (2)
    { 0x1E, 0x33, 0x30, 0x1C, 0x30, 0x33, 0x1E, 0x00},   // U+0033 (3)
    { 0x38, 0x3C, 0x36, 0x33, 0x7F, 0x30, 0x78, 0x00},   // U+0034 (4)
    { 0x3F, 0x03, 0x1F, 0x30, 0x30, 0x33, 0x1E, 0x00},   // U+0035 (5)
    { 0x1C, 0x06, 0x03, 0x1F, 0x33, 0x33, 0x1E, 0x00},   // U+0036 (6)
    { 0x3F, 0x33, 0x30, 0x18, 0x0C, 0x0C, 0x0C, 0x00},   // U+0037 (7)
    { 0x1E, 0x33, 0x33, 0x1E, 0x33, 0x33, 0x1E, 0x00},   // U+0038 (8)
    { 0x1E, 0x33, 0x33, 0x3E, 0x30, 0x18, 0x0E, 0x00},   // U+0039 (9)
    { 0x00, 0x0C, 0x0C, 0x00, 0x00, 0x0C, 0x0C, 0x00},   // U+003A (:)
    { 0x00, 0x0C, 0x0C, 0x00, 0x00, 0x0C, 0x0C, 0x06},   // U+003B (//)
    { 0x18, 0x0C, 0x06, 0x03, 0x06, 0x0C, 0x18, 0x00},   // U+003C (<)
    { 0x00, 0x00, 0x3F, 0x00, 0x00, 0x3F, 0x00, 0x00},   // U+003D (=)
    { 0x06, 0x0C, 0x18, 0x30, 0x18, 0x0C, 0x06, 0x00},   // U+003E (>)
    { 0x1E, 0x33, 0x30, 0x18, 0x0C, 0x00, 0x0C, 0x00},   // U+003F (?)
    { 0x3E, 0x63, 0x7B, 0x7B, 0x7B, 0x03, 0x1E, 0x00},   // U+0040 (@)
    { 0x0C, 0x1E, 0x33, 0x33, 0x3F, 0x33, 0x33, 0x00},   // U+0041 (A)
    { 0x3F, 0x66, 0x66, 0x3E, 0x66, 0x66, 0x3F, 0x00},   // U+0042 (B)
    { 0x3C, 0x66, 0x03, 0x03, 0x03, 0x66, 0x3C, 0x00},   // U+0043 (C)
    { 0x1F, 0x36, 0x66, 0x66, 0x66, 0x36, 0x1F, 0x00},   // U+0044 (D)
    { 0x7F, 0x46, 0x16, 0x1E, 0x16, 0x46, 0x7F, 0x00},   // U+0045 (E)
    { 0x7F, 0x46, 0x16, 0x1E, 0x16, 0x06, 0x0F, 0x00},   // U+0046 (F)
    { 0x3C, 0x66, 0x03, 0x03, 0x73, 0x66, 0x7C, 0x00},   // U+0047 (G)
    { 0x33, 0x33, 0x33, 0x3F, 0x33, 0x33, 0x33, 0x00},   // U+0048 (H)
    { 0x1E, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x1E, 0x00},   // U+0049 (I)
    { 0x78, 0x30, 0x30, 0x30, 0x33, 0x33, 0x1E, 0x00},   // U+004A (J)
    { 0x67, 0x66, 0x36, 0x1E, 0x36, 0x66, 0x67, 0x00},   // U+004B (K)
    { 0x0F, 0x06, 0x06, 0x06, 0x46, 0x66, 0x7F, 0x00},   // U+004C (L)
    { 0x63, 0x77, 0x7F, 0x7F, 0x6B, 0x63, 0x63, 0x00},   // U+004D (M)
    { 0x63, 0x67, 0x6F, 0x7B, 0x73, 0x63, 0x63, 0x00},   // U+004E (N)
    { 0x1C, 0x36, 0x63, 0x63, 0x63, 0x36, 0x1C, 0x00},   // U+004F (O)
    { 0x3F, 0x66, 0x66, 0x3E, 0x06, 0x06, 0x0F, 0x00},   // U+0050 (P)
    { 0x1E, 0x33, 0x33, 0x33, 0x3B, 0x1E, 0x38, 0x00},   // U+0051 (Q)
    { 0x3F, 0x66, 0x66, 0x3E, 0x36, 0x66, 0x67, 0x00},   // U+0052 (R)
    { 0x1E, 0x33, 0x07, 0x0E, 0x38, 0x33, 0x1E, 0x00},   // U+0053 (S)
    { 0x3F, 0x2D, 0x0C, 0x0C, 0x0C, 0x0C, 0x1E, 0x00},   // U+0054 (T)
    { 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x3F, 0x00},   // U+0055 (U)
    { 0x33, 0x33, 0x33, 0x33, 0x33, 0x1E, 0x0C, 0x00},   // U+0056 (V)
    { 0x63, 0x63, 0x63, 0x6B, 0x7F, 0x77, 0x63, 0x00},   // U+0057 (W)
    { 0x63, 0x63, 0x36, 0x1C, 0x1C, 0x36, 0x63, 0x00},   // U+0058 (X)
    { 0x33, 0x33, 0x33, 0x1E, 0x0C, 0x0C, 0x1E, 0x00},   // U+0059 (Y)
    { 0x7F, 0x63, 0x31, 0x18, 0x4C, 0x66, 0x7F, 0x00},   // U+005A (Z)
    { 0x1E, 0x06, 0x06, 0x06, 0x06, 0x06, 0x1E, 0x00},   // U+005B ([)
    { 0x03, 0x06, 0x0C, 0x18, 0x30, 0x60, 0x40, 0x00},   // U+005C (\)
    { 0x1E, 0x18, 0x18, 0x18, 0x18, 0x18, 0x1E, 0x00},   // U+005D (])
    { 0x08, 0x1C, 0x36, 0x63, 0x00, 0x00, 0x00, 0x00},   // U+005E (^)
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF},   // U+005F (_)
    { 0x0C, 0x0C, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0060 (`)
    { 0x00, 0x00, 0x1E, 0x30, 0x3E, 0x33, 0x6E, 0x00},   // U+0061 (a)
    { 0x07, 0x06, 0x06, 0x3E, 0x66, 0x66, 0x3B, 0x00},   // U+0062 (b)
    { 0x00, 0x00, 0x1E, 0x33, 0x03, 0x33, 0x1E, 0x00},   // U+0063 (c)
    { 0x38, 0x30, 0x30, 0x3e, 0x33, 0x33, 0x6E, 0x00},   // U+0064 (d)
    { 0x00, 0x00, 0x1E, 0x33, 0x3f, 0x03, 0x1E, 0x00},   // U+0065 (e)
    { 0x1C, 0x36, 0x06, 0x0f, 0x06, 0x06, 0x0F, 0x00},   // U+0066 (f)
    { 0x00, 0x00, 0x6E, 0x33, 0x33, 0x3E, 0x30, 0x1F},   // U+0067 (g)
    { 0x07, 0x06, 0x36, 0x6E, 0x66, 0x66, 0x67, 0x00},   // U+0068 (h)
    { 0x0C, 0x00, 0x0E, 0x0C, 0x0C, 0x0C, 0x1E, 0x00},   // U+0069 (i)
    { 0x30, 0x00, 0x30, 0x30, 0x30, 0x33, 0x33, 0x1E},   // U+006A (j)
    { 0x07, 0x06, 0x66, 0x36, 0x1E, 0x36, 0x67, 0x00},   // U+006B (k)
    { 0x0E, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x1E, 0x00},   // U+006C (l)
    { 0x00, 0x00, 0x33, 0x7F, 0x7F, 0x6B, 0x63, 0x00},   // U+006D (m)
    { 0x00, 0x00, 0x1F, 0x33, 0x33, 0x33, 0x33, 0x00},   // U+006E (n)
    { 0x00, 0x00, 0x1E, 0x33, 0x33, 0x33, 0x1E, 0x00},   // U+006F (o)
    { 0x00, 0x00, 0x3B, 0x66, 0x66, 0x3E, 0x06, 0x0F},   // U+0070 (p)
    { 0x00, 0x00, 0x6E, 0x33, 0x33, 0x3E, 0x30, 0x78},   // U+0071 (q)
    { 0x00, 0x00, 0x3B, 0x6E, 0x66, 0x06, 0x0F, 0x00},   // U+0072 (r)
    { 0x00, 0x00, 0x3E, 0x03, 0x1E, 0x30, 0x1F, 0x00},   // U+0073 (s)
    { 0x08, 0x0C, 0x3E, 0x0C, 0x0C, 0x2C, 0x18, 0x00},   // U+0074 (t)
    { 0x00, 0x00, 0x33, 0x33, 0x33, 0x33, 0x6E, 0x00},   // U+0075 (u)
    { 0x00, 0x00, 0x33, 0x33, 0x33, 0x1E, 0x0C, 0x00},   // U+0076 (v)
    { 0x00, 0x00, 0x63, 0x6B, 0x7F, 0x7F, 0x36, 0x00},   // U+0077 (w)
    { 0x00, 0x00, 0x63, 0x36, 0x1C, 0x36, 0x63, 0x00},   // U+0078 (x)
    { 0x00, 0x00, 0x33, 0x33, 0x33, 0x3E, 0x30, 0x1F},   // U+0079 (y)
    { 0x00, 0x00, 0x3F, 0x19, 0x0C, 0x26, 0x3F, 0x00},   // U+007A (z)
    { 0x38, 0x0C, 0x0C, 0x07, 0x0C, 0x0C, 0x38, 0x00},   // U+007B ({)
    { 0x18, 0x18, 0x18, 0x00, 0x18, 0x18, 0x18, 0x00},   // U+007C (|)
    { 0x07, 0x0C, 0x0C, 0x38, 0x0C, 0x0C, 0x07, 0x00},   // U+007D (})
    { 0x6E, 0x3B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+007E (~)
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}    // U+007F
};
uint8 gVectors[] =
{
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
};
typedef struct tagBITMAPINFOHEADER {
	uint32      biSize;
	uint32       biWidth;
	uint32       biHeight;
	uint16       biPlanes;
	uint16       biBitCount;
	uint32      biCompression;
	uint32      biSizeImage;
	uint32       biXPelsPerMeter;
	uint32       biYPelsPerMeter;
	uint32      biClrUsed;
	uint32      biClrImportant;
} BITMAPINFOHEADER;

typedef struct {
	BITMAPINFOHEADER info;
	char *buf;
	uint32* pal;
}VideDrv_Bmp_t;

VideDrv_Bmp_t VideoDrv_BmpBkg;
VideDrv_Bmp_t VideoDrv_BmpAbc;
#define VIDEODRV_SCREEN_XOFFS (10)
#define VIDEODRV_SCREEN_YOFFS (10)

uint8 VideoDrv_ScreenBuf[16][16];

#if (1==VIDEODRV_UNUSED_ENABLE)

void VideoDrv_Fb_PutChar_8(int ch, int x, int y, int c){
	ch &=0x7F;
	for (int cy=0; cy<8; cy++)
	{
		unsigned char b= VideoDrv_Fb_font8x8_basic[ch][cy];
		for (int cx=0; cx<8; cx++)
		{
			if ( b & (1<<cx)){
				VideoDrv_Fb_PutPixel_8(x+cx,y+cy,c);
			}
		}
	}
}
void VideoDrv_Fb_PutStr_8(const char* str, int x, int y, int c) {
	int len = strlen(str);
	for (int i = 0; i<len; i++) {
		VideoDrv_Fb_PutChar_8(str[i], x + i * 8, y, c);
	}
}
/*
void VideoDrv_Fb_PutStr_8(const char* str, int x, int y, int c) {
	int len = strlen(str);
	for (int i = 0; i<len; i++) {
		VideoDrv_Fb_PutChar_8(str[i], x + i * 8, y, c);
	}
}
*/
void VideoDrv_Fb_DrawVectors_8(char ch, int x, int y, int m, int c)
{
	uint8* pVec = gVectors + (ch * 32);
	uint8 lx, ly;
	for (int i = 0; i < 32; i++) {
		uint8 b = *pVec;
		uint8 py = (b >> 4)*m;
		uint8 px = (b & 15)*m;
		if (i) {
			VideoDrv_Fb_DrawLine_8(x+lx, y+ly, x+px, y+py, c);
		}
		lx = px; ly = py;
	}
}

void VideoDrv_Fb_DrawVectorStr_8(const char* str, int x, int y, int c) {
	int len = strlen(str);
	for (int i = 0; i<len; i++) {
		VideoDrv_Fb_DrawVectors_8(str[i], x + i * 16, y, 1, c);
	}
}
void VideoDrv_Fb_DrawVectorScreen_8(const char* screen, int x, int y, int lc) {
	for (int py = 0; py < 15; py++) {
		for (int px = 0; px < 15; px++) {
			unsigned char c = screen[py * 16 + px];
			if (c < (sizeof(gVectors) / 32)) {
				VideoDrv_Fb_DrawVectors_8(c, x + px * 16, y + py * 16, 1, lc);
			}
		}
	}
}
#endif

#define DET_MODULE_VIDEODRV_FB 2
#define DET_API_VIDEODRV_FB_LOADBMP 2
#define DET_API_VIDEODRV_E_FILENOTFOUND 2
void VideoDrv_Fb_FlipBmp(VideDrv_Bmp_t* bmp) {
	char* tmp;
	int len = (bmp->info.biWidth* bmp->info.biBitCount) / 8;
	tmp = malloc(len);
	for (uint32 i = 0; i < bmp->info.biHeight / 2; i++) {
		char* p1 = bmp->buf + len * i;
		char* p2 = bmp->buf + len * (bmp->info.biHeight - i-1);
		memcpy(tmp, p1, len);
		memcpy(p1, p2, len);
		memcpy(p2, tmp, len);
	}
	free(tmp);
}
Std_ReturnType VideoDrv_Fb_LoadBmp(const char*fname, VideDrv_Bmp_t* bmp) {
	Std_ReturnType ret = E_NOT_OK;
	FILE* f = fopen(fname, "rb");
	if (!f) {
		Det_ReportRuntimeError(DET_MODULE_VIDEODRV_FB, 0, DET_API_VIDEODRV_FB_LOADBMP, DET_API_VIDEODRV_E_FILENOTFOUND);
		return ret;
	}
	uint8 v8;
	uint32 FileSize, reserved1, DataOffset;
	if (1 != fread(&v8, 1, 1, f)) goto return_ret;
	if ('B' != v8) goto return_ret;
	if (1 != fread(&v8, 1, 1, f)) goto return_ret;
	if ('M' != v8) goto return_ret;
	if (1 != fread(&FileSize, 4, 1, f)) goto return_ret;
	if (1 != fread(&reserved1, 4, 1, f)) goto return_ret;
	if (1 != fread(&DataOffset, 4, 1, f)) goto return_ret;
	VideDrv_Bmp_t* pBmp = bmp;
	
	if (1 != fread(pBmp, 40, 1, f)) goto return_ret;
	if (pBmp->info.biSize < 40) goto return_ret;
	if (pBmp->info.biSize > 40) {
		fseek(f, pBmp->info.biSize - 40, SEEK_CUR);
	}
	//todo: unused members:
	(void)pBmp->info.biPlanes;
	(void)pBmp->info.biCompression;
	(void)pBmp->info.biXPelsPerMeter;
	(void)pBmp->info.biYPelsPerMeter;
	(void)pBmp->info.biClrImportant;

	uint32* pClr = malloc(4* pBmp->info.biClrUsed);
	pBmp->pal = pClr;
	if (1 != fread(pClr, 4 * pBmp->info.biClrUsed, 1, f)) goto return_ret;
	if (DataOffset > 54) {
		fseek(f, DataOffset, SEEK_SET);
	}
	pBmp->info.biSizeImage = pBmp->info.biHeight*pBmp->info.biWidth*pBmp->info.biBitCount / 8;
	pBmp->buf = malloc(pBmp->info.biSizeImage);
	if (1 != fread(pBmp->buf, pBmp->info.biSizeImage, 1, f)) goto return_ret;
	VideoDrv_Fb_FlipBmp(pBmp);
	ret= E_OK;
return_ret:
	fclose(f);
	return ret;
}
Std_ReturnType VideoDrv_Fb_FreeBmp(VideDrv_Bmp_t* bmp) {
	if (bmp->buf) free(bmp->buf);
	if (bmp->pal) free(bmp->pal);
	return E_OK;
}
Std_ReturnType VideoDrv_Fb_DrawBmp(VideDrv_Bmp_t* bmp,int x, int y) {
	VideoDrv_Fb_Copy_8(bmp->buf, 0, 0, bmp->info.biWidth, bmp->info.biHeight, bmp->info.biWidth, x, y);
	return E_OK;
}

#if (1==VIDEODRV_UNUSED_ENABLE)
Std_ReturnType VideoDrv_Fb_CopyFromBmp(VideDrv_Bmp_t* bmp, int sx, int sy, int w, int h, int dx, int dy) {
	VideoDrv_Fb_Copy_8(bmp->buf, sx, sy, w, h, bmp->info.biWidth, dx, dy);
	return E_OK;
}
#endif

Std_ReturnType VideoDrv_Fb_BltFromBmp(VideDrv_Bmp_t* bmp, int sx, int sy, int w, int h, int dx, int dy, uint8 transp) {
	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			uint8 c = *( bmp->buf + sx + x + (sy + y)* bmp->info.biWidth);
			if (c != transp) {
				*(fb_p + dx + x + (y + dy)*fb_finfo.line_length) = c;
			}
		}
	}
	return E_OK;
}

void VideoDrv_Fb_DrawBmpScreen_8(const char* screen, int x, int y) {
	VideoDrv_Fb_DrawBmp(&VideoDrv_BmpBkg, x, y);
	//VideoDrv_Fb_DrawBmp(&VideoDrv_BmpAbc, 400, y);
	x += 24; y += 24;
	for (int py = 0; py < 16; py++) {
		for (int px = 0; px < 16; px++) {
			unsigned char c = screen[py * 16 + px];
			unsigned char tx = c & 15;
			unsigned char ty = c >> 4;
			VideoDrv_Fb_BltFromBmp(&VideoDrv_BmpAbc, tx*13, ty*13, 13, 13, x+ px*13, y+ py * 13, 0);
		}
	}
}

#if (1==VIDEODRV_UNUSED_ENABLE)

void VideoDrv_Fb_PutPixel_8(int x, int y, int c)
{
    // calculate the pixel's byte offset inside the buffer
    unsigned int pix_offset = x + y * fb_finfo.line_length;

    // now this is about the same as 'fbp[pix_offset] = value'
    *((char*)(fb_p + pix_offset)) = c;
}

void VideoDrv_Fb_PutPixel_RGB24(int x, int y, int r, int g, int b)
{
    // calculate the pixel's byte offset inside the buffer
    // note: x * 3 as every pixel is 3 consecutive bytes
    unsigned int pix_offset = x * 3 + y * fb_finfo.line_length;

    // now this is about the same as 'fbp[pix_offset] = value'
    *((char*)(fb_p + pix_offset)) = r;
    *((char*)(fb_p + pix_offset + 1)) = g;
    *((char*)(fb_p + pix_offset + 2)) = b;
}

void VideoDrv_Fb_PutPixel_RGB565(int x, int y, int r, int g, int b)
{
    // calculate the pixel's byte offset inside the buffer
    // note: x * 2 as every pixel is 2 consecutive bytes
    unsigned int pix_offset = x * 2 + y * fb_finfo.line_length;

    // now this is about the same as 'fbp[pix_offset] = value'
    // but a bit more complicated for RGB565
    unsigned short c = ((r / 8) << 11) + ((g / 4) << 5) + (b / 8);
    // or: c = ((r / 8) * 2048) + ((g / 4) * 32) + (b / 8);
    // write 'two bytes at once'
    *((unsigned short*)(fb_p + pix_offset)) = c;
}

void VideoDrv_Fb_FillPxRect_8(int x, int y, int w, int h, int c) {
    int cx, cy;
    for (cy = 0; cy < h; cy++) {
        for (cx = 0; cx < w; cx++) {
            VideoDrv_Fb_PutPixel_8(x + cx, y + cy, c);
        }
    }
}

void VideoDrv_Fb_FillHRect_8(int x, int y, int w, int c) {
	memset(fb_p + x + y * fb_finfo.line_length, c, w);
}

void VideoDrv_Fb_ClearScreen_8(int c) {
	memset(fb_p, c, fb_vinfo.xres * fb_vinfo.yres);
}

void VideoDrv_Fb_DrawLine_8(int x0, int y0, int x1, int y1, int c) {
  int dx = x1 - x0;
  dx = (dx >= 0) ? dx : -dx; // abs()
  int dy = y1 - y0;
  dy = (dy >= 0) ? dy : -dy; // abs()
  int sx;
  int sy;
  if (x0 < x1)
    sx = 1;
  else
    sx = -1;
  if (y0 < y1)
    sy = 1;
  else
    sy = -1;
  int err = dx - dy;
  int e2;
  int done = 0;
  while (!done) {
    VideoDrv_Fb_PutPixel_8(x0, y0, c);
    if ((x0 == x1) && (y0 == y1))
      done = 1;
    else {
      e2 = 2 * err;
      if (e2 > -dy) {
        err = err - dy;
        x0 = x0 + sx;
      }
      if (e2 < dx) {
        err = err + dx;
        y0 = y0 + sy;
      }
    }
  }
}

// (x0, y0) = left top corner coordinates
// w = width and h = height
void VideoDrv_Fb_DrawRect_8(int x0, int y0, int w, int h, int c) {
  //VideoDrv_Fb_DrawLine_8(x0, y0, x0 + w, y0, c); // top
  VideoDrv_Fb_FillHRect_8(x0, y0, w, c); //top
  VideoDrv_Fb_DrawLine_8(x0, y0, x0, y0 + h, c); // left
  //VideoDrv_Fb_DrawLine_8(x0, y0 + h, x0 + w, y0 + h, c); // bottom
  VideoDrv_Fb_FillHRect_8(x0, y0+h, w, c); //bottom
  VideoDrv_Fb_DrawLine_8(x0 + w, y0, x0 + w, y0 + h, c); // right
}

void VideoDrv_Fb_FillRect_8(int x0, int y0, int w, int h, int c) {
  int y;
  for (y = 0; y < h; y++) {
    //VideoDrv_Fb_DrawLine_8(x0, y0 + y, x0 + w, y0 + y, c);
	VideoDrv_Fb_FillHRect_8(x0, y0 + y, w, c);
  }
}

void VideoDrv_Fb_DrawCircle_8(int x0, int y0, int r, int c)
{
  int x = r;
  int y = 0;
  int radiusError = 1 - x;

  while(x >= y)
  {
    // top left
    VideoDrv_Fb_PutPixel_8(-y + x0, -x + y0, c);
    // top right
    VideoDrv_Fb_PutPixel_8(y + x0, -x + y0, c);
    // upper middle left
    VideoDrv_Fb_PutPixel_8(-x + x0, -y + y0, c);
    // upper middle right
    VideoDrv_Fb_PutPixel_8(x + x0, -y + y0, c);
    // lower middle left
    VideoDrv_Fb_PutPixel_8(-x + x0, y + y0, c);
    // lower middle right
    VideoDrv_Fb_PutPixel_8(x + x0, y + y0, c);
    // bottom left
    VideoDrv_Fb_PutPixel_8(-y + x0, x + y0, c);
    // bottom right
    VideoDrv_Fb_PutPixel_8(y + x0, x + y0, c);

    y++;
    if (radiusError < 0)
	{
		radiusError += 2 * y + 1;
	} else {
		x--;
		radiusError+= 2 * (y - x + 1);
    }
  }
}

void VideoDrv_Fb_FillCircle_8(int x0, int y0, int r, int c) {
  int x = r;
  int y = 0;
  int radiusError = 1 - x;

  while(x >= y)
  {
    // top
    VideoDrv_Fb_DrawLine_8(-y + x0, -x + y0, y + x0, -x + y0, c);
    // upper middle
    VideoDrv_Fb_DrawLine_8(-x + x0, -y + y0, x + x0, -y + y0, c);
    // lower middle
    VideoDrv_Fb_DrawLine_8(-x + x0, y + y0, x + x0, y + y0, c);
    // bottom 
    VideoDrv_Fb_DrawLine_8(-y + x0, x + y0, y + x0, x + y0, c);

    y++;
    if (radiusError < 0)
    {
      radiusError += 2 * y + 1;
    } else {
      x--;
      radiusError+= 2 * (y - x + 1);
    }
  }
}
#endif

void VideoDrv_Fb_Copy_8(char* src, int sx, int sy, int sw, int sh, int sl, int dx, int dy){
	for (int y=0; y<sh; y++){
		char* psrc=	 src+ sx+ (y+sy) * sl;
		char* pdest= fb_p+ dx+ (y+dy) * fb_finfo.line_length;
		memcpy(pdest, psrc, sw);
	}
}
#ifdef VCIO_ENABLED
// helper function to talk to the mailbox interface
static int VideoDrv_Fb_MboxProperty(void *buf)
{
   if (fb_mboxfd < -1) return -1;
   
   int ret_val = ioctl(fb_mboxfd, IOCTL_MBOX_PROPERTY, buf);

   if (ret_val < 0) {
      printf("ioctl_set_msg failed:%d\n", ret_val);
   }

   return ret_val;
}

// helper function to set the framebuffer virtual offset == pan
static unsigned VideoDrv_Fb_SetFbVoffs(unsigned *x, unsigned *y)
{
   int i=0;
   unsigned p[32];
   p[i++] = 0; // size
   p[i++] = 0x00000000; // process request

   p[i++] = 0x00048009; // get physical (display) width/height
   p[i++] = 0x00000008; // buffer size
   p[i++] = 0x00000000; // request size
   p[i++] = *x; // value buffer
   p[i++] = *y; // value buffer 2

   p[i++] = 0x00000000; // end tag
   p[0] = i*sizeof *p; // actual size

   VideoDrv_Fb_MboxProperty(p);
   *x = p[5];
   *y = p[6];
   return p[1];
}
#endif

#if (1==VIDEODRV_UNUSED_ENABLE)
void VideoDrv_Fb_TestScreen(void){
	uint16 x, y;
    //unsigned int pix_offset;
/*
    // draw...
    // just fill upper half of the screen with something
    memset(fb_p, 0xff, fb_screensize/2);
    // and lower half with something else
    memset(fb_p + fb_screensize/2, 0x18, fb_screensize/2);
*/
    for (y = 0; y < (fb_vinfo.yres / 2); y++) {
      for (x = 0; x < fb_vinfo.xres; x++) {

        // calculate the pixel's byte offset inside the buffer
        // see the image above in the blog...
        //pix_offset = x + y * fb_finfo.line_length;

        // now this is about the same as fbp[pix_offset] = value
        //*((char*)(fb_p + pix_offset)) = 16 * x / fb_vinfo.xres;
		VideoDrv_Fb_PutPixel_8(x,y,16*x/fb_vinfo.xres);
      }
    }
	VideoDrv_Fb_PutStr_8("TEST screen", 100,100, 15);
}
#endif

Std_ReturnType VideoDrv_Fb_SetPalette() {
	VideDrv_Bmp_t* pBmp = &VideoDrv_BmpBkg;
	memset(fb_palnew_r, 0, 256 * sizeof(unsigned short));
	memset(fb_palnew_g, 0, 256 * sizeof(unsigned short));
	memset(fb_palnew_b, 0, 256 * sizeof(unsigned short));
	memset(fb_palnew_a, 0, 256 * sizeof(unsigned short));
	fb_palnew.start = 0;
	fb_palnew.len =256;
	fb_palnew.red = fb_palnew_r;
	fb_palnew.green = fb_palnew_g;
	fb_palnew.blue = fb_palnew_b;
	fb_palnew.transp = fb_palnew_a;
	if (pBmp->info.biClrUsed > 256) pBmp->info.biClrUsed = 256;

	for (uint32 i = 0; i < pBmp->info.biClrUsed; i++) {
		fb_palnew_a[i] = (pBmp->pal[i] >> 24)<<8;
		fb_palnew_r[i] = ((pBmp->pal[i] >> 16)&0xff) << 8;
		fb_palnew_g[i] = ((pBmp->pal[i] >> 8) & 0xff) << 8;
		fb_palnew_b[i] = (pBmp->pal[i] & 0xff) << 8;
		
		/*
		fb_palnew_a[i] = (pBmp->pal[i] >> 24) << 8;
		fb_palnew_b[i] = ((pBmp->pal[i] >> 16) & 0xff) << 8;
		fb_palnew_g[i] = ((pBmp->pal[i] >> 8) & 0xff) << 8;
		fb_palnew_r[i] = (pBmp->pal[i] & 0xff) << 8;
		*/
	}

	if (ioctl(fb_fd, FBIOPUTCMAP, &fb_palnew)) {
		printf("Error setting palette.\n");
		return E_NOT_OK;
	}
	return E_OK;
}
/**VideoDrv_Init
init screen memory, chargen table, call pwm hal.
*/
Std_ReturnType VideoDrv_Fb_Init(void* p) {
	UNUSED(p);
    Std_ReturnType ret=E_OK;
	
	// Open the framebuffer device file for reading and writing
	fb_fd = open(FBDEF_PATH, O_RDWR);
	if (fb_fd == -1) {
		printf("Error: cannot open framebuffer device.\n");
		return(1);
	}
	printf("The framebuffer device opened.\n");
	// Get variable screen information
	if (ioctl(fb_fd, FBIOGET_VSCREENINFO, &fb_vinfo)) {
		printf("Error reading variable screen info.\n");
	}
	// Store for reset (copy vinfo to vinfo_orig)
	memcpy(&fb_orig_vinfo, &fb_vinfo, sizeof(struct fb_var_screeninfo));
  
	

	// Change variable info
	fb_vinfo.bits_per_pixel = 8;
	if (ioctl(fb_fd, FBIOPUT_VSCREENINFO, &fb_vinfo)) {
		printf("Error setting variable information.\n");
	}
  
	// Get fixed screen information
	if (ioctl(fb_fd, FBIOGET_FSCREENINFO, &fb_finfo)) {
		printf("Error reading fixed information.\n");
	}
	printf("panstep:%d:%d\n", fb_finfo.xpanstep, fb_finfo.ypanstep);
	printf("accel:%x\n", fb_finfo.accel);
	printf("capabilities:%x\n", fb_finfo.capabilities);

	fb_screensize = fb_finfo.smem_len;
	
#ifdef VCIO_ENABLED
	fb_mboxfd = open(DEVICE_FILE_NAME, 0);
	if (fb_mboxfd < 0) {
		printf("Can't open device file: %s\n", DEVICE_FILE_NAME);
		printf("Try creating a device file with: mknod %s c %d 0\n", DEVICE_FILE_NAME, MAJOR_NUM);
		
	}
#endif
	if(1){
		// Get palette information
		memset(fb_palold_r, 0, 256 * sizeof(unsigned short));
		memset(fb_palold_g, 0, 256 * sizeof(unsigned short));
		memset(fb_palold_b, 0, 256 * sizeof(unsigned short));
		memset(fb_palold_a, 0, 256 * sizeof(unsigned short));
		fb_palold.start = 0;
		fb_palold.len = 256;
		fb_palold.red = fb_palold_r;
		fb_palold.green = fb_palold_g;
		fb_palold.blue = fb_palold_b;
		fb_palold.transp = fb_palold_a;
		if (ioctl(fb_fd, FBIOGETCMAP, &fb_palold)) {
			printf("Error reading palette.\n");
		}
		fb_pal_changed=1;
	}
	printf("Display info %dx%d, %d bpp, size:%ld\n", 
			fb_vinfo.xres, fb_vinfo.yres, 
			fb_vinfo.bits_per_pixel, fb_screensize );
  

	fb_p = (char*)mmap(0, 
					fb_screensize, 
					PROT_READ | PROT_WRITE, 
					MAP_SHARED, 
					fb_fd, 0);

	if ((int)fb_p == -1) {
		printf("Failed to mmap.\n");
		ret = E_NOT_OK;
	}
	else {
		// VideoDrv_Fb_TestScreen();
	}
	for (int y = 0; y < 16; y++) {
		for (int x = 0; x < 16; x++) {
			VideoDrv_ScreenBuf[y][x] = (y<<4) + x;
		}
	}

	printf("Load images\n");
	VideoDrv_Fb_LoadBmp("bkg2.bmp", &VideoDrv_BmpBkg);
	VideoDrv_Fb_LoadBmp("abc2.bmp", &VideoDrv_BmpAbc);
	VideoDrv_Fb_SetPalette();
	printf("Video subsystem initialized.\n");
	return ret;
}

Std_ReturnType VideoDrv_Fb_DeInit(void)
{
	VideoDrv_Fb_FreeBmp(&VideoDrv_BmpAbc);
	VideoDrv_Fb_FreeBmp(&VideoDrv_BmpBkg);
	if (fb_pal_changed) {
		if (ioctl(fb_fd, FBIOPUTCMAP, &fb_palold)) {
			printf("Error setting palette.\n");
		}
	}
	close(fb_mboxfd);
	munmap(fb_p, fb_screensize);
	if (ioctl(fb_fd, FBIOPUT_VSCREENINFO, &fb_orig_vinfo)) {
		printf("Error re-setting variable information.\n");
	}

	close(fb_fd);
	printf("The framebuffer device closed.\n");
	return E_OK;
}

void VideoDrv_Fb_MainFunctionDiagnostics(void){}
void VideoDrv_Fb_MainFunctionTest(void){}

/**VideoDrv_MainFunction
Cyclical main function
*/
int r=0;

void VideoDrv_Fb_MainFunction(void){
	
	r++;
	/*
	VideoDrv_Fb_FillRect_8(309, 10, 1, 400, 0);
	VideoDrv_Fb_PutPixel_8(309, 220+100*sin(6.28*r/1000), 14);
	VideoDrv_Fb_Copy_8(fb_p, 10, 10, 300, 400, 800, 9,10);
	*/
#ifdef VCIO_ENABLED
	int x = 15 * sin(r / 1000);
	int y = 0;
	VideoDrv_Fb_SetFbVoffs(&x, &y);
#endif
	VideoDrv_Fb_DrawBmpScreen_8((char*)VideoDrv_ScreenBuf, 80, 40);
}
