/*************************************************************************************************************************************************
 * @filename: 01_simple_window.c
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

int 
main(int argc, char* argv[]) {
  Display* display;		/* X Display结构体指针 */
  int screen_num;		/* 屏幕序号，X11角度来看多屏幕与现在多屏幕是不同的  */
  Window win;			/* 被创建的窗口ID */
  unsigned int display_width, 
               display_height;	/*  display显示屏的长和宽 */
  unsigned int width, height;	/* win显示窗口的长和宽  */
  unsigned int win_x, win_y;	/* win显示窗口的坐标*/
  unsigned int win_border_width; /* win显示窗口边界宽度 */
  char *display_name = getenv("DISPLAY");  /* 获取环境变量DISPLAY的值 */
  XEvent ev;

	/**
   * 1. display命名规范 hostname:display_number.screen_number
   *    可以这样理解：一台计算机可以有多个 display，一个 display 可以有多个屏幕。
   *    指定参数的时候：因为可以省略掉 hostname 和 screen_number，所以可以用 :0 、:1 、:2 等等
   * 2. 这个函数用来连接X服务器，Ubuntu Gnome桌面的X服务器就是 Xorg（开机的时候，显示管理器会运行Xorg）
  */
  display = XOpenDisplay(display_name);
  if (display == NULL) {
    fprintf(stderr, "%s: cannot connect to X server '%s'\n",
            argv[0], display_name);
    exit(1);
  }

  /* 得到显示屏幕序号，一般screen_num都是0 */
  screen_num = DefaultScreen(display);
  
  /* 显示屏长宽 */
  display_width = DisplayWidth(display, screen_num);
  display_height = DisplayHeight(display, screen_num);

  /* 新建窗口大小是显示屏大小的三分之一*/
  width = (display_width / 3);
  height = (display_height / 3);

  /* 放到屏幕左上角 */
  win_x = 0;
  win_y = 0;

  /* 1. 新建窗口的边界为2个像素，但是由于窗口管理器的存在，这个边界是看不见的
   * 2. 只有命令行模式下，只启动 Xorg（X服务器），才可以看见边界像素
   */
  win_border_width = 2;

  /**
   * 所有参数在头文件中都有注释，这里特别主要的是 RootWindow(display, screen_num)
  */
  win = XCreateSimpleWindow(display, RootWindow(display, screen_num),
                            win_x, win_y, width, height, win_border_width,
                            BlackPixel(display, screen_num),
                            WhitePixel(display, screen_num));

  /* 设定接受事件类型 */
  XSelectInput(display, win, ButtonPressMask|StructureNotifyMask );

  XMapWindow(display, win); /* 缺少映射到窗口函数，则不会显示 */

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

  while(1){
    /* XNextEvent 是一个阻塞函数 */
    XNextEvent(display, &ev);
    switch(ev.type) {
      case ConfigureNotify:
        if (width != ev.xconfigure.width
            || height != ev.xconfigure.height) {
          width = ev.xconfigure.width;
          height = ev.xconfigure.height;
          printf("Size changed to: %d by %d\n", width, height);
        }
        break;
      case ButtonPress:
        XCloseDisplay(display);
        return 0;
    }
  }
  return(0);
}
