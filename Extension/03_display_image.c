#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>


XImage *
CreateTrueColorImage(Display *display, Visual *visual, unsigned char *image, int width, int height)
{
    int i, j;

    /* image32 与 p是一个意思 */
    unsigned char *image32=(unsigned char *)malloc(width*height*4);
    unsigned char *p=image32;

    printf ("image32 = %p\n", image32);
    printf ("p = %p\n", p);

    for(i=0; i<width; i++)
    {
        for(j=0; j<height; j++)
        {   
            /* 第一个区域BGR随机 */
            if((i<256)&&(j<256))
            {
                *p++=rand()%256; // blue
                *p++=rand()%256; // green
                *p++=rand()%256; // red
            }
            else
            {
                *p++=i%256; // blue
                *p++=j%256; // green
                if(i<256)
                    *p++=i%256; // red
                else if(j<256)
                    *p++=j%256; // red
                else
                    *p++=(256-j)%256; // red
            }
            /* 阿尔法通道好像没有用*/
            p++;
        }
    }
    /**
     * 以下是XCreateImage函数的定义,各参数的描述如下：
     * @param display: 与X服务器的连接的指针。
     * @param visual：描述如何在屏幕上显示图像的Visual结构的指针。通常，你可以使用DefaultVisual宏获取默认的Visual。
     * @param depth：图像的深度（像素中的位数）。例如，8、16、24或32。（一般都是3 * 8 = 24）
     * @param format: 图像的格式。这通常是以下常量之一：
     *              XYBitmap: 图像是一个单位深度的位图。
     *              XYPixmap: 图像数据是一个位平面的列表。
     *              ZPixmap: 图像数据是像素值的数组。
     * @param offset：指定图像数据开始的字节的偏移。通常设置为0。
     * @param data：一个指针，指向包含图像数据的内存
     * @param width：图像的的宽度，以像素为单位。
     * @param height：图像的高度，以像素为单位。
     * @param bitmap_pad: 指定每行像素数据的对齐方式。常见的值是8、16或32。例如，对于24位深度的图像，
     *                    你通常会使用32作为bitmap_pad，这意味着每个像素数据被填充到32位。
     * @param bytes_per_line: 指定图像中每一行的字节数。如果为0，函数会自动计算这个值。
    */
    return XCreateImage(display, visual, DefaultDepth(display,DefaultScreen(display)), ZPixmap, 0, image32, width, height, 32, 0);
}

void processEvent(Display *display, Window window, XImage *ximage, int width, int height)
{
    static char *tir="This is red";
    static char *tig="This is green";
    static char *tib="This is blue";
    XEvent ev;
    XNextEvent(display, &ev);
    switch(ev.type)
    {
    case Expose:
        /**
         * @brief: XPutImage函数是Xlib中用于将XImage结构中的图像数据绘制到一个drawable上的函数。这个"drawable"可以是一个窗口或是一个Pixmap。
         * @param display: 与X服务器的连接的指针。
         * @param d: 目标drawable（窗口或Pixmap），你希望绘制图像到这个drawable上。
         * @param gc: Graphics context，它定义了如何绘制图像，包括颜色、线条风格、填充模式等。
         * @param image: 一个指向XImage结构的指针，这个结构包含你想要绘制的图像数据。
         * @param src_x, src_y: 图像数据中的起始坐标，指定从哪里开始复制图像。
         * @param dest_x, dest_y: 目标drawable中的坐标，指定图像数据应该被绘制的位置。
         * @param width, height: 需要复制的图像数据的宽度和高度。
        */
        XPutImage(display, window, DefaultGC(display, 0), ximage, 0, 0, 0, 0, width, height);
        XSetForeground(display, DefaultGC(display, 0), 0x00ff0000); // red
        XDrawString(display, window, DefaultGC(display, 0), 32,     32,     tir, strlen(tir));
        XDrawString(display, window, DefaultGC(display, 0), 32+256, 32,     tir, strlen(tir));
        XDrawString(display, window, DefaultGC(display, 0), 32+256, 32+256, tir, strlen(tir));
        XDrawString(display, window, DefaultGC(display, 0), 32,     32+256, tir, strlen(tir));
        XSetForeground(display, DefaultGC(display, 0), 0x0000ff00); // green
        XDrawString(display, window, DefaultGC(display, 0), 32,     52,     tig, strlen(tig));
        XDrawString(display, window, DefaultGC(display, 0), 32+256, 52,     tig, strlen(tig));
        XDrawString(display, window, DefaultGC(display, 0), 32+256, 52+256, tig, strlen(tig));
        XDrawString(display, window, DefaultGC(display, 0), 32,     52+256, tig, strlen(tig));
        XSetForeground(display, DefaultGC(display, 0), 0x000000ff); // blue
        XDrawString(display, window, DefaultGC(display, 0), 32,     72,     tib, strlen(tib));
        XDrawString(display, window, DefaultGC(display, 0), 32+256, 72,     tib, strlen(tib));
        XDrawString(display, window, DefaultGC(display, 0), 32+256, 72+256, tib, strlen(tib));
        XDrawString(display, window, DefaultGC(display, 0), 32,     72+256, tib, strlen(tib));
        break;
    case ButtonPress:
        /**
         * 当你调用XDestroyImage函数后，image指针将不再是有效的，并且你不应该再使用它。
         * 请注意，XDestroyImage函数会释放图像的数据内存（data成员），只有当obdata成员是NULL时才会这样做。
         * 如果你已经手动释放了data或使用了自定义的内存分配程序，你需要确保在调用XDestroyImage之前处理这些情况，以防止内存泄漏或双重释放。
        */
        XCloseDisplay(display);
        printf ("ximage->data = %p\n", ximage->data);
        XDestroyImage(ximage);
        // XDestroyImage会调用 free释放ximage->data
        exit(0);
    }
}

int main(int argc, char **argv)
{
    XImage *ximage;
    int width=512, height=512;
    Display *display = XOpenDisplay(NULL);
    /**
     * Visual"是与显示设备的特定硬件能力有关的数据结构。
     * 它描述了色彩映射如何与物理显示设备的特定部分相对应。
     * Visual结构为应用程序提供了有关像素值如何转换为实际颜色的信息。
     * 在X系统中，颜色显示的深度、颜色模型和硬件实现的颜色能力都是由Visual来描述的。
    */
    Visual *visual = DefaultVisual(display, 0);
    Window window = XCreateSimpleWindow(display, RootWindow(display, 0), 0, 0, width, height, 1, 0, 0);
    if(visual->class!=TrueColor)
    {
        fprintf(stderr, "Cannot handle non true color visual ...\n");
        exit(1);
    }

    ximage = CreateTrueColorImage(display, visual, 0, width, height);
    XSelectInput(display, window, ButtonPressMask|ExposureMask);
    XMapWindow(display, window);
    while(1)
    {
        processEvent(display, window, ximage, width, height);
    }
}