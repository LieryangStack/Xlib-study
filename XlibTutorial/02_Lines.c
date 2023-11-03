/* first include the standard headers that we're likely to need */
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xresource.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void forwardEventToChildren(Display *display, Window parent, XEvent *event) {
    Window root, parent_return;
    Window *children;
    unsigned int nchildren;

    // 获取子窗口列表
    if (XQueryTree(display, parent, &root, &parent_return, &children, &nchildren)) {
        for (unsigned int i = 0; i < nchildren; i++) {
            // 发送事件到子窗口
            XSendEvent(display, children[i], True, ExposureMask, event);
            // 递归处理孙窗口
            forwardEventToChildren(display, children[i], event);
        }
        // 释放窗口列表资源
        XFree(children);
    }
}

int main(int argc, char ** argv){
	int screen_num, width, height;
	unsigned long background, border;
	Window win;
	XEvent ev;
	Display *dpy;
	GC pen;
	XGCValues values;

	/* First connect to the display server */
	dpy = XOpenDisplay(NULL);
	if (!dpy) {fprintf(stderr, "unable to connect to display\n");return 7;}

	/* these are macros that pull useful data out of the display object */
	/* we use these bits of info enough to want them in their own variables */
	screen_num = DefaultScreen(dpy);
	background = BlackPixel(dpy, screen_num);
	border = WhitePixel(dpy, screen_num);

	width = 400; /* start with a small window */
	height = 400;

	win = XCreateSimpleWindow(dpy, DefaultRootWindow(dpy), /* display, parent */
		0,0, /* x, y: the window manager will place the window elsewhere */
		width, height, /* width, height */
		2, border, /* border width & colour, unless you have a window manager */
		background); /* background colour */

	printf ("win = 0x%lx\n", win);

	/* create the pen to draw lines with */
	values.foreground = WhitePixel(dpy, screen_num);
	values.line_width = 1;
	values.line_style = LineSolid;
	pen = XCreateGC(dpy, win, GCForeground|GCLineWidth|GCLineStyle,&values);


	/* tell the display server what kind of events we would like to see */
	XSelectInput(dpy, win, ButtonPressMask|StructureNotifyMask|ExposureMask|
	                       SubstructureNotifyMask);

	/* okay, put the window on the screen, please */
	XMapWindow(dpy, win);

	/* as each event that we asked about occurs, we respond.  In this
	 * case we note if the window's shape changed, and exit if a button
	 * is pressed inside the window.  We also draw lines whenever a part
	 * of a window becomes viewable. */
	while(1){
		XNextEvent(dpy, &ev);
		switch(ev.type){
		case Expose:
			forwardEventToChildren(dpy, win, &ev);
			XDrawLine(dpy, win, pen, 0, 0, width, height);
			XDrawLine(dpy, win, pen, width, 0, 0, height);
			printf("Expose changed to: %d by %d\n", width, height);
			break;
		case ConfigureNotify:
			if (width != ev.xconfigure.width
					|| height != ev.xconfigure.height) {
				width = ev.xconfigure.width;
				height = ev.xconfigure.height;
				XClearWindow(dpy, ev.xany.window);
				printf("Size changed to: %d by %d\n", width, height);
			}
			break;
		case CreateNotify:
			printf("Child window with ID %lu created.\n", ev.xcreatewindow.window);
			// 创建并发送Exposure事件
			XExposeEvent expose_event;
			expose_event.type = Expose;
			expose_event.display = dpy;
			expose_event.window = ev.xcreatewindow.window;
			expose_event.x = 0;
			expose_event.y = 0;
			expose_event.width = 300;
			expose_event.height = 300;
			expose_event.count = 0;

			XSendEvent(dpy, ev.xcreatewindow.window, False, ExposureMask, (XEvent *)&expose_event);
			XFlush(dpy);  // 确保事件立即发送
			break;
		case ButtonPress:
			XCloseDisplay(dpy);
			return 0;
		}
	}
}