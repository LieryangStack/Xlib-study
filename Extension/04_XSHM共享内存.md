# MIT-SHM(The MIT Shared Memory Extension)

## REQUIREMENTS

共享内存扩展只由某些X服务器提供。要查明您的服务器是否支持该扩展，请使用xdpyinfo(1)。特别地，为了能够使用此扩展，您的系统必须提供SYSV共享内存基元。这个扩展没有基于mmap的版本。要在Sun系统上使用共享内存，您必须构建您的内核时启用了SYSV共享内存——这不是默认配置。此外，Sun和Digital系统上的共享内存最大大小都需要增加；默认值对于任何有用的工作来说都太小了。

```
xdpyinfo | grep MIT-SHM
```

## WHAT IS PROVIDED

提供的基本能力是共享内存 XImages。本质上，这是 ximage 接口的一个版本，其中实际的图像数据存储在一个共享内存段中，因此不需要通过 Xlib 进程间通信渠道来传输。对于大型图像，使用这个设施可以实现真正的性能提升。

此外，一些实现提供了共享内存像素图。这些是由 X 服务器指定格式的二维像素数组，其中图像数据存储在共享内存段中。通过使用共享内存像素图，可以在不使用任何 Xlib 程序的情况下更改这些像素图的内容。只有当 X 服务器可以使用常规虚拟内存来存储像素图数据时，才能支持共享内存像素图；<span style="background: pink">如果像素图存储在某些特殊的图形硬件中，应用程序将无法与服务器共享它们。</span> Xdpyinfo(1) 并不打印这一特定的信息。

>注意：如果使用GPU处理，另外该如何处理，暂时还没研究！

## HOW TO USE THE SHARED MEMORY EXTENSION

使用共享内存扩展的代码必须包括多个头文件：

```c
#include <X11/Xlib.h>        /* 当然 */
#include <sys/ipc.h>
#include <sys/shm.h>
#include <X11/extensions/XShm.h>
```

当然，如果你正在构建的系统不支持共享内存，XShm.h 文件可能不存在。

任何使用共享内存扩展的代码都应该首先检查服务器是否提供扩展。你可能总是通过网络运行，或在某些扩展无法工作的其他环境中运行。要进行此检查，请调用：

```c
Status XShmQueryExtension (Display *display)
c
```
或者

```c
Status XShmQueryVersion (Display *display, int *major, int *minor, Bool *pixmaps)
```

其中“display”当然是你正在运行的显示器。如果可以使用共享内存扩展，这两个函数的返回值将是 True；否则你的程序应该使用常规 Xlib 调用进行操作。当扩展可用时，XShmQueryVersion 还会返回“major”和“minor”，这是扩展实现的版本号，以及“pixmaps”，如果支持共享内存像素图，则为 True。

## USE OF SHARED MEMORY XIMAGES

共享内存 XImages 的基本操作序列如下：

1. 创建共享内存 XImage 结构

2. 创建用于存储图像数据的共享内存段

3. 通知服务器关于共享内存段

4. 使用共享内存 XImage，就像使用普通的 XImage 一样。

要创建一个共享内存 XImage，请使用：

```c
XImage *XShmCreateImage (display, visual, depth, format, data,
                         shminfo, width, height)
Display *display;
Visual *visual;
unsigned int depth, width, height;
int format;
char *data;
XShmSegmentInfo *shminfo;
```

大多数参数与 XCreateImage 相同；我不会在这里详细介绍它们。但请注意，没有“offset”、“bitmap_pad”或“bytes_per_line”参数。这些数量将由服务器本身定义，您的代码需要遵守它们。除非您已经分配了共享内存段（见下文），否则应将“data”指针传递为 NULL。

还有一个额外的参数：“shminfo”，这是一个指向 XShmSegmentInfo 类型结构的指针。您必须分配这样一个结构，使其生命周期至少与共享内存 XImage 一样长。在调用 XShmCreateImage 之前，不需要初始化此结构。

如果一切顺利，返回值将是一个 XImage 结构，您可以用它进行后续步骤。

下一步是创建共享内存段。最好是在创建 XImage 之后再做，因为您需要使用该 XImage 中的信息来知道需要分配多少内存。创建段需要一个类似的调用：

```c
shminfo.shmid = shmget (IPC_PRIVATE, image->bytes_per_line * image->height, IPC_CREAT|0777);
```

（假设您调用您的共享内存 XImage 为“image”）。当然，你应该遵循规则，并对所有这些系统调用进行错误检查。此外，一定要使用 bytes_per_line 字段，而不是用来创建 XImage 的宽度，因为它们可能会有所不同。

请注意，系统返回的共享内存 ID 存储在 shminfo 结构中。服务器需要该 ID 来附加到段。

还要注意，由于安全原因，在许多系统上，X 服务器只会同意附加到共享内存段，如果它对“其他”可读写。在能够通过本地传输确定 X 客户端的 uid 的系统上，共享内存段可以只对客户端的 uid 可读写。

接下来，将此共享内存段附加到您的进程：

```c
shminfo.shmaddr = image->data = shmat (shminfo.shmid, 0, 0);
```

由 shmat 返回的地址应该同时存储在 XImage 结构体和 shminfo 结构体中。

为了完成填充 shminfo 结构体，你需要决定如何让服务器附加到共享内存段，并按照以下方式设置 readOnly 字段。通常，你会编写：

```c
shminfo.readOnly = False;
```

如果你将它设置为 True，服务器将无法写入这个段，因此 XShmGetImage 调用将失败。

最后，告诉服务器附加到你的共享内存段：

```c
Status XShmAttach (display, shminfo);
```

如果一切顺利，你会得到一个非零状态返回，你的 XImage 就准备好使用了。

要将共享内存 XImage 写入一个 X drawable，请使用 XShmPutImage：


```c
Status XShmPutImage (display, d, gc, image, src_x, src_y,
                     dest_x, dest_y, width, height, send_event)
Display *display;
Drawable d;
GC gc;
XImage *image;
int src_x, src_y, dest_x, dest_y;
unsigned int width, height;
bool send_event;

```
该接口与 XPutImage 的接口相同，因此我不会在这里重复那些文档。然而，还有一个额外的参数，叫做 send_event。如果这个参数传递为 True，服务器将在图像写入完成时生成一个 完成 事件；这样你的程序就可以知道何时可以安全地开始再次操作共享内存段。

完成事件的类型为 XShmCompletionEvent，定义如下：

```c
typedef struct {
    int   type;              /* 事件的类型 */
    unsigned long serial;   /* 处理的最后一个请求的序号 */
    Bool send_event;         /* 如果来自 SendEvent 请求，则为 true */
    Display *display;        /* 事件读取自的显示器 */
    Drawable drawable;       /* 请求的 drawable */
    int major_code;     /* ShmReqCode */
    int minor_code;     /* X_ShmPutImage */
    ShmSeg shmseg;      /* 请求中使用的 ShmSeg */
    unsigned long offset;   /* 在 ShmSeg 中使用的偏移量 */
} XShmCompletionEvent;
```

运行时将使用的事件类型值可以用如下形式确定：

```c
int CompletionType = XShmGetEventBase (display) + ShmCompletion;
```
如果在完成事件到达之前修改共享内存段，屏幕上看到的结果可能会不一致。

要将图像数据读入共享内存 XImage，请使用以下方法：

```c
Status XShmGetImage (display, d, image, x, y, plane_mask)
Display *display;
Drawable d;
XImage *image;
int x, y;
unsigned long plane_mask;
```

```c
其中 display 是感兴趣的显示器，d 是源 drawable，image 是目标 XImage，x 和 y 是在 d 内的偏移，plane_mask 定义要读取的平面。

要销毁共享内存 XImage，你应该首先指示服务器与它分离，然后销毁段本身，如下所示：
```

```c
XShmDetach (display, shminfo);
XDestroyImage (image);
shmdt (shminfo.shmaddr);
shmctl (shminfo.shmid, IPC_RMID, 0);
```

## USE OF SHARED MEMORY PIXMAPS

与 X 图像不同，任何图像格式都可用，共享内存扩展仅支持单一格式（即 XYPixmap 或 ZPixmap）存储在共享内存像素图中的数据。此格式独立于图像的深度（对于 1 位像素图来说，这个格式是什么并不重要）以及独立于屏幕。使用 XShmPixmapFormat 来获取服务器的格式：

```c
int XShmPixmapFormat (display)
Display *display;
```

如果您的应用程序可以处理服务器像素图数据格式（包括每像素位数等），请以与上面列出的共享内存 XImages 完全相同的方式创建共享内存段和 shminfo 结构。虽然严格来说不必先创建一个 XImage，但这样做几乎没有什么开销，并且会给你一个适当的 bytes_per_line 值来使用。

一旦你填满了 shminfo 结构，只需调用：

```c
Pixmap XShmCreatePixmap (display, d, data, shminfo, width,
                         height, depth);
        Display *display;
        Drawable d;
        char *data;
        XShmSegmentInfo *shminfo;
        unsigned int width, height, depth;
```

所有参数都与 XCreatePixmap 相同，另外两个参数是 data 和 shminfo。后者是之前使用过的同一个老 shminfo 结构。前者是指向共享内存段的指针，应与 shminfo.shmaddr 字段相同。我不确定为什么这是一个单独的参数。

## 参考
[MIT-SHM—The MIT Shared Memory Extension How the shared memory extension works：https://www.xfree86.org/current/mit-shm.html](https://www.xfree86.org/current/mit-shm.html)