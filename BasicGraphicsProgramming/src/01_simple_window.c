/*************************************************************************************************************************************************
 * @filename: 01_simple_drawing.c
 * @author: EryangLi
 * @version: 1.0
 * @date: Oct-31-2023
 * @brief: Xlib教程part1--开始  学习笔记
 * @note: 
 * @history: 
 *          1. Date:
 *             Author:
 *             Modification:
 *      
 *          2. ..
 *************************************************************************************************************************************************
*/

#include <X11/Xlib.h>

#include <stdio.h>
#include <stdlib.h>		/* getenv(), etc. */
#include <unistd.h>		/* sleep(), etc.  */

int main(int argc, char* argv[])
{
  Display* display;		/* pointer to X Display structure.           */
  int screen_num;		/* number of screen to place the window on.  */
  Window win;			/* pointer to the newly created window.      */
  unsigned int display_width,
               display_height;	/* height and width of the X display.        */
  unsigned int width, height;	/* height and width for the new window.      */
  unsigned int win_x, win_y;	/* location of the window's top-left corner. */
  unsigned int win_border_width; /* width of window's border.                */
  char *display_name = getenv("DISPLAY");  /* address of the X display.      */

	/* 连接到X服务器 */
  display = XOpenDisplay(display_name);
  if (display == NULL) {
    fprintf(stderr, "%s: cannot connect to X server '%s'\n",
            argv[0], display_name);
    exit(1);
  }

  /* get the geometry of the default screen for our display. */
  screen_num = DefaultScreen(display);
  display_width = DisplayWidth(display, screen_num);
  display_height = DisplayHeight(display, screen_num);

  /* make the new window occupy 1/9 of the screen's size. */
  width = (display_width / 3);
  height = (display_height / 3);

  /* the window should be placed at the top-left corner of the screen. */
  win_x = 0;
  win_y = 0;

  /* the window's border shall be 2 pixels wide. */
  win_border_width = 2;

  /* create a simple window, as a direct child of the screen's   */
  /* root window. Use the screen's white color as the background */
  /* color of the window. Place the new window's top-left corner */
  /* at the given 'x,y' coordinates.                             */
  win = XCreateSimpleWindow(display, RootWindow(display, screen_num),
                            win_x, win_y, width, height, win_border_width,
                            BlackPixel(display, screen_num),
                            WhitePixel(display, screen_num));

  /* make the window actually appear on the screen. */
  XMapWindow(display, win); /* 缺少映射到窗口函数，则不会显示 */

  /* flush all pending requests to the X server, and wait until */
  /* they are processed by the X server.                        */

	/* 1. 强制处理之前所有发送到服务器（display）的请求，并且等待所有事件处理完成才返回。
	 * 2. discard: 是一个布尔值。如果设置为True，那么此函数还会清除所有尚未处理的事件。
	 *             如果设置为False，则不会丢弃这些事件。
	 * 3.注意：过度使用XSync可能会导致程序效率降低，因为它会中断程序和服务器之间的通信流，
	 *   并强制服务器处理所有事件。因此，除非有特定的理由，否则最好避免频繁调用此函数。		
	 */
  //XSync(display, False); /* 缺少同步函数则不会显示窗口 */

  /**
   * 函数的主要目的是将任何还未发送到X server的缓冲区中的命令强制发送出去。
   * 这意味着，如果有任何挂起的请求（例如创建窗口、绘图操作等），
   * 它们会被立即发送到X server，但不会等待这些命令被执行。
  */
	XFlush(display);

  /* make a delay for a short period. */
  sleep(4);

  /* close the connection to the X server. */
  XCloseDisplay(display);
  return(0);
}
