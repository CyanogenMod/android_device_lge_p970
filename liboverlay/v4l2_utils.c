/*
 * Copyright (C) Texas Instruments - http://www.ti.com/
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

//#define OVERLAY_DEBUG 1
#define LOG_TAG "Overlay-V4L2"

#include <fcntl.h>
#include <errno.h>
#include <cutils/log.h>
#include <hardware/overlay.h>
#include <linux/videodev.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <stdlib.h>
#include "v4l2_utils.h"
#include <cutils/properties.h>
#include <sys/poll.h>

#define LOG_FUNCTION_NAME    LOGV("%s: %s",  __FILE__, __FUNCTION__);

#ifndef LOGE
#define LOGE(fmt,args...) \
        do { printf(fmt, ##args); } \
        while (0)
#endif

#ifndef LOGI
#define LOGI(fmt,args...) \
        do { LOGE(fmt, ##args); } \
        while (0)
#endif

static int mRotateOverlay = 0;
int v4l2_overlay_get(int name) {
    int result = -1;
    switch (name) {
        case OVERLAY_MINIFICATION_LIMIT:
            result = 4; // 0 = no limit
            break;
        case OVERLAY_MAGNIFICATION_LIMIT:
            result = 2; // 0 = no limit
            break;
        case OVERLAY_SCALING_FRAC_BITS:
            result = 0; // 0 = infinite
            break;
        case OVERLAY_ROTATION_STEP_DEG:
            result = 90; // 90 rotation steps (for instance)
            break;
        case OVERLAY_HORIZONTAL_ALIGNMENT:
            result = 1; // 1-pixel alignment
            break;
        case OVERLAY_VERTICAL_ALIGNMENT:
            result = 1; // 1-pixel alignment
            break;
        case OVERLAY_WIDTH_ALIGNMENT:
            result = 1; // 1-pixel alignment
            break;
        case OVERLAY_HEIGHT_ALIGNMENT:
            result = 1; // 1-pixel alignment
            break;
    }
    return result;
}

int v4l2_overlay_open(int id)
{
    LOG_FUNCTION_NAME
    char v4l2_dev_name[PATH_MAX];
    sprintf(v4l2_dev_name, "/dev/video%d", id + 1);
    LOGI ("Openning %s\n", v4l2_dev_name);
    return open(v4l2_dev_name, O_RDWR);
}

void dump_pixfmt(struct v4l2_pix_format *pix)
{
    LOGI("w: %d\n", pix->width);
    LOGI("h: %d\n", pix->height);
    LOGI("color: %x\n", pix->colorspace);
    switch (pix->pixelformat) {
        case V4L2_PIX_FMT_YUYV:
            LOGI ("YUYV\n");
            break;
        case V4L2_PIX_FMT_UYVY:
            LOGI ("UYVY\n");
            break;
        case V4L2_PIX_FMT_RGB565:
            LOGI ("RGB565\n");
            break;
        case V4L2_PIX_FMT_RGB565X:
            LOGI ("RGB565X\n");
            break;
        case V4L2_PIX_FMT_NV12:
            LOGI ("NV12\n");
            break;
        case V4L2_PIX_FMT_RGB32:
            LOGI ("RGB32\n");
        default:
            LOGI("not supported\n");
    }
}

void dump_crop(struct v4l2_crop *crop)
{
    LOGI("crop l: %d ", crop->c.left);
    LOGI("crop t: %d ", crop->c.top);
    LOGI("crop w: %d ", crop->c.width);
    LOGI("crop h: %d\n", crop->c.height);
}

void dump_window(struct v4l2_window *win)
{
    LOGI("window l: %d ", win->w.left);
    LOGI("window t: %d ", win->w.top);
    LOGI("window w: %d ", win->w.width);
    LOGI("window h: %d\n", win->w.height);
}
void v4l2_overlay_dump_state(int fd)
{
    struct v4l2_format format;
    struct v4l2_crop crop;
    int ret;

    LOGI("dumping driver state:");
    format.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    ret = ioctl(fd, VIDIOC_G_FMT, &format);
    if (ret < 0)
        return;
    LOGI("output pixfmt:\n");
    dump_pixfmt(&format.fmt.pix);

    format.type = V4L2_BUF_TYPE_VIDEO_OVERLAY;
    ret = ioctl(fd, VIDIOC_G_FMT, &format);
    if (ret < 0)
        return;
    LOGI("v4l2_overlay window:\n");
    dump_window(&format.fmt.win);

    crop.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    ret = ioctl(fd, VIDIOC_G_CROP, &crop);
    if (ret < 0)
        return;
    LOGI("output crop:\n");
    dump_crop(&crop);
/*
    crop.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    ret = ioctl(fd, VIDIOC_G_CROP, &crop);
    if (ret < 0)
        return;
    LOGI("ovelay crop:\n");
    dump_crop(&crop);
*/
}

static void error(int fd, const char *msg)
{
  LOGE("Error = %s from %s. errno = %d", strerror(errno), msg, errno);
#ifdef OVERLAY_DEBUG
  v4l2_overlay_dump_state(fd);
#endif
}

static int v4l2_overlay_ioctl(int fd, int req, void *arg, const char* msg)
{
    int ret;
    ret = ioctl(fd, req, arg);
    if (ret < 0) {
        error(fd, msg);
        return -1;
    }
    return 0;
}

int configure_pixfmt(struct v4l2_pix_format *pix, int32_t fmt,
                     uint32_t w, uint32_t h)
{
    LOG_FUNCTION_NAME

    int fd;

   pix->field = V4L2_FIELD_NONE;

   switch (fmt) {
    case OVERLAY_FORMAT_RGBA_8888:
        pix->pixelformat = V4L2_PIX_FMT_RGB32;
        break;
    case OVERLAY_FORMAT_RGB_565:
        pix->pixelformat = V4L2_PIX_FMT_RGB565;
        break;
    case OVERLAY_FORMAT_YCbYCr_422_I:
        pix->pixelformat = V4L2_PIX_FMT_YUYV;
        break;
    case OVERLAY_FORMAT_CbYCrY_422_I:
        pix->pixelformat = V4L2_PIX_FMT_UYVY;
        break;
    case OVERLAY_FORMAT_YCbCr_420_SP_SEQ_TB:
        /* NV12 Interlaced (Sequential Top-Bottom).
           Just update pix.field, and  NV12 params */
        pix->field = V4L2_FIELD_SEQ_TB;
        pix->pixelformat = V4L2_PIX_FMT_NV12;
        pix->bytesperline = 4096;
        break;
    case OVERLAY_FORMAT_YCbCr_420_SP:
        pix->pixelformat = V4L2_PIX_FMT_NV12;
        pix->bytesperline = 4096;
        break;
    //NOTE: we have to add SW color conversion algo
    //      inorder to support these below formats
    case OVERLAY_FORMAT_YCbCr_422_SP:
    case OVERLAY_FORMAT_YCbYCr_420_I:
    case OVERLAY_FORMAT_CbYCrY_420_I:
    case OVERLAY_FORMAT_BGRA_8888:
    return -1;

        default:
            return -1;
    }

    pix->width = w;
    pix->height = h;
    return 0;
}

static void configure_window(struct v4l2_window *win, int32_t w,
                             int32_t h, int32_t x, int32_t y)
{
    LOG_FUNCTION_NAME

    win->w.left = x;
    win->w.top = y;
    win->w.width = w;
    win->w.height = h;
}

void get_window(struct v4l2_format *format, int32_t *x,
                int32_t *y, int32_t *w, int32_t *h)
{
    LOG_FUNCTION_NAME

    *x = format->fmt.win.w.left;
    *y = format->fmt.win.w.top;
    *w = format->fmt.win.w.width;
    *h = format->fmt.win.w.height;
}

int v4l2_overlay_init(int fd, uint32_t w, uint32_t h, uint32_t fmt)
{
    LOG_FUNCTION_NAME

    struct v4l2_format format;
    int ret;

    format.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    ret = v4l2_overlay_ioctl(fd, VIDIOC_G_FMT, &format, "get format");
    if (ret)
        return ret;
    LOGV("v4l2_overlay_init:: w=%d h=%d", format.fmt.pix.width, format.fmt.pix.height);

    format.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    ret = configure_pixfmt(&format.fmt.pix, fmt, w, h);
    if (ret)
        return ret;
    LOGV("v4l2_overlay_init set:: w=%d h=%d fmt=%d", format.fmt.pix.width, format.fmt.pix.height, format.fmt.pix.pixelformat);
    ret = v4l2_overlay_ioctl(fd, VIDIOC_S_FMT, &format, "set output format");

    format.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    ret = v4l2_overlay_ioctl(fd, VIDIOC_G_FMT, &format, "get output format");
    LOGV("v4l2_overlay_init get:: w=%d h=%d fmt=%d", format.fmt.pix.width, format.fmt.pix.height, format.fmt.pix.pixelformat);

    char value[PROPERTY_VALUE_MAX];
    property_get("debug.video.rotateoverlay", value, "0");
    mRotateOverlay = atoi(value);
    LOGD_IF(mRotateOverlay, "overlay rotation enabled");

    return ret;
}

int v4l2_overlay_get_input_size_and_format(int fd, uint32_t *w, uint32_t *h, uint32_t *fmt)
{
    LOG_FUNCTION_NAME

    struct v4l2_format format;
    int ret;

    format.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    ret = v4l2_overlay_ioctl(fd, VIDIOC_G_FMT, &format, "get format");
    *w = format.fmt.pix.width;
    *h = format.fmt.pix.height;
    if (format.fmt.pix.pixelformat == V4L2_PIX_FMT_UYVY)
        *fmt = OVERLAY_FORMAT_YCbYCr_422_I;
    else return -EINVAL;
    return ret;
}


int v4l2_overlay_set_position(int fd, int32_t x, int32_t y, int32_t w, int32_t h)
{
    LOG_FUNCTION_NAME

    struct v4l2_format format;
    int ret;

     /* configure the src format pix */
    /* configure the dst v4l2_overlay window */
    format.type = V4L2_BUF_TYPE_VIDEO_OVERLAY;
    ret = v4l2_overlay_ioctl(fd, VIDIOC_G_FMT, &format,
                             "get v4l2_overlay format");
    if (ret)
       return ret;
    LOGV("v4l2_overlay_set_position:: w=%d h=%d", format.fmt.win.w.width, format.fmt.win.w.height);

    if (mRotateOverlay) {
        w = 800;//480;  // chris-sdc
        h = 480;//800;  // chris-sdc
        x = 0;
        y = 0;
    }
    configure_window(&format.fmt.win, w, h, x, y);

    format.type = V4L2_BUF_TYPE_VIDEO_OVERLAY;
    ret = v4l2_overlay_ioctl(fd, VIDIOC_S_FMT, &format,
                             "set v4l2_overlay format");
    LOGV("v4l2_overlay_set_position:: w=%d h=%d", format.fmt.win.w.width, format.fmt.win.w.height);

    if (ret)
       return ret;
    v4l2_overlay_dump_state(fd);

    return 0;
}

int v4l2_overlay_get_position(int fd, int32_t *x, int32_t *y, int32_t *w, int32_t *h)
{
    struct v4l2_format format;
    int ret;

    format.type = V4L2_BUF_TYPE_VIDEO_OVERLAY;
    ret = v4l2_overlay_ioctl(fd, VIDIOC_G_FMT, &format, "get v4l2_overlay format");
    if (ret)
       return ret;
    get_window(&format, x, y, w, h);
    return 0;
}

int v4l2_overlay_set_crop(int fd, uint32_t x, uint32_t y, uint32_t w, uint32_t h)
{
    LOG_FUNCTION_NAME

    struct v4l2_crop crop;
    int ret;

    crop.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    ret = v4l2_overlay_ioctl(fd, VIDIOC_G_CROP, &crop, "get crop");
    crop.c.left = x;
    crop.c.top = y;
    crop.c.width = w;
    crop.c.height = h;
    crop.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;

    return v4l2_overlay_ioctl(fd, VIDIOC_S_CROP, &crop, "set crop");
}

int v4l2_overlay_get_crop(int fd, uint32_t *x, uint32_t *y, uint32_t *w, uint32_t *h)
{
    LOG_FUNCTION_NAME

    struct v4l2_crop crop;
    int ret;

    crop.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    ret = v4l2_overlay_ioctl(fd, VIDIOC_G_CROP, &crop, "get crop");
    *x = crop.c.left;
    *y = crop.c.top;
    *w = crop.c.width;
    *h = crop.c.height;
    return ret;
}


int v4l2_overlay_set_rotation(int fd, int degree, int step, uint32_t mirror)
{
    LOG_FUNCTION_NAME

    int ret;
    struct v4l2_control ctrl;
    memset(&ctrl, 0, sizeof(ctrl));
    ctrl.id = V4L2_CID_VFLIP;
    ctrl.value = mirror;
    ret = v4l2_overlay_ioctl(fd, VIDIOC_S_CTRL, &ctrl, "set FLIP");

    memset(&ctrl, 0, sizeof(ctrl));
    ctrl.id = V4L2_CID_ROTATE;
    ctrl.value = degree;
    if (mRotateOverlay) {
        ctrl.value = 90;
    }
    ret = v4l2_overlay_ioctl(fd, VIDIOC_S_CTRL, &ctrl, "set rotation");

    return ret;
}

int v4l2_overlay_get_rotation(int fd, int* degree, int step, uint32_t* mirror)
{
    LOG_FUNCTION_NAME
    int ret;
    struct v4l2_control control;
    memset(&control, 0, sizeof(control));
    control.id = V4L2_CID_ROTATE;
    ret = ioctl (fd, VIDIOC_G_CTRL, &control);
    if (ret < 0) {
        error (fd, "VIDIOC_G_CTRL id: V4L2_CID_ROTATE ioctl");
        return ret;
    }

    if (mRotateOverlay) {
        control.value = 90;
    }
    *degree = control.value;

    memset(&control, 0, sizeof(control));
    control.id = V4L2_CID_VFLIP;
    ret = ioctl (fd, VIDIOC_G_CTRL, &control);
    if (ret < 0) {
        error (fd, "VIDIOC_G_CTRL id: V4L2_CID_VFLIP ioctl");
        return ret;
    }

    *mirror = control.value;

    return ret;
}


int v4l2_overlay_set_colorkey(int fd, int enable, int colorkey, int keyType)
{
    LOG_FUNCTION_NAME

    int ret;
    struct v4l2_framebuffer fbuf;
    struct v4l2_format fmt;

    memset(&fbuf, 0, sizeof(fbuf));
    ret = v4l2_overlay_ioctl(fd, VIDIOC_G_FBUF, &fbuf, "get transparency enables");

    if (ret)
        return ret;
#ifdef TARGET_OMAP4
    if (enable)
    {
        if (keyType == EVIDEO_SOURCE) {
            fbuf.flags &= ~V4L2_FBUF_FLAG_CHROMAKEY;
            fbuf.flags |= V4L2_FBUF_FLAG_SRC_CHROMAKEY;
        } else {
            fbuf.flags |= V4L2_FBUF_FLAG_CHROMAKEY;
            fbuf.flags &= ~V4L2_FBUF_FLAG_SRC_CHROMAKEY;
        }
    }
    else
    {
        fbuf.flags &= ~V4L2_FBUF_FLAG_CHROMAKEY;
        fbuf.flags &= ~V4L2_FBUF_FLAG_SRC_CHROMAKEY;
    }
#else
    if (enable)
        fbuf.flags |= V4L2_FBUF_FLAG_CHROMAKEY;
    else
        fbuf.flags &= ~V4L2_FBUF_FLAG_CHROMAKEY;
#endif

    ret = v4l2_overlay_ioctl(fd, VIDIOC_S_FBUF, &fbuf, "enable colorkey");

    if (ret)
        return ret;

    if (enable)

    {
        memset(&fmt, 0, sizeof(fmt));
        fmt.type = V4L2_BUF_TYPE_VIDEO_OVERLAY;
        ret = v4l2_overlay_ioctl(fd, VIDIOC_G_FMT, &fmt, "get colorkey");

        if (ret)
            return ret;

        fmt.fmt.win.chromakey = colorkey & 0xFFFFFF;

        ret = v4l2_overlay_ioctl(fd, VIDIOC_S_FMT, &fmt, "set colorkey");
    }

    return ret;
}

int v4l2_overlay_set_zorder(int fd, int value)
{
    LOG_FUNCTION_NAME
    int ret;
    struct v4l2_format fmt;
    memset(&fmt, 0, sizeof(fmt));
    fmt.type = V4L2_BUF_TYPE_VIDEO_OVERLAY;
    ret = v4l2_overlay_ioctl(fd, VIDIOC_G_FMT, &fmt, "get zorder");
    if (ret)
        return ret;

    fmt.fmt.win.zorder = value & 0x3;
    ret = v4l2_overlay_ioctl(fd, VIDIOC_S_FMT, &fmt, "set zorder");
    return ret;
}

int v4l2_overlay_get_zorder(int fd, int* value)
{
    LOG_FUNCTION_NAME
    int ret;
    struct v4l2_format fmt;
    memset(&fmt, 0, sizeof(fmt));
    fmt.type = V4L2_BUF_TYPE_VIDEO_OVERLAY;
    ret = v4l2_overlay_ioctl(fd, VIDIOC_G_FMT, &fmt, "get zorder");
    if (ret)
        return ret;

    *value = fmt.fmt.win.zorder;
    return ret;
}

int v4l2_overlay_set_global_alpha(int fd, int enable, int alpha)
{
    LOG_FUNCTION_NAME

    int ret;
    struct v4l2_framebuffer fbuf;
    struct v4l2_format fmt;

    memset(&fbuf, 0, sizeof(fbuf));
    ret = v4l2_overlay_ioctl(fd, VIDIOC_G_FBUF, &fbuf, "get transparency enables");

    if (ret)
        return ret;

    if (enable)
        fbuf.flags |= V4L2_FBUF_FLAG_GLOBAL_ALPHA;
    else
        fbuf.flags &= ~V4L2_FBUF_FLAG_GLOBAL_ALPHA;

    ret = v4l2_overlay_ioctl(fd, VIDIOC_S_FBUF, &fbuf, "enable global alpha");

    if (ret)
        return ret;

    if (enable)
    {
        memset(&fmt, 0, sizeof(fmt));
        fmt.type = V4L2_BUF_TYPE_VIDEO_OVERLAY;
        ret = v4l2_overlay_ioctl(fd, VIDIOC_G_FMT, &fmt, "get global alpha");

        if (ret)
            return ret;

        fmt.fmt.win.global_alpha = alpha & 0xFF;

        ret = v4l2_overlay_ioctl(fd, VIDIOC_S_FMT, &fmt, "set global alpha");
    }

    return ret;
}

int v4l2_overlay_set_local_alpha(int fd, int enable)
{
    int ret;
    struct v4l2_framebuffer fbuf;

    ret = v4l2_overlay_ioctl(fd, VIDIOC_G_FBUF, &fbuf,
                             "get transparency enables");

    if (ret)
        return ret;

    if (enable)
        fbuf.flags |= V4L2_FBUF_FLAG_LOCAL_ALPHA;
    else
        fbuf.flags &= ~V4L2_FBUF_FLAG_LOCAL_ALPHA;

    ret = v4l2_overlay_ioctl(fd, VIDIOC_S_FBUF, &fbuf, "enable global alpha");

    return ret;
}

int v4l2_overlay_req_buf(int fd, uint32_t *num_bufs, int cacheable_buffers, int maintain_coherency, int memtype)
{
    LOG_FUNCTION_NAME

    struct v4l2_requestbuffers reqbuf;
    int ret, i;
    reqbuf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    reqbuf.memory = V4L2_MEMORY_MMAP;
    if (memtype == EMEMORY_USRPTR)
    {
        reqbuf.memory = V4L2_MEMORY_USERPTR;
    }
    reqbuf.count = *num_bufs;
    reqbuf.reserved[0] = cacheable_buffers | (maintain_coherency << 1); /* Bit 0 = cacheable_buffers, Bit 1 = maintain_coherency */
    LOGV("reqbuf.reserved[0] = %x", reqbuf.reserved[0]);
    ret = ioctl(fd, VIDIOC_REQBUFS, &reqbuf);
    if (ret < 0) {
        error(fd, "reqbuf ioctl");
        return ret;
    }
    LOGV("%d buffers allocated %d requested", reqbuf.count, 4);
    if (reqbuf.count > *num_bufs) {
        error(fd, "Not enough buffer structs passed to get_buffers");
        return -ENOMEM;
    }
    *num_bufs = reqbuf.count;
    LOGV("buffer cookie is %d", reqbuf.type);
    return 0;
}

static int is_mmaped(struct v4l2_buffer *buf)
{
    return buf->flags == V4L2_BUF_FLAG_MAPPED;
}

static int is_queued(struct v4l2_buffer *buf)
{
    /* is either on the input or output queue in the kernel */
    return (buf->flags & V4L2_BUF_FLAG_QUEUED) ||
           (buf->flags & V4L2_BUF_FLAG_DONE);
}

static int is_dequeued(struct v4l2_buffer *buf)
{
    /* is on neither input or output queue in kernel */
    return (!(buf->flags & V4L2_BUF_FLAG_QUEUED) &&
            !(buf->flags & V4L2_BUF_FLAG_DONE));
}

int v4l2_overlay_query_buffer(int fd, int index, struct v4l2_buffer *buf)
{
    LOG_FUNCTION_NAME

    memset(buf, 0, sizeof(struct v4l2_buffer));

    buf->type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    buf->memory = V4L2_MEMORY_MMAP;
    buf->index = index;
    LOGV("query buffer, mem=%u type=%u index=%u\n", buf->memory, buf->type,
         buf->index);
    return v4l2_overlay_ioctl(fd, VIDIOC_QUERYBUF, buf, "querybuf ioctl");
}

int v4l2_overlay_map_buf(int fd, int index, void **start, size_t *len)
{
    LOG_FUNCTION_NAME

    struct v4l2_buffer buf;
    int ret;

    ret = v4l2_overlay_query_buffer(fd, index, &buf);
    if (ret)
        return ret;

    if (is_mmaped(&buf)) {
        LOGE("Trying to mmap buffers that are already mapped!\n");
        return -EINVAL;
    }

    *len = buf.length;
    *start = mmap(NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED,
                  fd, buf.m.offset & ~(PAGE_SIZE - 1));
    if (*start == MAP_FAILED) {
        LOGE("map failed, length=%u offset=%u\n", buf.length, buf.m.offset);
        return -EINVAL;
    }
    *((__u32 **) start) += buf.m.offset & (PAGE_SIZE - 1);
    return 0;
}

int v4l2_overlay_unmap_buf(void *start, size_t len)
{
    LOG_FUNCTION_NAME

    return munmap((void *) (~(PAGE_SIZE - 1) & (__u32) start), len);
}


int v4l2_overlay_get_caps(int fd, struct v4l2_capability *caps)
{
    return v4l2_overlay_ioctl(fd, VIDIOC_QUERYCAP, caps, "query cap");
}

int v4l2_overlay_stream_on(int fd)
{
    LOG_FUNCTION_NAME

    uint32_t type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    return v4l2_overlay_ioctl(fd, VIDIOC_STREAMON, &type, "stream on");
}

int v4l2_overlay_stream_off(int fd)
{
    LOG_FUNCTION_NAME

    uint32_t type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    return v4l2_overlay_ioctl(fd, VIDIOC_STREAMOFF, &type, "stream off");
}

int v4l2_overlay_q_buf(int fd, int index, int memtype, void* buffer, size_t length)
{
    //LOG_FUNCTION_NAME
    struct v4l2_buffer buf;
    int ret;

    /*
    ret = v4l2_overlay_query_buffer(fd, buffer_cookie, index, &buf);
    if (ret)
        return ret;
    if (is_queued(buf)) {
        LOGE("Trying to queue buffer to kernel that is already queued!\n");
        return -EINVAL
    }
    */
    buf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    buf.index = index;
    buf.memory = V4L2_MEMORY_MMAP;
    if (memtype == EMEMORY_USRPTR)
    {
        buf.memory = V4L2_MEMORY_USERPTR;
        buf.m.userptr = (unsigned long)buffer;
        buf.length = length;
    }
    buf.flags = 0;

    return v4l2_overlay_ioctl(fd, VIDIOC_QBUF, &buf, "qbuf");
}

int v4l2_overlay_dq_buf(int fd, int *index, int memtype, void* buffer, size_t length)
{
    struct v4l2_buffer buf;
    int ret;
    struct pollfd p;

    /*
    ret = v4l2_overlay_query_buffer(fd, buffer_cookie, index, &buf);
    if (ret)
        return ret;

    if (is_dequeued(buf)) {
        LOGE("Trying to dequeue buffer that is not in kernel!\n");
        return -EINVAL
    }
    */
    /* check if buffer is available */

    /* use poll() to timeout gracefully */
    p.fd     = fd;
    p.events = POLLOUT;

    /* for now use 1/15s for timeout */
	ret = poll(&p, 1, 250);  // Tushar - OMAPS00245785 - [] - Changed it to 1/4s OR 4 FPS i.e. 250ms from 67ms

    if (ret <= 0)
    {
	LOGE("ERROR:v4l2_overlay_dq_buf ret = %d, errno = %d", ret, errno);	//shinuk.lee_to_check_errorno
        return ret ? -errno : -EIO;
    }
	
    buf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    buf.memory = V4L2_MEMORY_MMAP;
    if (memtype == EMEMORY_USRPTR)
    {
        buf.memory = V4L2_MEMORY_USERPTR;
        buf.m.userptr = (unsigned long)buffer;
        buf.length = length;
    }
    ret = v4l2_overlay_ioctl(fd, VIDIOC_DQBUF, &buf, "dqbuf");
    if (ret)
    {
	LOGE("ERROR:v4l2_overlay_dq_buf ret = %d, errno = %d", ret, errno);	//shinuk.lee_to_check_errorno
      	return errno;
    }
    *index = buf.index;
    return 0;
}

int v4l2_overlay_getId(int fd, int* id)
{
    LOG_FUNCTION_NAME
    int ret;
    struct v4l2_control ctrl;
    memset(&ctrl, 0, sizeof(ctrl));
    ctrl.id = V4L2_CID_TI_DISPC_OVERLAY;
    ctrl.value = -1;
    ret = v4l2_overlay_ioctl(fd, VIDIOC_G_CTRL, &ctrl, "get path");
    if (ret < 0) {
        error(fd, "VIDIOC_G_CTRL id: V4L2_CID_TI_DISPC_OVERLAY ioctl");
        return ret;
        }
    *id = ctrl.value;
     return ret;
}

enum v4l2_s3d_mode get_s3d_mode(uint32_t mode)
{
    switch(mode) {
        default:
        case OVERLAY_S3D_MODE_OFF:
            return V4L2_S3D_MODE_OFF;
        case OVERLAY_S3D_MODE_ON:
            return V4L2_S3D_MODE_ON;
        case OVERLAY_S3D_MODE_ANAGLYPH:
            return V4L2_S3D_MODE_ANAGLYPH;
    }
}

uint32_t set_s3d_mode(enum v4l2_s3d_mode mode)
{
    switch(mode) {
        default:
        case V4L2_S3D_MODE_OFF:
            return OVERLAY_S3D_MODE_OFF;
        case V4L2_S3D_MODE_ON:
            return OVERLAY_S3D_MODE_ON;
        case V4L2_S3D_MODE_ANAGLYPH:
            return OVERLAY_S3D_MODE_ANAGLYPH;
    }
}

int v4l2_overlay_set_s3d_mode(int fd, uint32_t mode)
{
    LOG_FUNCTION_NAME
    int ret;
    struct v4l2_control ctrl;
    memset(&ctrl, 0, sizeof(ctrl));
    ctrl.id = V4L2_CID_PRIVATE_S3D_MODE;
    ctrl.value = get_s3d_mode(mode);
    ret = v4l2_overlay_ioctl(fd, VIDIOC_S_CTRL, &ctrl, "set s3d mode");
    return ret;
}

int v4l2_overlay_get_s3d_mode(int fd, uint32_t *mode)
{
    LOG_FUNCTION_NAME
    int ret;
    struct v4l2_control ctrl;
    memset(&ctrl, 0, sizeof(ctrl));
    ctrl.id = V4L2_CID_PRIVATE_S3D_MODE;
    ret = v4l2_overlay_ioctl(fd, VIDIOC_G_CTRL, &ctrl, "get s3d mode");
    *mode = set_s3d_mode(ctrl.value);

    return ret;
}

int v4l2_overlay_set_display_id(int fd, uint32_t id)
{
    LOG_FUNCTION_NAME
    int ret;
    struct v4l2_control ctrl;
    memset(&ctrl, 0, sizeof(ctrl));
    ctrl.id = V4L2_CID_PRIVATE_DISPLAY_ID;
    ctrl.value = id;
    ret = v4l2_overlay_ioctl(fd, VIDIOC_S_CTRL, &ctrl, "set display ID");

    return ret;
}

int v4l2_overlay_set_anaglyph_type(int fd, uint32_t id)
{
    LOG_FUNCTION_NAME
    int ret;
    struct v4l2_control ctrl;
    memset(&ctrl, 0, sizeof(ctrl));
    ctrl.id = V4L2_CID_PRIVATE_ANAGLYPH_TYPE;
    ctrl.value = id;
    ret = v4l2_overlay_ioctl(fd, VIDIOC_S_CTRL, &ctrl, "set anaglpyh type");

    return ret;
}

void configure_s3d_format(struct v4l2_frame_packing *frame_packing,
                        uint32_t fmt, uint32_t order, uint32_t subsampling)
{
    enum v4l2_frame_pack_type type;
    enum v4l2_frame_pack_order v4l2_order;
    enum v4l2_frame_pack_sub_sample v4l2_subsampling;

    switch(fmt) {
        default:
        case OVERLAY_S3D_FORMAT_NONE:
            type = V4L2_FPACK_NONE;
            break;
        case OVERLAY_S3D_FORMAT_OVERUNDER:
            type = V4L2_FPACK_OVERUNDER;
            break;
        case OVERLAY_S3D_FORMAT_SIDEBYSIDE:
            type = V4L2_FPACK_SIDEBYSIDE;
            break;
        case OVERLAY_S3D_FORMAT_ROW_IL:
            type = V4L2_FPACK_ROW_IL;
            break;
        case OVERLAY_S3D_FORMAT_COL_IL:
            type = V4L2_FPACK_COL_IL;
            break;
        case OVERLAY_S3D_FORMAT_PIX_IL:
            type = V4L2_FPACK_PIX_IL;
            break;
        case OVERLAY_S3D_FORMAT_CHECKB:
            type = V4L2_FPACK_CHECKB;
            break;
        case OVERLAY_S3D_FORMAT_FRM_SEQ:
            type = V4L2_FPACK_FRM_SEQ;
            break;
    }

    switch(order) {
        default:
        case OVERLAY_S3D_ORDER_LF:
            v4l2_order = V4L2_FPACK_ORDER_LF;
            break;
        case OVERLAY_S3D_ORDER_RF:
            v4l2_order = V4L2_FPACK_ORDER_RF;
            break;
    }

    switch(subsampling) {
        default:
        case OVERLAY_S3D_SS_NONE:
            v4l2_subsampling = V4L2_FPACK_SS_NONE;
            break;
        case OVERLAY_S3D_SS_HOR:
            v4l2_subsampling = V4L2_FPACK_SS_HOR;
            break;
        case OVERLAY_S3D_SS_VERT:
            v4l2_subsampling = V4L2_FPACK_SS_VERT;
            break;
    }

    frame_packing->type = type;
    frame_packing->order = v4l2_order;
    frame_packing->sub_samp = v4l2_subsampling;
}

int v4l2_overlay_set_s3d_format(int fd, uint32_t fmt, uint32_t order, uint32_t subsampling)
{
    LOG_FUNCTION_NAME
    int ret;
    struct v4l2_format format;

    format.type = V4L2_BUF_TYPE_PRIVATE;
    configure_s3d_format((struct v4l2_frame_packing *)format.fmt.raw_data, fmt, order, subsampling);
    ret = v4l2_overlay_ioctl(fd, VIDIOC_S_FMT, &format, "set s3d format");

    return ret;

}

int v4l2_overlay_get_s3d_format(int fd, uint32_t *fmt, uint32_t *order, uint32_t *subsampling)
{

    LOG_FUNCTION_NAME
    int ret;
    struct v4l2_format format;

    format.type = V4L2_BUF_TYPE_PRIVATE;
    ret = v4l2_overlay_ioctl(fd, VIDIOC_G_FMT, &format, "get s3d format");
    *fmt = ((struct v4l2_frame_packing *)format.fmt.raw_data)->type;
    *order =((struct v4l2_frame_packing *)format.fmt.raw_data)->order;
    *subsampling = ((struct v4l2_frame_packing *)format.fmt.raw_data)->sub_samp;

    return ret;
}


/*
Copies 2D buffer to 1D buffer. All heights, widths etc. should be in bytes.
The function copies the lower no. of bytes i.e. if nSize1D < (nHeight2D * nWidth2D)
then nSize1D bytes is copied else (nHeight2D * nWidth2D) bytes is copied.
This function does not return any leftover no. of bytes, the calling function
needs to take care of leftover bytes on its own
*/
static int32_t framenumber = 0;
int32_t Util_Memcpy_2Dto1D(void* pSrc2D, uint32_t nHeight2D, uint32_t nWidth2D, uint32_t nStride2D)
{
   FILE *pOutFile;
   int32_t eError = 0;
   uint8_t *pInBuffer;
   uint32_t i;

   char framenumberstr[100];
   sprintf(framenumberstr, "/patterns/output/frame_%d.txt", framenumber);
   LOGE("file path %s",framenumberstr);
   pOutFile = fopen(framenumberstr, "wb");
   if(pOutFile == NULL)
   {
    LOGE("\n!!!!!!!!!!!!!!!!Error in file open\n");
    return eError;
   }
   LOGE("File Opened");
   pInBuffer = (uint8_t *)pSrc2D;
   LOGE("2Dto1D 0x%x WxH [%d]x[%d]",(int)pInBuffer,nWidth2D,nHeight2D);

    for(i = 0; i < nHeight2D; i++)
    {
    //LOGE("WR%i 0x%x",i,pInBuffer);
    fwrite(pInBuffer, 1, nWidth2D, pOutFile);
    fflush(pOutFile);
        pInBuffer = (uint8_t *)((uint32_t)pInBuffer + nStride2D);
    }
   fclose(pOutFile);
   framenumber++;
   LOGE("Frame dump done");
   return eError;
}

