// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <stdio.h>

#include <X11/Xlib.h>
#include <X11/Xcursor/Xcursor.h>
#include <X11/extensions/Xfixes.h>

#include "xcursor_remap.h"

static int xc_remap(const char *src, const char *dst);

static XcursorImages*
xc_load_images(const char *theme, const char *name, int size)
{
    printf("Will load cursor file: %s - %s\n", theme ? theme : "(default)", name);
    return XcursorLibraryLoadImages(name, theme, size);
}

static void
xc_change_cursor_by_name(Display *dpy, const char *theme,
                 const char *src, const char* dst, int size)
{
    // load cursor images
    XcursorImages *dst_images = xc_load_images(theme, dst, size);
    if (!dst_images) {
        XcursorImagesDestroy(dst_images);
        fprintf(stderr, "Failed to load cursor images: %s/%s\n", src, dst);
        return;
    }

    Cursor dst_cursor = XcursorImagesLoadCursor(dpy, dst_images);

    Window root = XDefaultRootWindow(dpy);
    XUndefineCursor(dpy, root);
    XDefineCursor(dpy, root, dst_cursor);
    printf("Will change cursor: %s --> %lu(%s)\n", src, dst_cursor, dst);
    XFixesChangeCursorByName(dpy, dst_cursor, src);

    XcursorImagesDestroy(dst_images);
    XFreeCursor(dpy, dst_cursor);
    return ;
}

static int
xc_remap(const char *src, const char *dst)
{
    Display *dpy = XOpenDisplay(NULL);
    if (!dpy) {
        fprintf(stderr, "Failed to open display\n");
        return -1;
    }

    const char *theme = XcursorGetTheme(dpy);
    int size = XcursorGetDefaultSize(dpy);

    xc_change_cursor_by_name(dpy, theme, src, dst, size);

    XCloseDisplay(dpy);
    return 0;
}

int
xc_left_ptr_to_watch(int enabled)
{
    if (enabled) {
        return xc_remap("left_ptr", "left_ptr_watch");
    } else {
        return xc_remap("left_ptr_watch", "left_ptr");
    }
}
