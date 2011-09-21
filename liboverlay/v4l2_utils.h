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

#ifndef ANDROID_ZOOM_REPO_HARDWARE_TI_OMAP3_LIBOVERLAY_V4L2_UTILS_H_
#define ANDROID_ZOOM_REPO_HARDWARE_TI_OMAP3_LIBOVERLAY_V4L2_UTILS_H_

#include <linux/videodev.h>

#define OVERLAY_FORMAT_ARGB_8888 100
#define OVERLAY_FORMAT_ARGB_4444 200

typedef struct
{
    int fd;
    size_t length;
    uint32_t offset;
    void *ptr;
} mapping_data_t;

enum
{
    EMEMORY_MMAP   = 0x0,
    EMEMORY_USRPTR = 0x1
};

enum
{
   EVIDEO_SOURCE         = 0x0,
   EGRAPHICS_DESTINATION = 0x1

};

int v4l2_overlay_open(int id);
int v4l2_overlay_get_caps(int fd, struct v4l2_capability *caps);
int v4l2_overlay_req_buf(int fd, uint32_t *num_bufs, int cacheable_buffers, int maintain_coherency, int memtype);
int v4l2_overlay_query_buffer(int fd, int index, struct v4l2_buffer *buf);
int v4l2_overlay_map_buf(int fd, int index, void **start, size_t *len);
int v4l2_overlay_unmap_buf(void *start, size_t len);
int v4l2_overlay_stream_on(int fd);
int v4l2_overlay_stream_off(int fd);
int v4l2_overlay_q_buf(int fd, int index, int memtype, void* buffer, size_t length);
int v4l2_overlay_dq_buf(int fd, int *index, int memtype, void* buffer, size_t length);
int v4l2_overlay_init(int fd, uint32_t w, uint32_t h, uint32_t fmt);
int v4l2_overlay_get_input_size(int fd, uint32_t *w, uint32_t *h, uint32_t *fmt);
int v4l2_overlay_set_position(int fd, int32_t x, int32_t y, int32_t w,
                              int32_t h);
int v4l2_overlay_get_position(int fd, int32_t *x, int32_t *y, int32_t *w,
                              int32_t *h);
int v4l2_overlay_set_crop(int fd, uint32_t x, uint32_t y, uint32_t w,
                              uint32_t h);
int v4l2_overlay_get_crop(int fd, uint32_t *x, uint32_t *y, uint32_t *w,
                              uint32_t *h);
int v4l2_overlay_set_rotation(int fd, int degree, int step, uint32_t mirror);
int v4l2_overlay_get_rotation(int fd, int* degree, int step, uint32_t* mirror);
int v4l2_overlay_set_colorkey(int fd, int enable, int colorkey, int keyType);
int v4l2_overlay_set_global_alpha(int fd, int enable, int alpha);
int v4l2_overlay_set_local_alpha(int fd, int enable);
int v4l2_overlay_set_zorder(int fd, int value);
int v4l2_overlay_get_zorder(int fd, int* value);

int32_t Util_Memcpy_2Dto1D(void* pSrc2D, uint32_t nHeight2D, uint32_t nWidth2D, uint32_t nStride2D);
int v4l2_overlay_getId(int fd, int* id);

int v4l2_overlay_set_s3d_mode(int fd, uint32_t mode);
int v4l2_overlay_get_s3d_mode(int fd, uint32_t *mode);
int v4l2_overlay_set_s3d_format(int fd, uint32_t fmt, uint32_t order, uint32_t subsampling);
int v4l2_overlay_get_s3d_format(int fd, uint32_t *fmt, uint32_t *order, uint32_t *subsampling);
int v4l2_overlay_set_display_id(int fd, uint32_t id);
int v4l2_overlay_set_anaglyph_type(int fd, uint32_t type);

enum {
  V4L2_OVERLAY_PLANE_GRAPHICS,
  V4L2_OVERLAY_PLANE_VIDEO1,
  V4L2_OVERLAY_PLANE_VIDEO2,
};

/* this constant should be insync with that defined in kernel v4l2 header
*/
#define V4L2_CID_TI_DISPC_OVERLAY 0x08000000

/*S3D private definitions, should be in sync with S3D V4L2 kernel header*/
#define VIDIOC_PRIVATE_S3D_S_OFFS \
    _IOWR('V', BASE_VIDIOC_PRIVATE + 1, struct v4l2_s3d_offsets)

#define V4L2_CID_PRIVATE_DISPLAY_ID (V4L2_CID_PRIVATE_BASE+1)
#define V4L2_CID_PRIVATE_ANAGLYPH_TYPE (V4L2_CID_PRIVATE_BASE+2)
#define V4L2_CID_PRIVATE_S3D_MODE (V4L2_CID_PRIVATE_BASE+3)

enum v4l2_anaglyph_type {
    /*Left view Red, right view Cyan */
    V4L2_ANAGLYPH_RED_CYAN = 0,
    /*Left view Red, right view Blue */
    V4L2_ANAGLYPH_RED_BLUE = 1,
    /*Left view Green, right view Magenta */
    V4L2_ANAGLYPH_GR_MAG,
    V4L2_ANAGLYPH_MAX
};

enum v4l2_s3d_mode {
    V4L2_S3D_MODE_OFF = 0,
    V4L2_S3D_MODE_ON,
    V4L2_S3D_MODE_ANAGLYPH,
    V4L2_S3D_MODE_MAX
};

enum v4l2_frame_pack_type {
    V4L2_FPACK_NONE = 0,
    V4L2_FPACK_OVERUNDER,
    V4L2_FPACK_SIDEBYSIDE,
    V4L2_FPACK_ROW_IL,
    V4L2_FPACK_COL_IL,
    V4L2_FPACK_PIX_IL,
    V4L2_FPACK_CHECKB,
    V4L2_FPACK_FRM_SEQ,
};

enum v4l2_frame_pack_order {
    V4L2_FPACK_ORDER_LF = 0,
    V4L2_FPACK_ORDER_RF,
};

enum v4l2_frame_pack_sub_sample {
    V4L2_FPACK_SS_NONE = 0,
    V4L2_FPACK_SS_HOR,
    V4L2_FPACK_SS_VERT,
};

struct v4l2_frame_packing {
    enum v4l2_frame_pack_type type;
    enum v4l2_frame_pack_order order;
    enum v4l2_frame_pack_sub_sample sub_samp;
};

struct v4l2_s3d_offsets {
    /*Offset from base address to active left view start */
    unsigned long l;
    /*Offset from base address to active right view start */
    unsigned long r;
    /*Cropping w and h */
    unsigned int w;
    unsigned int h;
};

#define V4L2_CID_ROTATE         0x00980900 + 34  //(V4L2_CID_BASE+34)

#endif  // ANDROID_ZOOM_REPO_HARDWARE_TI_OMAP3_LIBOVERLAY_V4L2_UTILS_H_
