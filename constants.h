#ifndef CONSTANTS_H
#define CONSTANTS_H

#define MAX(a,b) ((a) > (b) ? (a):(b))
#define MIN(a,b) ((a) < (b) ? (a):(b))

const static int WindowWidth = 300;
const static int WindowMargin = 10;     //边距
const static int WindowLeave = 3;       //隐藏后的剩余空间
const static int TitleHeight = 30;      //Item标题栏高度
const static int StatusBarHeight = 30;  //Item状态栏高度
const static int ItemWidth = WindowWidth - 2 * WindowMargin;
const static int ItemHeight = 200;
const static int ItemMargin = 10;

const static int PixmapWidth = 180;     //图像最大显示宽度
const static int PixmapHeight = 128;    //图像最大显示高度
const static int FileIconWidth = PixmapWidth / 2;
const static int FileIconHeight = PixmapHeight / 2;
const static int PixmapxStep = 15;
const static int PixmapyStep = 5;
const static int ContentMargin = 21;

#endif // CONSTANTS_H
