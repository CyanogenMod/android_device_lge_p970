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

#ifndef TIOVERLAY_H_
#define TIOVERLAY_H_

#include <hardware/overlay.h>
#include "overlay_common.h"
#include "v4l2_utils.h"

#define OVERLAY_DATA_MARKER  (0x68759746) // OVRLYSHM on phone keypad

/** this structure represents
 * an overlay. here we use a subclass, where we can store our own state.
 * This handles will be passed across processes and possibly given to other
 * HAL modules (for instance video decode modules).
 */
struct handle_t : public native_handle {
    /* add the data fields we need here, for instance: */
    int video_fd;
    int overlayobj_sharedfd;
    int overlayobj_size;
    int overlayobj_index;
};

//forward declaration
class overlay_control_context_t;
class overlay_data_context_t;

class overlay_ctrl_t {
public:
    overlay_ctrl_t() {
        // position maintained here is wrt to DSS.
        posX = 0;
        posY = 0;
        posW = LCD_WIDTH;
        posH = LCD_HEIGHT;
        colorkey = 0;
        rotation = 0;
        alpha = 0;
        zorder = 3;
        panel = 0x0;
        mirror = 0x0;

    };
public:
  uint32_t posX;
  uint32_t posY;
  uint32_t posW;
  uint32_t posH;
  int32_t colorkey;
  uint32_t rotation;
  uint32_t alpha;
  uint32_t zorder;
  uint32_t panel;
  uint32_t mirror;
} ;

class overlay_data_t {
public:
    overlay_data_t() {
        cropX = 0;
        cropY = 0;
        cropW = LCD_WIDTH;
        cropH = LCD_HEIGHT;
        s3d_mode = OVERLAY_S3D_MODE_OFF;
        s3d_fmt = OVERLAY_S3D_FORMAT_NONE;
        s3d_order = OVERLAY_S3D_ORDER_LF;
        s3d_subsampling = OVERLAY_S3D_SS_NONE;
    }

public:
  uint32_t cropX;
  uint32_t cropY;
  uint32_t cropW;
  uint32_t cropH;
  bool s3d_active;
  uint32_t s3d_mode;
  uint32_t s3d_fmt;
  uint32_t s3d_order;
  uint32_t s3d_subsampling;
};


// A separate instance of this class is created per overlay
class overlay_object : public overlay_t {
public:
    handle_t mControlHandle;
    handle_t mDataHandle;
    int mLinkVideoCtrlfd; //fd for the video device getting linked
    int mLinkVideoDatafd; //fd for the video device getting linked
    uint32_t marker;
    volatile int32_t refCnt;

    size_t *buffers_len;
    void **buffers;
    int num_buffers;

    uint32_t controlReady; // Only updated by the control side
    uint32_t dataReady;    // Only updated by the data side
    uint32_t streamEn;

    pthread_mutex_t lock;
    pthread_mutexattr_t attr;

    uint32_t dispW;
    uint32_t dispH;

    // Need to count Qd buffers to be sure we don't block DQ'ing when exiting
    int qd_buf_count;

    overlay_ctrl_t      mCtl;
    overlay_ctrl_t      mCtlStage;
    overlay_data_t      mData;
    mapping_data_t*     mapping_data;

    int cacheable_buffers;
    int maintain_coherency;
    int optimalQBufCnt;
    int mappedbufcount;
    int attributes_changed;

    char overlaymanagerpath[PATH_MAX];
    char overlayenabled[PATH_MAX];

    struct displayMetaData {
        int mPanelIndex;
        int mManagerIndex;
        int mTobeDisabledPanelIndex;
        };
    displayMetaData mDisplayMetaData;
    static overlay_handle_t getHandleRef(struct overlay_t* overlay) {
        /* returns a reference to the handle, caller doesn't take ownership */
        return &(static_cast<overlay_object *>(overlay)->mControlHandle);
    }

public:
    void init(int ctlfd, int w, int h, int format, int numbuffers, int index) {
        this->overlay_t::getHandleRef = getHandleRef;
        mControlHandle.version     = sizeof(native_handle);
        mControlHandle.numFds      = 2;
        mControlHandle.numInts     = 2; // extra ints we have in our handle
        mControlHandle.video_fd = ctlfd;
        mControlHandle.overlayobj_index = index;
        mDataHandle.overlayobj_index = index;
        this->w = w;
        this->h = h;
        this->w_stride = 0;
        this->h_stride = 0;
        this->format = format;
        this->num_buffers = numbuffers;
        this->mLinkVideoCtrlfd = -1;
        this->mLinkVideoDatafd = -1;
        memset( &mCtl, 0, sizeof( mCtl ) );
        memset( &mCtlStage, 0, sizeof( mCtlStage ) );
    }

    int  getctrl_videofd()const  { return mControlHandle.video_fd; }
    int  getdata_videofd()const   { return mDataHandle.video_fd;}
    int  getctrl_ovlyobjfd()const {return mControlHandle.overlayobj_sharedfd;}
    int  getdata_ovlyobjfd()const {return mDataHandle.overlayobj_sharedfd;}

    int  getctrl_linkvideofd()const {return mLinkVideoCtrlfd;}
    int  getdata_linkvideofd()const {return mLinkVideoDatafd;}
    void  setctrl_linkvideofd(int fd) {mLinkVideoCtrlfd = fd;}
    void  setdata_linkvideofd(int fd) {mLinkVideoDatafd = fd;}

    int  getIndex() const    { return mControlHandle.overlayobj_index; }
    int  getsize() const    { return mControlHandle.overlayobj_size; }

    overlay_ctrl_t*  data()     { return &mCtl; }
    overlay_ctrl_t*   staging()   { return &mCtlStage; }

};

// Only one instance is created per platform: create a singleton object
class overlay_control_context_t : public overlay_control_device_t {
public:
    static overlay_object* getOverlayobj(overlay_handle_t handle);

    static int overlay_get(struct overlay_control_device_t *dev, int name);
    static overlay_t* overlay_createOverlay(struct overlay_control_device_t *dev,
            uint32_t w, uint32_t h, int32_t  format);
    static overlay_t* overlay_createOverlay(struct overlay_control_device_t *dev,
            uint32_t w, uint32_t h, int32_t  format, int isS3D);
    static void overlay_destroyOverlay(struct overlay_control_device_t *dev,
                                       overlay_t* overlay);
    static int overlay_setPosition(struct overlay_control_device_t *dev,
                                   overlay_t* overlay, int x, int y, uint32_t w,
                                   uint32_t h);
    static int overlay_getPosition(struct overlay_control_device_t *dev,
                                   overlay_t* overlay, int* x, int* y, uint32_t* w,
                                   uint32_t* h);
    static int overlay_setParameter(struct overlay_control_device_t *dev,
                                    overlay_t* overlay, int param, int value);
    static int overlay_stage(struct overlay_control_device_t *dev,
                              overlay_t* overlay);
    static int overlay_commit(struct overlay_control_device_t *dev,
                              overlay_t* overlay);
    static int overlay_control_close(struct hw_device_t *dev);

    static int overlay_requestOverlayClone(struct overlay_control_device_t* dev, overlay_t* overlay,int enable);

public:
    /* this method has to index the correct overly object and map to the current process and
     * return the overlay object to the current data path.
     */
    static int  create_shared_overlayobj(overlay_object** overlayobj);
    static void destroy_shared_overlayobj(overlay_object *overlayobj, bool isCtrlpath = true);
    static overlay_object* open_shared_overlayobj(int ovlyfd, int ovlysize);
    static void close_shared_overlayobj(overlay_object *overlayobj);
    static void calculateWindow(overlay_object *overlayobj, overlay_ctrl_t *finalWindow, int panelId, bool isCrtlpath = true);
    static void calculateDisplayMetaData(overlay_object *overlayobj, int panelId);

    //methods for link device
     static int CommitLinkDevice(overlay_control_device_t *dev, overlay_object* overlayobj);

public:
    /**
     * inorder to avoid the static data in this .so we are making this array non-static
     * with the assumption that only one of this class is created.
     */
    overlay_object* mOmapOverlays[MAX_NUM_OVERLAYS];

};

// A separate instance is created per overlay data side user
class overlay_data_context_t : public  overlay_data_device_t {
public:
    static int overlay_initialize(struct overlay_data_device_t *dev,
                           overlay_handle_t handle);
    static int overlay_resizeInput(struct overlay_data_device_t *dev, uint32_t w, uint32_t h);
    static int overlay_data_setParameter(struct overlay_data_device_t *dev,
                                         int param, int value);
    static int overlay_setCrop(struct overlay_data_device_t *dev, uint32_t x,
                               uint32_t y, uint32_t w, uint32_t h);
    static int overlay_getCrop(struct overlay_data_device_t *dev , uint32_t* x,
                               uint32_t* y, uint32_t* w, uint32_t* h);
    static int overlay_dequeueBuffer(struct overlay_data_device_t *dev,
                              overlay_buffer_t *buffer);
    static int overlay_queueBuffer(struct overlay_data_device_t *dev,
                            overlay_buffer_t buffer);
    static void *overlay_getBufferAddress(struct overlay_data_device_t *dev,
                                   overlay_buffer_t buffer);
    static int overlay_getBufferCount(struct overlay_data_device_t *dev);
    static int overlay_data_close(struct hw_device_t *dev);
    static int overlay_set_s3d_params(struct overlay_data_device_t *dev, uint32_t s3d_mode,
                           uint32_t s3d_fmt, uint32_t s3d_order, uint32_t s3d_subsampling);

    /* our private state goes below here */
public:
    static int  enable_streaming(overlay_object* ovly, bool isDatapath = true);
    static int  enable_streaming_locked(overlay_object* ovly, bool isDatapath = true);
    static int  disable_streaming(overlay_object* ovly, bool isDatapath = true);
    static int  disable_streaming_locked(overlay_object* ovly, bool isDatapath = true);

    overlay_object* omap_overlay;

};

static int overlay_device_open(const struct hw_module_t* module,
                               const char* name, struct hw_device_t** device);


//struct to maintain the display panel names and paths for sysfs
struct displayPanelMetaData {
    char displayenabled[PATH_MAX];
    char displayname[PATH_MAX];
    char displaytimings[PATH_MAX];
};

//struct to maintain the display Manager names and paths for sysfs
struct displayManagerMetaData {
    char managername[PATH_MAX];
    char managerdisplay[PATH_MAX];
    char managertrans_key_value[PATH_MAX];
    char managertrans_key_type[PATH_MAX];
    char managertrans_key_enabled[PATH_MAX];
};


#endif  // TIOVERLAY_H_

