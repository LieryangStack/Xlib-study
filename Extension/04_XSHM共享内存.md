# MIT-SHM(The MIT Shared Memory Extension)

## REQUIREMENTS

共享内存扩展只由某些X服务器提供。要查明您的服务器是否支持该扩展，请使用xdpyinfo(1)。特别地，为了能够使用此扩展，您的系统必须提供SYSV共享内存基元。这个扩展没有基于mmap的版本。要在Sun系统上使用共享内存，您必须构建您的内核时启用了SYSV共享内存——这不是默认配置。此外，Sun和Digital系统上的共享内存最大大小都需要增加；默认值对于任何有用的工作来说都太小了。

```
xdpyinfo | grep MIT-SHM
```

## WHAT IS PROVIDED

## HOW TO USE THE SHARED MEMORY EXTENSION

## USE OF SHARED MEMORY XIMAGES

## USE OF SHARED MEMORY PIXMAPS