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

#include <string.h>
#include <errno.h>
#include <sys/reboot.h>
#include <cutils/properties.h>

/* Pseudo recovery boot
 *
 * The P970 does not have a hardware key combo to boot into recovery,
 * so use a static binary + keychords to allow emergency recovery
 * startups even when /system is unavailable. Won't prevent bricks
 * from broken kernels, but should allow going into recovery as long
 * as init starts */

int main() {
	char bootstate[3];

	property_get("dev.bootcomplete", bootstate, "0");

	/* Regular boot completed, abort */
	if (strlen(bootstate) && !strncmp(bootstate,"1",1))
		return -ENODEV;

	__reboot(LINUX_REBOOT_MAGIC1, LINUX_REBOOT_MAGIC2, LINUX_REBOOT_CMD_RESTART2, "recovery");

	/* Should never get here */
	return -ENODEV;
}
