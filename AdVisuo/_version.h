// _version.h

#pragma once

#define VERSION 30000	// 3.00.00

#define VERSION_MAJOR	(VERSION / 10000)
#define VERSION_MINOR	((VERSION % 10000) / 100)
#define VERSION_REV		(VERSION % 100)

#define VERSION_DATE	#__DATE__

// revisions:
// 00 - 09 - alpha or work in progress
// 10 - 19 - beta
// 20 - 29 - release candidate or early release
// 30 - 39 - stable release
