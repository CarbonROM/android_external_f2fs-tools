/**
 * f2fs_format.c
 *
 * Copyright (c) 2012 Samsung Electronics Co., Ltd.
 *             http://www.samsung.com/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#define _LARGEFILE64_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <time.h>
//#include <linux/fs.h>
#include <uuid/uuid.h>

#include "f2fs_fs.h"
#include "f2fs_format_utils.h"

extern struct f2fs_configuration config;

static void mkfs_usage()
{
	MSG(0, "\nUsage: mkfs.f2fs [options] device\n");
	MSG(0, "[options]:\n");
	MSG(0, "  -a heap-based allocation [default:1]\n");
	MSG(0, "  -d debug level [default:0]\n");
	MSG(0, "  -e [extension list] e.g. \"mp3,gif,mov\"\n");
	MSG(0, "  -l label\n");
	MSG(0, "  -o overprovision ratio [default:5]\n");
	MSG(0, "  -s # of segments per section [default:1]\n");
	MSG(0, "  -z # of sections per zone [default:1]\n");
	MSG(0, "  -t 0: nodiscard, 1: discard [default:1]\n");
	exit(1);
}

static void f2fs_parse_options(int argc, char *argv[])
{
	static const char *option_string = "a:d:e:l:o:s:z:t:";
	int32_t option=0;

	while ((option = getopt(argc,argv,option_string)) != EOF) {
		switch (option) {
		case 'a':
			config.heap = atoi(optarg);
			if (config.heap == 0)
				MSG(0, "Info: Disable heap-based policy\n");
			break;
		case 'd':
			config.dbg_lv = atoi(optarg);
			MSG(0, "Info: Debug level = %d\n", config.dbg_lv);
			break;
		case 'e':
			config.extension_list = strdup(optarg);
			MSG(0, "Info: Add new extension list\n");
			break;
		case 'l':		/*v: volume label */
			if (strlen(optarg) > 512) {
				MSG(0, "Error: Volume Label should be less than\
						512 characters\n");
				mkfs_usage();
			}
			config.vol_label = optarg;
			MSG(0, "Info: Label = %s\n", config.vol_label);
			break;
		case 'o':
			config.overprovision = atoi(optarg);
			MSG(0, "Info: Overprovision ratio = %u%%\n",
								atoi(optarg));
			break;
		case 's':
			config.segs_per_sec = atoi(optarg);
			MSG(0, "Info: Segments per section = %d\n",
								atoi(optarg));
			break;
		case 'z':
			config.secs_per_zone = atoi(optarg);
			MSG(0, "Info: Sections per zone = %d\n", atoi(optarg));
			break;
		case 't':
			config.trim = atoi(optarg);
			MSG(0, "Info: Trim is %s\n", config.trim ? "enabled": "disabled");
			break;
		default:
			MSG(0, "\tError: Unknown option %c\n",option);
			mkfs_usage();
			break;
		}
	}

	if ((optind + 1) != argc) {
		MSG(0, "\tError: Device not specified\n");
		mkfs_usage();
	}

	config.reserved_segments  =
			(2 * (100 / config.overprovision + 1) + 6)
			* config.segs_per_sec;
	config.device_name = argv[optind];
}

int main(int argc, char *argv[])
{
	MSG(0, "\n\tF2FS-tools: mkfs.f2fs Ver: %s (%s)\n\n",
				F2FS_TOOLS_VERSION,
				F2FS_TOOLS_DATE);
	f2fs_init_configuration(&config);

	f2fs_parse_options(argc, argv);

	if (f2fs_dev_is_umounted(&config) < 0)
		return -1;

	if (f2fs_get_device_info(&config) < 0)
		return -1;

	if (f2fs_format_device() < 0)
		return -1;

	MSG(0, "Info: format successful\n");

	return 0;
}