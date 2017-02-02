/*
 * Copyright (c) 2000 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. The rights granted to you under the License
 * may not be used to create, or enable the creation or redistribution of,
 * unlawful or unlicensed copies of an Apple operating system, or to
 * circumvent, violate, or enable the circumvention or violation of, any
 * terms of an Apple operating system software license agreement.
 *
 * Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_END@
 */

#ifndef _PEXPERT_I386_BOOT_H
#define _PEXPERT_I386_BOOT_H

//#include <stdint.h>

/*
 * What the booter leaves behind for the kernel.
 */

/*
 * Types of boot driver that may be loaded by the booter.
 */
enum {
  kBootDriverTypeInvalid        = 0,
  kBootDriverTypeKEXT           = 1,
  kBootDriverTypeMKEXT          = 2
};

enum {
  kEfiReservedMemoryType        = 0,
  kEfiLoaderCode                = 1,
  kEfiLoaderData                = 2,
  kEfiBootServicesCode          = 3,
  kEfiBootServicesData          = 4,
  kEfiRuntimeServicesCode       = 5,
  kEfiRuntimeServicesData       = 6,
  kEfiConventionalMemory        = 7,
  kEfiUnusableMemory            = 8,
  kEfiACPIReclaimMemory         = 9,
  kEfiACPIMemoryNVS             = 10,
  kEfiMemoryMappedIO            = 11,
  kEfiMemoryMappedIOPortSpace   = 12,
  kEfiPalCode                   = 13,
  kEfiMaxMemoryType             = 14
};

/*
 * Memory range descriptor.
 */
typedef struct EfiMemoryRange {
  UINT32  Type;
  UINT32  Pad;
  UINT64  PhysicalStart;
  UINT64  VirtualStart;
  UINT64  NumberOfPages;
  UINT64  Attribute;
} EfiMemoryRange;

#define BOOT_LINE_LENGTH        1024
#define BOOT_STRING_LEN         BOOT_LINE_LENGTH

/*
 * Video information..
 */
struct Boot_VideoV1 {
  UINT32  v_baseAddr; /* Base address of video memory */
  UINT32  v_display;  /* Display Code (if Applicable */
  UINT32  v_rowBytes; /* Number of bytes per pixel row */
  UINT32  v_width;  /* Width */
  UINT32  v_height; /* Height */
  UINT32  v_depth;  /* Pixel Depth */
};

typedef struct Boot_VideoV1 Boot_VideoV1;

struct Boot_Video {
  UINT32  v_display;  /* Display Code (if Applicable */
  UINT32  v_rowBytes; /* Number of bytes per pixel row */
  UINT32  v_width;  /* Width */
  UINT32  v_height; /* Height */
  UINT32  v_depth;  /* Pixel Depth */
  UINT32  v_resv[7];  /* Reserved */
  UINT64  v_baseAddr; /* Base address of video memory */
};

typedef struct Boot_Video Boot_Video;

/* Values for v_display */

#define VGA_TEXT_MODE           0
#define GRAPHICS_MODE           1
#define FB_TEXT_MODE            2

/* Boot argument structure - passed into Mach kernel at boot time.
 * "Revision" can be incremented for compatible changes
 */
#define kBootArgsRevision       0
#define kBootArgsVersion        2

#define kBootArgsVersion2       2

#define kBootArgsEfiMode64      64

/* Bitfields for boot_args->flags */
#define kBootArgsFlagRebootOnPanic      (1 << 0)
#define kBootArgsFlagHiDPI              (1 << 1)
#define kBootArgsFlagBlack              (1 << 2)
#define kBootArgsFlagCSRActiveConfig    (1 << 3)
#define kBootArgsFlagCSRConfigMode      (1 << 4)
#define kBootArgsFlagCSRBoot            (1 << 5)
#define kBootArgsFlagBlackBg            (1 << 6)
#define kBootArgsFlagLoginUI            (1 << 7)
#define kBootArgsFlagInstallUI          (1 << 8)

/* Rootless configuration flags */
#define CSR_ALLOW_UNTRUSTED_KEXTS       (1 << 0)
#define CSR_ALLOW_UNRESTRICTED_FS       (1 << 1)
#define CSR_ALLOW_TASK_FOR_PID          (1 << 2)
#define CSR_ALLOW_KERNEL_DEBUGGER       (1 << 3)
#define CSR_ALLOW_APPLE_INTERNAL        (1 << 4)
//#define CSR_ALLOW_DESTRUCTIVE_DTRACE    (1 << 5) /* name deprecated */
#define CSR_ALLOW_UNRESTRICTED_DTRACE   (1 << 5)
#define CSR_ALLOW_UNRESTRICTED_NVRAM    (1 << 6)
#define CSR_ALLOW_DEVICE_CONFIGURATION  (1 << 7)
#define CSR_ALLOW_ANY_RECOVERY_OS       (1 << 8)

#define CSR_VALID_FLAGS (CSR_ALLOW_UNTRUSTED_KEXTS | \
                         CSR_ALLOW_UNRESTRICTED_FS | \
                         CSR_ALLOW_TASK_FOR_PID | \
                         CSR_ALLOW_KERNEL_DEBUGGER | \
                         CSR_ALLOW_APPLE_INTERNAL | \
                         CSR_ALLOW_UNRESTRICTED_DTRACE | \
                         CSR_ALLOW_UNRESTRICTED_NVRAM | \
                         CSR_ALLOW_DEVICE_CONFIGURATION | \
                         CSR_ALLOW_ANY_RECOVERY_OS)

/* CSR capabilities that a booter can give to the system */
#define CSR_CAPABILITY_UNLIMITED        (1 << 0)
#define CSR_CAPABILITY_CONFIG           (1 << 1)
#define CSR_CAPABILITY_APPLE_INTERNAL   (1 << 2)

#define CSR_VALID_CAPABILITIES (CSR_CAPABILITY_UNLIMITED | CSR_CAPABILITY_CONFIG | CSR_CAPABILITY_APPLE_INTERNAL)

typedef struct {
  UINT16          Revision; /* Revision of boot_args structure */
  UINT16          Version;  /* Version of boot_args structure */

  UINT8           efiMode;    /* 32 = 32-bit, 64 = 64-bit */
  UINT8           debugMode;  /* Bit field with behavior changes */
  UINT16          flags;

  CHAR8           CommandLine[BOOT_LINE_LENGTH];  /* Passed in command line */

  UINT32          MemoryMap;  /* Physical address of memory map */
  UINT32          MemoryMapSize;
  UINT32          MemoryMapDescriptorSize;
  UINT32          MemoryMapDescriptorVersion;

  Boot_VideoV1    VideoV1; /* Video Information */

  UINT32          deviceTreeP;    /* Physical address of flattened device tree */
  UINT32          deviceTreeLength; /* Length of flattened tree */

  UINT32          kaddr;            /* Physical address of beginning of kernel text */
  UINT32          ksize;            /* Size of combined kernel text+data+efi */

  UINT32          efiRuntimeServicesPageStart; /* physical address of defragmented runtime pages */
  UINT32          efiRuntimeServicesPageCount;
  UINT64          efiRuntimeServicesVirtualPageStart; /* virtual address of defragmented runtime pages */

  UINT32          efiSystemTable;   /* physical address of system table in runtime area */
  UINT32          kslide;

  UINT32          performanceDataStart; /* physical address of log */
  UINT32          performanceDataSize;

  UINT32          keyStoreDataStart; /* physical address of key store data */
  UINT32          keyStoreDataSize;
  UINT64          bootMemStart;
  UINT64          bootMemSize;
  UINT64          PhysicalMemorySize;
  UINT64          FSBFrequency;
  UINT64          pciConfigSpaceBaseAddress;
  UINT32          pciConfigSpaceStartBusNumber;
  UINT32          pciConfigSpaceEndBusNumber;
  UINT32          csrActiveConfig;
  UINT32          csrCapabilities;
  UINT32          boot_SMC_plimit;
  UINT16          bootProgressMeterStart;
  UINT16          bootProgressMeterEnd;
  Boot_Video      Video;    /* Video Information */
  UINT32          __reserved4[712];
} BootArgs2;

#endif /* _PEXPERT_I386_BOOT_H */