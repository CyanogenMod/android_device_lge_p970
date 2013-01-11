/*
 * Copyright (C) 2011 The CyanogenMod Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


//#define LOG_NDEBUG 0
#define LOG_TAG "lights"

#include <cutils/log.h>

#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>

#include <sys/ioctl.h>
#include <sys/types.h>

#include <hardware/lights.h>

/******************************************************************************/

static pthread_once_t g_init = PTHREAD_ONCE_INIT;
static pthread_mutex_t g_lock = PTHREAD_MUTEX_INITIALIZER;
static struct light_state_t g_notification;

char const*const LCD_FILE
        = "/sys/class/leds/lcd-backlight/brightness";

char const*const LCD_STATE
        = "/sys/class/leds/lcd-backlight/onoff";

char const*const BUTTON_BRIGHTNESS
        = "/sys/devices/platform/omap/omap_i2c.2/i2c-2/2-001a/led_brightness";

char const*const BUTTON_STATE
        = "/sys/devices/platform/omap/omap_i2c.2/i2c-2/2-001a/led_onoff";
char const*const BUTTON_SYNC
        = "/sys/devices/platform/omap/omap_i2c.2/i2c-2/2-001a/led_sync";

char const*const BUTTON_PULSE
        = "/sys/devices/platform/omap/omap_i2c.2/i2c-2/2-001a/blink_enable";

char const*const BUTTON_MENU1
        = "/sys/devices/platform/omap/omap_i2c.2/i2c-2/2-001a/0x06";
char const*const BUTTON_MENU2
        = "/sys/devices/platform/omap/omap_i2c.2/i2c-2/2-001a/0x07";

char const*const BUTTON_HOME1
        = "/sys/devices/platform/omap/omap_i2c.2/i2c-2/2-001a/0x0D";
char const*const BUTTON_HOME2
        = "/sys/devices/platform/omap/omap_i2c.2/i2c-2/2-001a/0x0E";

char const*const BUTTON_BACK1
        = "/sys/devices/platform/omap/omap_i2c.2/i2c-2/2-001a/0x10";
char const*const BUTTON_BACK2
        = "/sys/devices/platform/omap/omap_i2c.2/i2c-2/2-001a/0x11";

char const*const BUTTON_SEARCH1
        = "/sys/devices/platform/omap/omap_i2c.2/i2c-2/2-001a/0x03";
char const*const BUTTON_SEARCH2
        = "/sys/devices/platform/omap/omap_i2c.2/i2c-2/2-001a/0x04";

char const*const BUTTON_BLUELEFT1
        = "/sys/devices/platform/omap/omap_i2c.2/i2c-2/2-001a/0x09";
char const*const BUTTON_BLUELEFT2
        = "/sys/devices/platform/omap/omap_i2c.2/i2c-2/2-001a/0x0A";

char const*const BUTTON_BLUERIGHT1
        = "/sys/devices/platform/omap/omap_i2c.2/i2c-2/2-001a/0x13";
char const*const BUTTON_BLUERIGHT2
        = "/sys/devices/platform/omap/omap_i2c.2/i2c-2/2-001a/0x14";

/**
 * device methods
 */

void init_globals(void)
{
    // init the mutex
    pthread_mutex_init(&g_lock, NULL);

}

static int
write_int(char const* path, int value)
{
    int fd;
    static int already_warned = 0;

    fd = open(path, O_RDWR);
    if (fd >= 0) {
        char buffer[20];
        int bytes = sprintf(buffer, "%d\n", value);
        int amt = write(fd, buffer, bytes);
        close(fd);
        return amt == -1 ? -errno : 0;
    } else {
        if (already_warned == 0) {
            ALOGE("write_int failed to open %s\n", path);
            already_warned = 1;
        }
        return -errno;
    }
}

static int
is_lit(struct light_state_t const* state)
{
    return state->color & 0x00ffffff;
}

static int
rgb_to_brightness(struct light_state_t const* state)
{
    int color = state->color & 0x00ffffff;
    return ((77*((color>>16)&0x00ff))
            + (150*((color>>8)&0x00ff)) + (29*(color&0x00ff))) >> 8;
}

static int
set_light_buttons(struct light_device_t* dev,
        struct light_state_t const* state)
{
    int err = 0;
    int on = is_lit(state);
    long value = rgb_to_brightness(state);

    value /= 7;

    ALOGV("Setting button brightness to %ld",value);

    pthread_mutex_lock(&g_lock);
    err = write_int(BUTTON_STATE, value ? 1 : 0);
    write_int(BUTTON_SYNC, 1);
    write_int(BUTTON_BRIGHTNESS, (int)value);
    pthread_mutex_unlock(&g_lock);
    return err;
}

static int
set_light_backlight(struct light_device_t* dev,
        struct light_state_t const* state)
{
    int err = 0;
    int brightness = rgb_to_brightness(state);

    ALOGV("Setting display brightness to %d",brightness);

    pthread_mutex_lock(&g_lock);
    if (brightness) {
        write_int(LCD_STATE, 1);
    } else {
        write_int(LCD_STATE, 0);
    }
    err = write_int(LCD_FILE, (brightness));
    pthread_mutex_unlock(&g_lock);

    return err;
}

static int
set_light_notifications(struct light_device_t* dev,
        struct light_state_t const* state)
{
    int err = 0;
    int on = is_lit(state);
    int red, green, blue = 0;

    red = (state->color >> 16) & 0xff;
    green = (state->color >> 8) & 0xff;
    blue = (state->color) & 0xff;

    /* Predominant color wins */
    if (blue > green) green = 0;
    if (blue > red) red = 0;
    if (red > green) green = 0;

    ALOGV("Calling notification light with state %d",on);
    pthread_mutex_lock(&g_lock);
    if (!on) {
        err = write_int(BUTTON_PULSE, 0);
        err = write_int(BUTTON_STATE, 1);
        err = write_int(BUTTON_SYNC, 1);
        err = write_int(BUTTON_BRIGHTNESS, 0);
    } else {
        err = write_int(BUTTON_STATE, 1);
        if (!err) {
            err = write_int(BUTTON_SYNC, 1);
            if (green) {
                err = write_int(BUTTON_PULSE, 20);
            } else if (red) {
                err = write_int(BUTTON_BRIGHTNESS, 20);
            } else if (blue) {
                write_int(BUTTON_MENU1, 0);
                write_int(BUTTON_MENU2, 0);
                write_int(BUTTON_HOME1, 0);
                write_int(BUTTON_HOME2, 0);
                write_int(BUTTON_BACK1, 0);
                write_int(BUTTON_BACK2, 0);
                write_int(BUTTON_SEARCH1, 0);
                write_int(BUTTON_SEARCH2, 0);
                write_int(BUTTON_BLUELEFT1, 20);
                write_int(BUTTON_BLUELEFT2, 20);
                write_int(BUTTON_BLUERIGHT1, 20);
                write_int(BUTTON_BLUERIGHT2, 20);
            }
        }
    }
    pthread_mutex_unlock(&g_lock);
    return err;
}

/** Close the lights device */
static int
close_lights(struct light_device_t *dev)
{
    if (dev) {
        free(dev);
    }
    return 0;
}


/******************************************************************************/

/**
 * module methods
 */

/** Open a new instance of a lights device using name */
static int open_lights(const struct hw_module_t* module, char const* name,
        struct hw_device_t** device)
{
    int (*set_light)(struct light_device_t* dev,
            struct light_state_t const* state);

    if (0 == strcmp(LIGHT_ID_BACKLIGHT, name)) {
        set_light = set_light_backlight;
    }
    else if (0 == strcmp(LIGHT_ID_BUTTONS, name)) {
        set_light = set_light_buttons;
    }
    else if (0 == strcmp(LIGHT_ID_NOTIFICATIONS, name)) {
        set_light = set_light_notifications;
    }
    else {
        return -EINVAL;
    }

    pthread_once(&g_init, init_globals);

    struct light_device_t *dev = malloc(sizeof(struct light_device_t));
    memset(dev, 0, sizeof(*dev));

    dev->common.tag = HARDWARE_DEVICE_TAG;
    dev->common.version = 0;
    dev->common.module = (struct hw_module_t*)module;
    dev->common.close = (int (*)(struct hw_device_t*))close_lights;
    dev->set_light = set_light;

    *device = (struct hw_device_t*)dev;
    return 0;
}


static struct hw_module_methods_t lights_module_methods = {
    .open =  open_lights,
};

/*
 * The lights Module
 */
struct hw_module_t HAL_MODULE_INFO_SYM = {
    .tag = HARDWARE_MODULE_TAG,
    .version_major = 1,
    .version_minor = 0,
    .id = LIGHTS_HARDWARE_MODULE_ID,
    .name = "LGE P970 lights Module",
    .author = "CyanogenMod Project",
    .methods = &lights_module_methods,
};
