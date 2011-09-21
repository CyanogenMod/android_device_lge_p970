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
/** Author: Sunita Nadampalli < sunitan@ti.com >
*/

#include <binder/IPCThreadState.h>
#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>
#include <utils/Log.h>

#include <surfaceflinger/Surface.h>
#include <surfaceflinger/ISurface.h>
#include <surfaceflinger/ISurfaceComposer.h>
#include <surfaceflinger/SurfaceComposerClient.h>
#include <surfaceflinger/ISurfaceComposerClient.h>
#include <ui/Overlay.h>
#include "v4l2_utils.h"
#include "overlay_common.h"

#include <pthread.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <getopt.h>

using namespace android;

namespace android {

#define MIN(X, Y) ((X) < (Y) ? (X) : (Y))

#define RET_CHECK_EQ(x, y, z) {if (x != y) { LOGE("ErrScenario Test failed @%d", z); return;} }
#define RET_CHECK_NOTEQ(x, y, z) {if (x == y) { LOGE("ErrScenario Test failed @%d", z); return;} }

class Test {
public:
    void getISurface(const sp<SurfaceControl>& s) {
    mSurface =  s->getISurface();
    return;
    }

    sp<ISurface> mSurface;

};

/** overlay test class definition**/
class OverlayTest : public Test
{
public:
    sp<ProcessState> mProc;
    void* mBuffers1[NUM_OVERLAY_BUFFERS_MAX];
    void* mBuffers2[NUM_OVERLAY_BUFFERS_MAX];
    int   mNumBuffers;

    //surface flinger client data structures
    sp<SurfaceComposerClient> mClient1;
    sp<SurfaceComposerClient> mClient2;

    sp<SurfaceControl> mSurfaceCtrl1;
    sp<SurfaceControl> mSurfaceCtrl2;

    sp<Overlay> mOverlay1;
    sp<Overlay> mOverlay2;

    sp<ISurface> mSurface1;
    sp<ISurface> mSurface2;

    float alpha1;
    float alpha2;

    int deg1;
    int deg2;

    int mirror1;
    int mirror2;

    int newwidth1;
    int newheight1;
    int newwidth2;
    int newheight2;

    int win_x;
    int win_y;

    int zOrder1;
    int zOrder2;

    bool zoom1;
    bool zoom2;
    bool autoscale1;
    bool autoscale2;
    bool resize;

public:
    int init();
    void deinit();
    void resetParams()
    {
    alpha1 = 1.0;
    alpha2 = 1.0;

    deg1 = -1;
    deg2 = -1;

    mirror1 = 0;
    mirror2 = 0;

    newwidth1 = -1;
    newheight1 = -1;
    newwidth2 = -1;
    newheight2 = -1;

    win_x = 100;
    win_y = 100;

    zOrder1 = -1;
    zOrder2 = -1;

    zoom1 = false;
    zoom2 = false;

    autoscale1 = false;
    autoscale2 = false;
    resize = false;
    }
    void testOverlay(char* img1, uint32_t w1, uint32_t h1, uint8_t fmt1, uint8_t panel1, char* img2=NULL, uint32_t w2=0, uint32_t h2=0, uint8_t fmt2 = 0, uint8_t panel2=0);
   void testErrorScenarios();
};


//Single client + one Overlay
void OverlayTest::testOverlay(char* img1, uint32_t w1, uint32_t h1, uint8_t fmt1, uint8_t panel1,
                            char* img2, uint32_t w2, uint32_t h2, uint8_t fmt2, uint8_t panel2)
{
    LOGE("testOverlay++");
#ifdef TARGET_OMAP4
    if ((w1 > FULLHD_WIDTH )||(w2 > FULLHD_WIDTH)||(h1 > FULLHD_HEIGHT)||(h2 > FULLHD_HEIGHT))
#else
    if (w1 > MAX_OVERLAY_WIDTH_VAL ||
        h1 > MAX_OVERLAY_HEIGHT_VAL ||
        w2 > MAX_OVERLAY_WIDTH_VAL ||
        h2 > MAX_OVERLAY_HEIGHT_VAL ||
        w1*h1 > MAX_OVERLAY_RESOLUTION ||
        w2*h2 > MAX_OVERLAY_RESOLUTION)
#endif
        {
        LOGE("Invalid Image resolution");
        return;
        }

    int bytesPerPixel1 = 2;
    int bytesPerPixel2 = 2;

    // now request an overlay
    sp<ISurface> firstSurface = mSurface1;
    sp<SurfaceComposerClient> firstClient = mClient1;
    sp<SurfaceControl> firstSurfaceCtrl = mSurfaceCtrl1;
    sp<OverlayRef> ref1;

    sp<ISurface> secondSurface = mSurface2;
    sp<SurfaceComposerClient> secondClient = mClient2;
    sp<SurfaceControl> secondSurfaceCtrl = mSurfaceCtrl2;
    sp<OverlayRef> ref2;

    int format;
    switch(fmt1)
    {
    case 0:
        format = OVERLAY_FORMAT_RGB_565;
        bytesPerPixel1 = 2;
        break;
    case 1:
        format = OVERLAY_FORMAT_RGBA_8888;
        bytesPerPixel1 = 4;
        break;
    case 2:
        format = OVERLAY_FORMAT_BGRA_8888;
        bytesPerPixel1 = 4;
        break;
    case 3:
        format = OVERLAY_FORMAT_YCbCr_422_SP;
        bytesPerPixel1 = 1; //this is not the bytes per complete pixel, instead only Y-component
        break;
    case 4:
        format = OVERLAY_FORMAT_YCbCr_420_SP;
        bytesPerPixel1 = 1; //this is not the bytes per complete pixel, instead only Y-component
        break;
    case 5:
        format = OVERLAY_FORMAT_YCbYCr_422_I;
        bytesPerPixel1 = 2;
        break;
    case 6:
        format = OVERLAY_FORMAT_YCbYCr_420_I;
        bytesPerPixel1 = 1; //this is not the bytes per complete pixel, instead only Y-component
        break;
    case 7:
        format = OVERLAY_FORMAT_CbYCrY_422_I;
        bytesPerPixel1 = 2;
        break;
    case 8:
        format = OVERLAY_FORMAT_CbYCrY_420_I;
        bytesPerPixel1 = 1; //this is not the bytes per complete pixel, instead only Y-component
        break;
    default:
        format = fmt1;
        break;
    };


    ref1 = firstSurface->createOverlay(w1, h1, format, mirror1);
    if(ref1 == NULL)
    {
        LOGE("NoMemory for overlayRef[%d]", __LINE__);
        return;
    }
    mOverlay1 = new Overlay(ref1);
    if(mOverlay1 == NULL)
    {
        LOGE("NoMemory for overlay[%d]", __LINE__);
        return;
    }

    if (resize) {
    int ret = mOverlay1->setParameter(OVERLAY_NUM_BUFFERS, -11);
    if (ret != 0) {
        LOGE("Yes, Overlay ERROR scenario is tested");
    }
    else {
        LOGE("Failed:Overlay error scenario test");
        return;
    }
    mOverlay1->resizeInput(w1/2, h1/2);

    mOverlay1->setParameter(OVERLAY_NUM_BUFFERS, 40);
    mOverlay1->resizeInput(w1, h1);
    }

    mOverlay1->setParameter(OVERLAY_NUM_BUFFERS, 10);
    mOverlay1->resizeInput(w1, h1);

    if (img2 != NULL)
    {
    int format;
    switch(fmt2)
    {
    case 0:
        format = OVERLAY_FORMAT_RGB_565;
        bytesPerPixel2 = 2;
        break;
    case 1:
        format = OVERLAY_FORMAT_RGBA_8888;
        bytesPerPixel2 = 4;
        break;
    case 2:
        format = OVERLAY_FORMAT_BGRA_8888;
        bytesPerPixel2 = 4;
        break;
    case 3:
        format = OVERLAY_FORMAT_YCbCr_422_SP;
        bytesPerPixel2 = 1; //this is not the bytes per complete pixel, instead only Y-component
        break;
    case 4:
        format = OVERLAY_FORMAT_YCbCr_420_SP;
        bytesPerPixel2 = 1; //this is not the bytes per complete pixel, instead only Y-component
        break;
    case 5:
        format = OVERLAY_FORMAT_YCbYCr_422_I;
        bytesPerPixel2 = 2;
        break;
    case 6:
        format = OVERLAY_FORMAT_YCbYCr_420_I;
        bytesPerPixel2 = 1; //this is not the bytes per complete pixel, instead only Y-component
        break;
    case 7:
        format = OVERLAY_FORMAT_CbYCrY_422_I;
        bytesPerPixel2 = 2;
        break;
    case 8:
        format = OVERLAY_FORMAT_CbYCrY_420_I;
        bytesPerPixel2 = 1; //this is not the bytes per complete pixel, instead only Y-component
        break;
    default:
        format = fmt2;
        break;
    };
    ref2 = secondSurface->createOverlay(w2, h2, format, mirror2);
    if(ref2 == NULL)
    {
        LOGE("NoMemory for overlayRef[%d]", __LINE__);
        return;
    }
    mOverlay2 = new Overlay(ref2);
    if(mOverlay2 == NULL)
    {
        LOGE("NoMemory for overlay[%d]", __LINE__);
        return;
    }
    mOverlay2->setParameter(OVERLAY_NUM_BUFFERS, 10);
    mOverlay2->resizeInput(w2, h2);
    }

    firstClient->openTransaction();
    firstSurfaceCtrl->setPosition(win_x, win_y);
    if ((newwidth1 != -1) && (newheight1 != -1))
    {
        firstSurfaceCtrl->setSize(newwidth1, newheight1);
    }
    else
    {
        firstSurfaceCtrl->setSize(w1, h1);
    }

    if (zOrder1 != -1)
    {
        firstSurfaceCtrl->setLayer(zOrder1);
    }
    if (deg1 != -1)
    {
    switch(deg1)
        {
        case 0:
            firstClient->setOrientation(panel1,ISurfaceComposer::eOrientationDefault,0);
            break;
        case 90:
            firstClient->setOrientation(panel1,ISurfaceComposer::eOrientation90,0);
            break;
        case 180:
            firstClient->setOrientation(panel1,ISurfaceComposer::eOrientation180,0);
            break;
        case 270:
            firstClient->setOrientation(panel1,ISurfaceComposer::eOrientation270,0);
            break;
        default:
            firstClient->setOrientation(panel1,ISurfaceComposer::eOrientationDefault,0);
        };
    }
    firstSurfaceCtrl->setAlpha(alpha1);
    firstClient->closeTransaction();

    //Set Display ID on ISurface directly
    firstSurface->setDisplayId(panel1);

    if (img2 != NULL)
    {
        secondClient->openTransaction();
        secondSurfaceCtrl->setPosition(320, 0);
        if ((newwidth2 != -1) && (newheight2 != -1))
        {
            secondSurfaceCtrl->setSize(newwidth2, newheight2);
        }
        else
        {
            secondSurfaceCtrl->setSize(w2, h2);
        }
        if (zOrder2 != -1)
        {
            secondSurfaceCtrl->setLayer(zOrder2);
        }
        if (deg2 != -1)
        {
        switch(deg2)
            {
            case 0:
                secondClient->setOrientation(panel2,ISurfaceComposer::eOrientationDefault,0);
                break;
            case 90:
                secondClient->setOrientation(panel2,ISurfaceComposer::eOrientation90,0);
                break;
            case 180:
                secondClient->setOrientation(panel2,ISurfaceComposer::eOrientation180,0);
                break;
            case 270:
                secondClient->setOrientation(panel2,ISurfaceComposer::eOrientation270,0);
                break;
            default:
                secondClient->setOrientation(panel2,ISurfaceComposer::eOrientationDefault,0);
            };
        }
    secondSurfaceCtrl->setAlpha(alpha2);
    secondClient->closeTransaction();

    //Set Display ID on ISurface directly
    secondSurface->setDisplayId(panel2);

    }

    int filedes1 = -1;
    long int file1size = 0;
    mapping_data_t* data1;
    int cnt1 = 0, iter = 0;
    overlay_buffer_t buffer1;
    int numBuffers1;
    uint32_t framesize1 = w1*h1*bytesPerPixel1;
    if (fmt1 == 4) {
        framesize1 = (w1*h1*3)/2;   //for NV-12 it is 12 bits per pixel
    }
    uint8_t* localbuffer1 = new uint8_t [framesize1];

    int filedes2 = -1;
    long int file2size = 0;
    mapping_data_t* data2;
    int cnt2 = 0;
    overlay_buffer_t buffer2;
    int numBuffers2;
    uint32_t framesize2 = w2*h2*bytesPerPixel2;
    if (fmt2 == 4) {
        framesize2 = (w2*h2*3)/2;   //for NV-12 format, it is 12 bits per pixel
    }
    uint8_t* localbuffer2 = new uint8_t [framesize2];

    filedes1 = open(img1, O_RDONLY);
    if ( filedes1 < 0 ) {
        LOGE("\n\n\n\nError: failed to open the file %s for reading\n\n\n\n", img1);
        delete []localbuffer1;
        localbuffer1 = NULL;
        delete []localbuffer2;
        localbuffer2 = NULL;
        return;
    }
    struct stat stat;
    fstat(filedes1, &stat);
    file1size = stat.st_size;
    LOGE("File1 size = %ld", file1size);
    numBuffers1 = mOverlay1->getBufferCount();

    for (int i1 = 0; i1 < numBuffers1; i1++) {
        data1 = (mapping_data_t *)mOverlay1->getBufferAddress((void*)i1);
        if (data1 == NULL)
        {
            LOGE("No Buffer from Overlay [%x]", (unsigned int)data1);
        delete []localbuffer1;
        localbuffer1 = NULL;
        delete []localbuffer2;
        localbuffer2 = NULL;
        close(filedes1);
        return;
        }
        mBuffers1[i1] = data1->ptr;
    }

    if (img2 != NULL)
    {
        filedes2 = open(img2, O_RDONLY);
        if ( filedes2 < 0 ) {
        LOGE("\nError: failed to open the file %s for reading\n", img2);
        delete []localbuffer1;
        localbuffer1 = NULL;
        delete []localbuffer2;
        localbuffer2 = NULL;
        close(filedes1);
        close(filedes2);
        return;
    }
    fstat(filedes2, &stat);
    file2size = stat.st_size;
    LOGE("File2 size = %ld", file2size);
    numBuffers2 = mOverlay2->getBufferCount();
    for (int i2 = 0; i2 < numBuffers2; i2++) {
        data2 = (mapping_data_t *)mOverlay2->getBufferAddress((void*)i2);
        if (data2 == NULL)
        {
            LOGE("No Buffer from Overlay [%x]", (unsigned int)data2);
            delete []localbuffer1;
            localbuffer1 = NULL;
            delete []localbuffer2;
            localbuffer2 = NULL;
            close(filedes1);
            close(filedes2);
            return;
        }
        mBuffers2[i2] = data2->ptr;
        }
    }

    int x, y, w, h, cropw = w1/2, croph = h1/2;
    int totalIter = file1size/(framesize1*numBuffers1);
    static int cropx = 0;
    static int cropy = 0;

    if (zoom1) {
        mOverlay1->setCrop(cropx, cropy, cropw, croph);
    }

    for (int i3 = 0; i3 < NUM_BUFFERS_TO_BE_QUEUED_FOR_OPTIMAL_PERFORMANCE; i3++)
    {
        // copy the file into the buffers:
        //we can use read: provided we have the file des
        int ret1 = read (filedes1, (void*)localbuffer1, framesize1);
        if (ret1 != (int)framesize1)
        {
            LOGE ("\nReading error file1.[%d]\n", errno);
            delete []localbuffer1;
            localbuffer1 = NULL;
            delete []localbuffer2;
            localbuffer2 = NULL;
            close(filedes1);
            close(filedes2);
            return;
        }
        file1size -= framesize1;

        uint8_t *p1 = (uint8_t*) mBuffers1[cnt1];
        uint8_t* runningptr1 = localbuffer1;

        #ifdef TARGET_OMAP4
        int total_height_bytes = h1;
        int total_width_bytes = bytesPerPixel1*w1;
        if (fmt1 == 4) {
            total_height_bytes = (3*h1)/2;
            }
            uint32_t stride = 4096;
            //copy y-buffer, almost bytewise copy, except for stride jumps.
            uint8_t* p1y = (uint8_t*) runningptr1;
            uint8_t* p2y = (uint8_t*) p1;
            for(int i=0;i<total_height_bytes;i++)
                {
                //copy whole row of Y pixels. source and desination will point to new row each time.
                memcpy(p2y+i*stride, p1y+i*total_width_bytes,total_width_bytes);
                }
        #else
            memcpy(mBuffers1[cnt1], runningptr1, framesize1);
        #endif
        mOverlay1->queueBuffer((void*)(cnt1++));

        if (img2 != NULL)
        {
            //we can use pread: provided we have the file des
            int ret2 = read (filedes2, (void*)localbuffer2, framesize2);
            if (ret2 != (int)framesize2)
            {
                LOGE ("\nReading error file1.[%d]\n", errno);
                delete []localbuffer1;
                localbuffer1 = NULL;
                delete []localbuffer2;
                localbuffer2 = NULL;
                close(filedes1);
                close(filedes2);
                return;
            }
            file2size -= framesize2;
            uint8_t *p2 = (uint8_t*) mBuffers2[cnt2];
            uint8_t* runningptr2 = localbuffer2;
            #ifdef TARGET_OMAP4
        int total_height_bytes = h2;
        int total_width_bytes = bytesPerPixel2*w2;
        if (fmt2 == 4) {
            total_height_bytes = (3*h2)/2;
            }
            uint32_t stride = 4096;
            //copy y-buffer, almost bytewise copy, except for stride jumps.
            uint8_t* p1y = (uint8_t*) runningptr2;
            uint8_t* p2y = (uint8_t*) p2;
            for(int i=0;i<total_height_bytes;i++)
                {
                //copy whole row of Y pixels. source and desination will point to new row each time.
                memcpy(p2y+i*stride, p1y+i*total_width_bytes,total_width_bytes);
                }
            #else
                memcpy(mBuffers2[cnt2], runningptr2, framesize2);
            #endif
            mOverlay2->queueBuffer((void*)(cnt2++));
        }
    }

    do{
    if (file1size > 0)
    {
        int ret1 = read (filedes1, (void*)localbuffer1, framesize1);
        if (ret1 != (int)framesize1)
        {
            LOGE ("\nReading error file or File End.[%d]\n", errno);
            break;
        }
        file1size -= framesize1;
        uint8_t *p1 = (uint8_t*) mBuffers1[cnt1];
        uint8_t* runningptr1 = localbuffer1;
        #ifdef TARGET_OMAP4
        int total_height_bytes = h1;
        int total_width_bytes = bytesPerPixel1*w1;
if (fmt1 == 4) {
    total_height_bytes = (3*h1)/2;
    }
            uint32_t stride = 4096;
            //copy y-buffer, almost bytewise copy, except for stride jumps.
            uint8_t* p1y = (uint8_t*) runningptr1;
            uint8_t* p2y = (uint8_t*) p1;
            for(int i=0;i<total_height_bytes;i++)
                {
                //copy whole row of Y pixels. source and desination will point to new row each time.
                memcpy(p2y+i*stride, p1y+i*total_width_bytes,total_width_bytes);
                }
        #else
            memcpy(mBuffers1[cnt1], runningptr1, framesize1);
        #endif

    //start queueing the buffers
    mOverlay1->queueBuffer((void*)(cnt1++));

    //now deque the buffers
    mOverlay1->dequeueBuffer(&buffer1);
    }
    if (img2 != NULL)
    {
        if (file2size > 0)
        {
            int ret2 = read (filedes2, (void*)localbuffer2, framesize2);
            if (ret2 != (int)framesize2)
            {
                LOGE ("\nReading error file2.[%d]\n", errno);
                break;
            }
            file2size -= framesize2;
            uint8_t *p2 = (uint8_t*) mBuffers2[cnt2];
            uint8_t* runningptr2 = localbuffer2;
            #ifdef TARGET_OMAP4
        int total_height_bytes = h2;
        int total_width_bytes = bytesPerPixel2*w2;
        if (fmt2 == 4) {
            total_height_bytes = (3*h2)/2;
            }
            uint32_t stride = 4096;
            //copy y-buffer, almost bytewise copy, except for stride jumps.
            uint8_t* p1y = (uint8_t*) runningptr2;
            uint8_t* p2y = (uint8_t*) p2;
            for(int i=0;i<total_height_bytes;i++)
                {
                //copy whole row of Y pixels. source and desination will point to new row each time.
                memcpy(p2y+i*stride, p1y+i*total_width_bytes,total_width_bytes);
                }
            #else
                memcpy(mBuffers2[cnt2], runningptr2, framesize2);
            #endif

            //start queueing the buffers
            mOverlay2->queueBuffer((void*)(cnt2++));
            //now deque the buffers
            mOverlay2->dequeueBuffer(&buffer2);
        }
    }

    if (cnt1 > (numBuffers1-1))
    {
        iter++;
        cnt1 = 0;
        cnt2 = 0;

        if (zoom1)
        {
            /**on-the-fly setcrop supports only the co-ordinates chnage, but not
            * the window size changes
            **/
            cropx = cropx+5;
            cropy = cropy+5;
            if (cropx > cropw/5) {
                cropx = 0;
            }
            if (cropy > croph/5) {
                cropy = 0;
            }
            mOverlay1->setCrop(cropx, cropy,cropw, croph);
            firstClient->openTransaction();
                firstSurfaceCtrl->setPosition(win_x, win_y);
            firstSurfaceCtrl->setSize(w1, h1);
            firstClient->closeTransaction();
        }
        if (autoscale1)
        {
            if (iter < 100)
            {
                firstClient->openTransaction();
                firstSurfaceCtrl->setPosition(30+iter*8, 30+iter*4);
                firstSurfaceCtrl->setSize(w1 -(w1*iter)/100, h1-(h1*iter)/100);
                firstClient->closeTransaction();
            }
            else if (iter < 200)
            {
                firstClient->openTransaction();
                firstSurfaceCtrl->setPosition(30+(iter-100)*8, 30+(iter-100)*4);
                firstSurfaceCtrl->setSize(w1-(w1*(iter-100))/(100), h1-(h1*(iter-100))/(100));
                firstClient->closeTransaction();
            }
            else if (iter < 300)
            {
                firstClient->openTransaction();
                firstSurfaceCtrl->setPosition(30+(iter-200)*8, 30+(iter-200)*4);
                firstSurfaceCtrl->setSize(w1-(w1*(iter-200))/(100), h1-(h1*(iter-200))/(100));
                firstClient->closeTransaction();
            }
            else if (iter == 300)
            {
                firstClient->openTransaction();
                firstSurfaceCtrl->setPosition(30, 30);
                firstSurfaceCtrl->setSize(w1, h1);
                firstClient->closeTransaction();
            }
        }

        if (zoom2)
        {
            bool changed = false;
            if (iter == 1)
            {
                cropx = 30;
                cropy = 30;
                cropw = 100;
                croph = 100;
                changed = true;
            }
            else if (iter == totalIter/4)
            {
                cropx = 100;
                cropy = 0;
                cropw = 100;
                croph = 100;
                changed = true;
            }
            else if (iter == totalIter/2)
            {
                cropx = 0;
                cropy = 100;
                cropw = 100;
                croph = 100;
                changed = true;
            }
            else if (iter == 3*totalIter/4)
            {
                cropx = 80;
                cropy = 80;
                cropw = 100;
                croph = 100;
                changed = true;
            }
            if (changed)
            {
                mOverlay2->setCrop(cropx, cropy,cropw, croph);
                secondClient->openTransaction();
                secondSurfaceCtrl->setPosition(50, 50);
                secondSurfaceCtrl->setSize(w2, h2);
                secondClient->closeTransaction();
                changed = false;
            }
        }

        if (autoscale2)
        {
            if (iter < 100)
            {
                secondClient->openTransaction();
                secondSurfaceCtrl->setPosition(30+iter*8, 30+iter*4);
                secondSurfaceCtrl->setSize(w2 -(w2*iter)/100, h2-(h2*iter)/100);
                secondClient->closeTransaction();
            }
            else if (iter < 200)
            {
                secondClient->openTransaction();
                secondSurfaceCtrl->setPosition(30+(iter-100)*8, 30+(iter-100)*4);
                secondSurfaceCtrl->setSize(w2-(w2*(iter-100))/(100), h2-(h2*(iter-100))/(100));
                secondClient->closeTransaction();
            }
            else if (iter < 300)
            {
                secondClient->openTransaction();
                secondSurfaceCtrl->setPosition(30+(iter-200)*8, 30+(iter-200)*4);
                secondSurfaceCtrl->setSize(w2-(w2*(iter-200))/(100), h2-(h2*(iter-200))/(100));
                secondClient->closeTransaction();
            }
            else if (iter == 300)
            {
                secondClient->openTransaction();
                secondSurfaceCtrl->setPosition(30, 30);
                secondSurfaceCtrl->setSize(w2, h2);
                secondClient->closeTransaction();
            }
        }
    }
    }while((file1size > 0) || (file2size > 0));

    //resetting Rotation
    if (deg1 > 0){
        firstClient->openTransaction();
        firstClient->setOrientation(panel1,ISurfaceComposer::eOrientationDefault,0);
        firstClient->closeTransaction();
    }

    printf("\n\nPlayback Done\n\n");
    //resetParams();
    deinit();

    if (localbuffer1 != NULL)
    {
        delete []localbuffer1;
        localbuffer1 = NULL;
    }
    if (localbuffer2 != NULL)
    {
        delete []localbuffer2;
        localbuffer2 = NULL;
    }

    close(filedes1);
    close(filedes2);
    LOGD("testOverlay--");
}

/** menthod to test all possible error scenarios from overlay library
*/
void OverlayTest::testErrorScenarios()
{
    /** the error scenarios to be tested
    *<1>Passing NULL arguments
    *<2>passing Invalid argumnets
    *<3>requesting for que buffers beyond max
    *<4>creating overlay with invalid size
    *<5>requesting for invalid overlay bufffers
    *<6>requesting for Invalid optimal buffer for queuing
    *<7>request for invalid formats
    *<8>deque without queuing the buffers first
    *<9>corrupt handle and create overlay
    *<10>setting up for invalid window size
    *<11>request for invalid crop size
    *<12>queue buffers beyond the max limit
    *<13>create overlays beyond max limit
    *<14>call apis with NULL device
    *<15>call resizeInput multiple times
    */

    sp<ISurface> firstSurface = mSurface1;
    sp<SurfaceComposerClient> firstClient = mClient1;
    sp<SurfaceControl> firstSurfaceCtrl = mSurfaceCtrl1;
    sp<OverlayRef> ref1;

    /** create overlay with invalid sizes*/
    //Invalid width
    ref1 = firstSurface->createOverlay(MAX_OVERLAY_WIDTH_VAL*2, MAX_OVERLAY_HEIGHT_VAL, OVERLAY_FORMAT_RGB_565, 0);
    RET_CHECK_EQ(ref1, NULL, __LINE__);
    //Invalid height
    ref1 = firstSurface->createOverlay(MAX_OVERLAY_WIDTH_VAL, MAX_OVERLAY_HEIGHT_VAL*2, OVERLAY_FORMAT_RGB_565, 0);
    RET_CHECK_EQ(ref1, NULL, __LINE__);
    //Invalid width x height
    ref1 = firstSurface->createOverlay(MAX_OVERLAY_WIDTH_VAL+1, MAX_OVERLAY_HEIGHT_VAL+1, OVERLAY_FORMAT_RGB_565, 0);
    RET_CHECK_EQ(ref1, NULL, __LINE__);
    //ZERO size
    ref1 = firstSurface->createOverlay(0, MAX_OVERLAY_HEIGHT_VAL, OVERLAY_FORMAT_DEFAULT, 0);
    RET_CHECK_EQ(ref1, NULL, __LINE__);
    //negative size
    ref1 = firstSurface->createOverlay(MAX_OVERLAY_WIDTH_VAL, -1, OVERLAY_FORMAT_DEFAULT, 0);
    RET_CHECK_EQ(ref1, NULL, __LINE__);
    //Invalid format
    ref1 = firstSurface->createOverlay(MAX_OVERLAY_WIDTH_VAL/2, MAX_OVERLAY_HEIGHT_VAL/2, OVERLAY_FORMAT_DEFAULT+1, 0);
    RET_CHECK_EQ(ref1, NULL, __LINE__);
    //currently unsupported format
    ref1 = firstSurface->createOverlay(MAX_OVERLAY_WIDTH_VAL/2, MAX_OVERLAY_HEIGHT_VAL/2, OVERLAY_FORMAT_BGRA_8888, 0);
    RET_CHECK_EQ(ref1, NULL, __LINE__);


    //correct format & size
    ref1 = firstSurface->createOverlay(MAX_OVERLAY_WIDTH_VAL/2, MAX_OVERLAY_HEIGHT_VAL/2, OVERLAY_FORMAT_RGB_565, 0);
    RET_CHECK_NOTEQ(ref1, NULL, __LINE__);

    mOverlay1 = new Overlay(ref1);
    RET_CHECK_NOTEQ(mOverlay1, NULL, __LINE__);

    int ret = mOverlay1->setParameter(OVERLAY_NUM_BUFFERS, -11);
    RET_CHECK_NOTEQ(ret, 0 , __LINE__);

    //Same res as createOverlay. So, no change should occur.
    ret = mOverlay1->resizeInput(MAX_OVERLAY_WIDTH_VAL/2, MAX_OVERLAY_HEIGHT_VAL/2);
    RET_CHECK_EQ(ret, 0 , __LINE__);

    ret = mOverlay1->resizeInput(LCD_WIDTH, LCD_HEIGHT);
    RET_CHECK_EQ(ret, 0 , __LINE__);

    ret = mOverlay1->setParameter(OVERLAY_NUM_BUFFERS, 0);
    RET_CHECK_NOTEQ(ret, 0, __LINE__);

    ret = mOverlay1->setParameter(OVERLAY_NUM_BUFFERS, 100);
    RET_CHECK_EQ(ret, 0 , __LINE__);

    ret = mOverlay1->getBufferCount();
    RET_CHECK_EQ(ret, NUM_OVERLAY_BUFFERS_MAX, __LINE__);

    ret = mOverlay1->setParameter(OVERLAY_NUM_BUFFERS, 10);
    RET_CHECK_EQ(ret, 0 , __LINE__);

    ret = mOverlay1->getBufferCount();
    int buffercnt = (ret < NUM_OVERLAY_BUFFERS_MAX)? ret : NUM_OVERLAY_BUFFERS_MAX;
    RET_CHECK_EQ(ret, buffercnt, __LINE__);

    ret = mOverlay1->resizeInput(200, 200);
    RET_CHECK_EQ(ret, 0, __LINE__);

    ret = mOverlay1->resizeInput(10, 0);
    RET_CHECK_NOTEQ(ret, 0, __LINE__);

    ret = mOverlay1->resizeInput(MAX_OVERLAY_WIDTH_VAL*2, 10);
    RET_CHECK_NOTEQ(ret, 0, __LINE__);

    ret = mOverlay1->resizeInput(MAX_OVERLAY_WIDTH_VAL, MAX_OVERLAY_HEIGHT_VAL*2);
    RET_CHECK_NOTEQ(ret, 0, __LINE__);

    ret = mOverlay1->resizeInput(MAX_OVERLAY_WIDTH_VAL, MAX_OVERLAY_HEIGHT_VAL);
    RET_CHECK_NOTEQ(ret, 0, __LINE__);

    ret = mOverlay1->resizeInput(0, MAX_OVERLAY_HEIGHT_VAL);
    RET_CHECK_NOTEQ(ret, 0, __LINE__);

    ret = mOverlay1->resizeInput(MAX_OVERLAY_WIDTH_VAL, -1);
    RET_CHECK_NOTEQ(ret, 0, __LINE__);

    /** finally overlay is created and inialized after passing all err tests !!!
    */
    //now start setting Invalid parameters
    //since there are indirectly configured to overlay by Surface Flinger
    // we cant check for return values here, but system should not crash or something like that
    // overlay library should return error values: observe the prints
    firstClient->openTransaction();
    firstSurfaceCtrl->setPosition(-100, -200);
    firstSurfaceCtrl->setSize(0, 0);
    firstClient->closeTransaction();

    firstClient->openTransaction();
    firstSurfaceCtrl->setPosition(MAX_OVERLAY_WIDTH_VAL*2, MAX_OVERLAY_WIDTH_VAL*2);
    firstSurfaceCtrl->setSize(0, 0);
    firstClient->closeTransaction();

    firstClient->openTransaction();
    firstSurfaceCtrl->setLayer(1000);
    firstClient->closeTransaction();

    firstClient->openTransaction();
    firstSurfaceCtrl->setLayer(-1000);
    firstClient->closeTransaction();

    firstClient->openTransaction();
    firstSurfaceCtrl->setLayer(0);
    firstClient->closeTransaction();

    firstClient->openTransaction();
    firstClient->setOrientation(0,1000,0);
    firstClient->closeTransaction();

    firstClient->openTransaction();
    firstClient->setOrientation(0,ISurfaceComposer::eOrientationDefault,0);
    firstClient->closeTransaction();

    firstClient->openTransaction();
    firstSurfaceCtrl->setAlpha(0);
    firstClient->closeTransaction();


    firstClient->openTransaction();
    firstSurfaceCtrl->setAlpha(-1000);
    firstClient->closeTransaction();

    firstClient->openTransaction();
    firstSurfaceCtrl->setAlpha(1000);
    firstClient->closeTransaction();

    //Tested control parameter configuration; Now lets test buffer q/dq apis
    //data setParameter apis
    ret = mOverlay1->setParameter(OPTIMAL_QBUF_CNT, -100);
    RET_CHECK_NOTEQ(ret, 0, __LINE__);

    ret = mOverlay1->setParameter(OPTIMAL_QBUF_CNT, NUM_OVERLAY_BUFFERS_REQUESTED);
    RET_CHECK_EQ(ret, 0, __LINE__);

    ret = mOverlay1->setParameter(OPTIMAL_QBUF_CNT, NUM_OVERLAY_BUFFERS_REQUESTED*2);
    RET_CHECK_EQ(ret, 0, __LINE__);

    //test crop api for errors
    ret = mOverlay1->setCrop(0,0, 100,100);
    RET_CHECK_EQ(ret, 0, __LINE__);

    ret = mOverlay1->setCrop(0,0, 0,0);
    RET_CHECK_NOTEQ(ret, 0, __LINE__);

    ret = mOverlay1->setCrop(-1,-100, MAX_OVERLAY_WIDTH_VAL,MAX_OVERLAY_HEIGHT_VAL);
    RET_CHECK_NOTEQ(ret, 0, __LINE__);

    ret = mOverlay1->setCrop(10,10, MAX_OVERLAY_WIDTH_VAL*2,MAX_OVERLAY_HEIGHT_VAL*2);
    RET_CHECK_NOTEQ(ret, 0, __LINE__);

    //Call Sequence: resizeInput can be called after setCrop(), but only
    //buffer flush should happen in this case.
    ret = mOverlay1->resizeInput(200, 200);
    RET_CHECK_EQ(ret, 0, __LINE__);

    //get buffer addresses beyond the max buffer cnt
    int numBuffers1 = mOverlay1->getBufferCount();
    mapping_data_t* data1;
    void* mBuffers1[NUM_OVERLAY_BUFFERS_MAX*2];

    for (int i1 = 0; i1 < numBuffers1*2; i1++) {
        data1 = (mapping_data_t *)mOverlay1->getBufferAddress((void*)i1);
        if (i1 < numBuffers1) {
            RET_CHECK_NOTEQ(data1, NULL, __LINE__);
        }
        else {
            RET_CHECK_EQ(data1, NULL, __LINE__);
        }
        mBuffers1[i1] = data1;
    }
    overlay_buffer_t buffer1;
    //start dequeing buffering, even before queing
    ret = mOverlay1->dequeueBuffer(&buffer1);
    RET_CHECK_NOTEQ(ret, 0, __LINE__);

    for (int k = 0; k < numBuffers1*2; k++) {
        ret = mOverlay1->queueBuffer((void*)k);
        if (k < numBuffers1) {
            RET_CHECK_EQ(ret, k+1, __LINE__);
        }
        else {
            RET_CHECK_NOTEQ(ret, k+1, __LINE__);
        }
    }

    //Call Sequence: resizeInput can be called after queueBuffer(), but only
    //buffer flush should happen in this case.
    ret = mOverlay1->resizeInput(200, 200);
    RET_CHECK_EQ(ret, 0, __LINE__);

    //set invalid screen ID
    // create pushbuffer surface for Invalid screen and virtual sink
    //surface would be created normally, but for the default screen:Primary LCD
    mSurfaceCtrl2 = (mClient2->createSurface(getpid(), OVERLAY_ON_VIRTUAL_SINK, LCD_WIDTH, LCD_HEIGHT,
    PIXEL_FORMAT_UNKNOWN, ISurfaceComposer::ePushBuffers | ISurfaceComposer::eFXSurfaceNormal));
    RET_CHECK_NOTEQ(mSurfaceCtrl2, NULL, __LINE__);

    // create pushbuffer surface for Invalid screen and virtual sink
    mSurfaceCtrl2 = (mClient2->createSurface(getpid(), OVERLAY_ON_VIRTUAL_SINK+1, LCD_WIDTH, LCD_HEIGHT,
    PIXEL_FORMAT_UNKNOWN, ISurfaceComposer::ePushBuffers | ISurfaceComposer::eFXSurfaceNormal));
    RET_CHECK_NOTEQ(mSurfaceCtrl2, NULL, __LINE__);

    sp<OverlayRef> ref2;
    sp<OverlayRef> ref3;
    sp<OverlayRef> ref4;

    //correct format & size: 2nd overlay
    ref2 = mSurface2->createOverlay(MAX_OVERLAY_WIDTH_VAL/2, MAX_OVERLAY_HEIGHT_VAL/2, OVERLAY_FORMAT_RGB_565, 0);
    RET_CHECK_NOTEQ(ref2, NULL, __LINE__);

    //create two  more surfaces for error scenario testing
    sp<SurfaceControl> mSurfaceCtrl3;
    sp<SurfaceControl> mSurfaceCtrl4;

    sp<ISurface> mSurface3;
    sp<ISurface> mSurface4;

    // create pushbuffer surface
    mSurfaceCtrl3 = (mClient1->createSurface(getpid(), OVERLAY_ON_PRIMARY, LCD_WIDTH, LCD_HEIGHT,
                        PIXEL_FORMAT_UNKNOWN, ISurfaceComposer::ePushBuffers | ISurfaceComposer::eFXSurfaceNormal));
    if (mSurfaceCtrl3 == NULL)
    {
        LOGE("No Memory[%d]", __LINE__);
        return;
    }

    // get to the isurface
    Test::getISurface(mSurfaceCtrl3);
    mSurface3 = mSurface;
    if (mSurface3 == NULL)
    {
        LOGE("No Memory[%d]", __LINE__);
        return;
    }

    // create pushbuffer surface
    mSurfaceCtrl4 = (mClient2->createSurface(getpid(), OVERLAY_ON_PRIMARY, LCD_WIDTH, LCD_HEIGHT,
                        PIXEL_FORMAT_UNKNOWN, ISurfaceComposer::ePushBuffers | ISurfaceComposer::eFXSurfaceNormal));
    if (mSurfaceCtrl4 == NULL)
    {
        LOGE("No Memory[%d]", __LINE__);
        return;
    }

    // get to the isurface
    Test::getISurface(mSurfaceCtrl4);
    mSurface4 = mSurface;
    if (mSurface4 == NULL)
    {
        LOGE("No Memory[%d]", __LINE__);
        return;
    }

    //correct format & size: 3rd Overlay
    ref3 = mSurface3->createOverlay(MAX_OVERLAY_WIDTH_VAL/2, MAX_OVERLAY_HEIGHT_VAL/2, OVERLAY_FORMAT_RGB_565, 0);
    RET_CHECK_EQ(ref3, NULL, __LINE__);
    //correct format & size 4th Overlay
    ref4 = mSurface4->createOverlay(MAX_OVERLAY_WIDTH_VAL/2, MAX_OVERLAY_HEIGHT_VAL/2, OVERLAY_FORMAT_RGB_565, 0);
    RET_CHECK_EQ(ref4, NULL, __LINE__);

    if ( NULL != mSurface3.get() ) {
        mSurface3.clear();
    }

    if ( NULL != mSurface4.get() ) {
        mSurface4.clear();
    }

    if ( NULL != mSurfaceCtrl3.get() ) {
        mSurfaceCtrl3->clear();
        mSurfaceCtrl3.clear();
    }

    if ( NULL != mSurfaceCtrl4.get() ) {
        mSurfaceCtrl4->clear();
        mSurfaceCtrl4.clear();
    }

    deinit();

    printf("\n\nError Scenario test completed\n\n");

}


/** Initialize the surfaces**/
int OverlayTest::init()
{
    LOGV("init++");
    // set up the thread-pool
    mProc = (ProcessState::self());
    ProcessState::self()->startThreadPool();

    // create a client to surfaceflinger
    mClient1 = new SurfaceComposerClient();
    if (mClient1 == NULL)
    {
        LOGE("No Memory[%d]", __LINE__);
        return -1;
    }
    // create pushbuffer surface
    mSurfaceCtrl1 = (mClient1->createSurface(getpid(), OVERLAY_ON_PRIMARY, LCD_WIDTH, LCD_HEIGHT,
                        PIXEL_FORMAT_UNKNOWN, ISurfaceComposer::ePushBuffers | ISurfaceComposer::eFXSurfaceNormal));
    if (mSurfaceCtrl1 == NULL)
    {
        LOGE("No Memory[%d]", __LINE__);
        return -1;
    }

    // get to the isurface
    Test::getISurface(mSurfaceCtrl1);
    mSurface1 = mSurface;
    if (mSurface1 == NULL)
    {
        LOGE("No Memory[%d]", __LINE__);
        return -1;
    }

    // create a client to surfaceflinger
    mClient2 = new SurfaceComposerClient();
    if (mClient2 == NULL)
    {
        LOGE("No Memory[%d]", __LINE__);
        return -1;
    }
    // create pushbuffer surface
    mSurfaceCtrl2 = (mClient2->createSurface(getpid(), OVERLAY_ON_PRIMARY, LCD_WIDTH, LCD_HEIGHT,
                        PIXEL_FORMAT_UNKNOWN, ISurfaceComposer::ePushBuffers | ISurfaceComposer::eFXSurfaceNormal));
    if (mSurfaceCtrl2 == NULL)
    {
        LOGE("No Memory[%d]", __LINE__);
        return -1;
    }

    // get to the isurface
    Test::getISurface(mSurfaceCtrl2);
    mSurface2 = mSurface;
    if (mSurface2 == NULL)
    {
        LOGE("No Memory[%d]", __LINE__);
        return -1;
    }

    resetParams();
    return 0;
    LOGE("init--");

}

void OverlayTest::deinit()
{
    if (mOverlay1 != NULL)
    {
        mOverlay1->destroy();
        mOverlay1 = NULL;
    }
    if (mOverlay2 != NULL)
    {
        mOverlay2->destroy();
        mOverlay2 = NULL;
    }

    if ( NULL != mSurface1.get() ) {
        mSurface1.clear();
    }

    if ( NULL != mSurface2.get() ) {
        mSurface2.clear();
    }

    if ( NULL != mSurfaceCtrl1.get() ) {
        mSurfaceCtrl1->clear();
        mSurfaceCtrl1.clear();
    }

    if ( NULL != mSurfaceCtrl2.get() ) {
        mSurfaceCtrl2->clear();
        mSurfaceCtrl2.clear();
    }

    if ( NULL != mClient1.get() ) {
        mClient1->dispose();
        mClient1.clear();
    }

    if ( NULL != mClient2.get() ) {
        mClient2->dispose();
        mClient2.clear();
    }

}

}; //android name space

void printUsage()
{
    printf("\n\nUsage: for single overlay");
    printf("\noverlay_test <single/multi ovly> <test scenario> <file> <w> <h> <format> <panel>");
    printf("\nExample: ./system/bin/overlay_test S 1 test.yuv 320 240 1 0");

    printf("\n\nUsage: for multiple overlay");
    printf("\noverlay_test <single/multi ovly> <test scenario> <file1> <w1> <h1> <format1> <panel1> <file2> <w2> <h2> <format2> <panel2>");
    printf("\nExample: ./system/bin/overlay_test M 5 test1.yuv 320 240 2 0 test2.rgb 640 480 1 1");

    printf("\n\nTest Scenarios");
    printf("\n0 -\tDefalut playback");
    printf("\n1 -\tTo test Alpha Blend");
    printf("\n2 -\tTo test Scaling");
    printf("\n3 -\tTo test Rotation");
    printf("\n4 -\tTo test Mirroring");
    printf("\n5 -\tTo test z-order");
    printf("\n6 -\tTo test digital Zoom");
    printf("\n7 -\tTo test auto scale");
    printf("\n8 -\tTo test resize Overlay and dynamic buffer alloc");
    printf("\n9 -\tTo test all Error Scenarios");

    printf("\n\nFORMAT Ids");
    printf("\n0 - RGB565");
    printf("\n1 - RGBA8888");
    printf("\n2 - BGRA8888");
    printf("\n3 - YUV422-SP");
    printf("\n4 - YUV420-SP");
    printf("\n5 - YUYV422-I");
    printf("\n6 - YUYV420-I");
    printf("\n7 - UYVY422-I");
    printf("\n8 - UYVY420-I");

    printf("\n\nPANEL Ids");
    printf("\n0 -\tPrimary LCD");
    printf("\n1 -\tSecondary LCD");
    printf("\n2 -\tHDTV");
    printf("\n3 -\tPICO DLP");

    printf("\n\nOptional Arguments");
    printf("\n-x val : win x");
    printf("\n-y val : win y");
    printf("\n-w val : New Width");
    printf("\n-h val : New Height");
    printf("\n-i val : Repeat test 'val' times");

    printf("\n\n");
}


/** main entry point**/
int main (int argc, char* argv[])
{

    int opt;
    const char* const short_options = "x:y:w:h:p:n:i:";
    const struct option long_options[] = {
        {"win_x",           1, NULL, 'x'},
        {"win_y",           1, NULL, 'y'},
        {"newwidth1",       1, NULL, 'w'},
        {"newheight1",      1, NULL, 'h'},
        {"iterations",      1, NULL, 'i'},
        {NULL,               0, NULL, 0}
    };

    char* img1, img2;
    uint32_t w1, w2, h1, h2;
    uint8_t fmt1, fmt2, panel1, panel2;

    if (argc < 8){
        printUsage();
        return 0;
    }

    OverlayTest test;
    int ret = test.init();
    if (ret != 0)
    {
        printf("Overlay Test Init failed\n");
        return 0;
    }

    int val = -1;
    int stressTest = 1;
    bool cmp1 = strcmp(argv[1], "S");
    bool cmp2 = strcmp(argv[1], "s");

    if ((!cmp1) || (!cmp2))
    {
        switch(atoi(argv[2]))
        {
        case 1:
            printf("Enter the Alpha value (0 to 255)\n");
            scanf("%d", &val);
            test.alpha1 = val/1.0;
        break;

        case 2:
            printf("Enter the new width\n");
            scanf("%d", &val);
            test.newwidth1 = val;
            printf("Enter the new height\n");
            scanf("%d", &val);
            test.newheight1 = val;
        break;

        case 3:
            printf("Enter the rotation angle (0/90/180/270\n");
            scanf("%d", &val);
            test.deg1 = val;
        break;

        case 4:
            printf("Enter the mirror (1-Horizontal, 2-Vertical)\n");
            scanf("%d", &val);
            test.mirror1 = val;
        break;

        case 5:
            printf("Enter the zOrder (0 to 3)\n");
            scanf("%d", &val);
            test.zOrder1 = val;
        break;

        case 6:
            test.zoom1 = true;
        break;

        case 7:
            test.autoscale1 = true;
        break;

        case 8:
            test.resize = true;
        break;

        case 9:
            test.testErrorScenarios();
        return 0;

        default:
            printf("starting with default settings\n");
            test.alpha1 = 1.0;
        };

        img1 = argv[3];
        w1 = atoi(argv[4]);
        h1 = atoi(argv[5]);
        fmt1 = atoi(argv[6]);
        panel1 = atoi(argv[7]);

        while((opt = getopt_long(argc, argv, short_options, long_options, NULL)) != -1) {
            switch(opt) {
                case 'x':
                    test.win_x = atoi(optarg);
                    break;
                case 'y':
                    test.win_y = atoi(optarg);
                    break;
                case 'w':
                    test.newwidth1 = atoi(optarg);
                    break;
                case 'h':
                    test.newheight1 = atoi(optarg);
                    break;
                case 'i':
                    stressTest = MIN(5000, atoi(optarg));
                    break;
                case ':':
                    LOGE("\nError - Option `%c' needs a value\n\n", optopt);
                    return -1;
                case '?':
                    LOGE("\nError - No such option: `%c'\n\n", optopt);
                    return -1;
            }
        }

        for (int i = 0; i < stressTest; i ++)
        {
            LOGD("\n\n---------------------Iteration %d ------------------------------\n\n", i);
            test.testOverlay(img1, w1, h1, fmt1, panel1);
            sleep(2); //Sleep for 2 seconds. Give SF time to destroy overlay
        }
    }
    else
    {
        if (argc < 13){
            printUsage();
            return 0;
        }

        LOGD("Testing Multiple Overlays");
        switch(atoi(argv[2]))
        {
            case 1:
                printf("Enter the first Alpha value (0 to 255)\n");
                scanf("%d", &val);
                test.alpha1 = val/1.0;
                printf("Enter the second Alpha value (0 to 255)\n");
                scanf("%d", &val);
                test.alpha2 = val/1.0;
            break;

            case 2:
                printf("Enter the first new width\n");
                scanf("%d", &val);
                test.newwidth1 = val;
                printf("Enter the first new height\n");
                scanf("%d", &val);
                test.newheight1 = val;
                printf("Enter the sec new width\n");
                scanf("%d", &val);
                test.newwidth2 = val;
                printf("Enter the sec new height\n");
                scanf("%d", &val);
                test.newheight2 = val;
            break;

            case 3:
                printf("Enter the first rotation angle (0/90/180/270)\n");
                scanf("%d", &val);
                test.deg1 = val;
                printf("Enter the sec rotation angle (0/90/180/270)\n");
                scanf("%d", &val);
                test.deg2 = val;
            break;

            case 4:
                printf("Enter the first mirror (1-Horizontal, 2-Vertical)\n");
                scanf("%d", &val);
                test.mirror1 = val;
                printf("Enter the sec mirror (1-Horizontal, 2-Vertical)\n");
                scanf("%d", &val);
                test.mirror2 = val;
            break;

            case 5:
                printf("Enter the first zOrder (0 to 3)\n");
                scanf("%d", &val);
                test.zOrder1 = val;
                printf("Enter the sec zOrder (0 to 3)\n");
                scanf("%d", &val);
                test.zOrder2 = val;
            break;

            case 6:
                test.zoom1 = true;
                test.zoom2 = true;
            break;

            case 7:
                test.autoscale1 = true;
                test.autoscale2 = true;
            break;

            case 8:
                test.resize = true;
            break;

            case 9:
                test.testErrorScenarios();
            return 0;

            default:
                printf("starting with default settings\n");
                test.alpha1 = 1.0;
                test.alpha2 = 1.0;
        };

        for (int i = 0; i < stressTest; i ++)
        {
            LOGD("\n\n---------------------Iteration %d ------------------------------\n\n", i);
            test.testOverlay(argv[3], atoi(argv[4]), atoi(argv[5]), atoi(argv[6]), atoi(argv[7]), argv[8], atoi(argv[9]), atoi(argv[10]), atoi(argv[11]), atoi(argv[12]));
            sleep(2); //Sleep for 2 seconds. Give SF time to destroy overlay
        }

    }
return 0;
}
