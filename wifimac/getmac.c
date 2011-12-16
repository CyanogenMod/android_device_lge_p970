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

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/* Read device NVRAM and set the Wifi MAC address accordingly */

int main() {
	int fd1; FILE *fd2;
	int macbyte;
	int i;

	fd1 = open("/dev/block/mmcblk0p11",O_RDONLY);
	fd2 = fopen("/data/misc/wifi/config","w");

	fprintf(fd2,"vlan_mode=0\n\
mpc=1\n\
wme=1\n\
wme_apsd=0\n\
wme_qosinfo=0x00\n\
wme_auto_trigger=0\n\
wme_apsd_trigger=290000\n\
roam_off=0\n\
roam_scan_period=20\n\
roam_delta=20\n\
roam_trigger=-70\n\
PM=2\n\
assoc_listen=1\n\
assoc_retry_max=7\n\
");

	fprintf(fd2,"cur_etheraddr=");
	for (i = 0; i<6; i++) {
		macbyte=0;
		lseek(fd1,i+514,SEEK_SET);
		read(fd1,&macbyte,1);
		fprintf(fd2,"%.2x",macbyte);
		if (i!=5) fprintf(fd2,":");
		else fprintf(fd2,"\n");
	}

	close(fd1);
	fclose(fd2);
	return 0;
}
