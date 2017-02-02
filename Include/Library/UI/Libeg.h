/*
 * libeg/libeg.h
 * EFI graphics library header for users
 *
 * Copyright (c) 2006 Christoph Pfisterer
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 *  * Neither the name of Christoph Pfisterer nor the names of the
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __LIBEG_LIBEG_H__
#define __LIBEG_LIBEG_H__

#include <PiDxe.h>
#include <Base.h>
#include <Uefi.h>
#include <FrameworkDxe.h>

// Protocol Includes
#include <Protocol/AcpiTable.h>
#include <Protocol/Cpu.h>
#include <Protocol/DataHub.h>
#include <Protocol/DevicePath.h>
#include <Protocol/DevicePathFromText.h>
#include <Protocol/DevicePathToText.h>
#include <Protocol/EdidActive.h>
#include <Protocol/EdidDiscovered.h>
#include <Protocol/GraphicsOutput.h>
#include <Protocol/LoadedImage.h>
#include <Protocol/PciIo.h>
#include <Protocol/ScsiIo.h>
#include <Protocol/SimpleFileSystem.h>
#include <Protocol/SimpleTextIn.h>
#include <Protocol/SimpleTextOut.h>
#include <Protocol/Smbios.h>
#include <Protocol/UnicodeCollation.h>

// Guid Includes
#include <Guid/Acpi.h>
#include <Guid/ConsoleInDevice.h>
#include <Guid/ConsoleOutDevice.h>
#include <Guid/DataHubRecords.h>
#include <Guid/DxeServices.h>
#include <Guid/EventGroup.h>
#include <Guid/FileInfo.h>
#include <Guid/FileSystemInfo.h>
#include <Guid/FileSystemVolumeLabelInfo.h>
#include <Guid/GlobalVariable.h>
#include <Guid/MemoryTypeInformation.h>
#include <Guid/MemoryAllocationHob.h>
#include <Guid/SmBios.h>
#include <Guid/StandardErrorDevice.h>

// Library Includes
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/DevicePathLib.h>
#include <Library/DxeServicesLib.h>
#include <Library/DxeServicesTableLib.h>
#include <Library/HobLib.h>
#include <Library/IoLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PcdLib.h>
#include <Library/PrintLib.h>
#include <Library/TimerLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>

// IndustryStandard Includes
#include <IndustryStandard/Pci.h>
#include <IndustryStandard/SmBus.h>
#include <IndustryStandard/Acpi.h>
#include <IndustryStandard/HighPrecisionEventTimerTable.h>
#include <IndustryStandard/AppleSmBios.h>

/* types */

typedef enum {
  //FONT_NONE,
  FONT_ALFA,
  FONT_GRAY,
  FONT_LOAD,
  FONT_RAW
} FONT_TYPE;

/* This should be compatible with EFI_UGA_PIXEL */
typedef struct {
  UINT8 b, g, r, a;
} EG_PIXEL;

typedef struct {
  INTN        Width;
  INTN        Height;
  EG_PIXEL    *PixelData;
  BOOLEAN     HasAlpha;   //moved here to avoid alignment issue
} EG_IMAGE;

typedef struct {
  INTN     XPos;
  INTN     YPos;
  INTN     Width;
  INTN     Height;
} EG_RECT;

#define TEXT_YMARGIN (2)
#define TEXT_XMARGIN (8)

#define EG_EIPIXELMODE_GRAY         (0)
#define EG_EIPIXELMODE_GRAY_ALPHA   (1)
#define EG_EIPIXELMODE_COLOR        (2)
#define EG_EIPIXELMODE_COLOR_ALPHA  (3)
#define EG_EIPIXELMODE_ALPHA        (4)
#define EG_MAX_EIPIXELMODE          EG_EIPIXELMODE_ALPHA

#define EG_EICOMPMODE_NONE          (0)
#define EG_EICOMPMODE_RLE           (1)
#define EG_EICOMPMODE_EFICOMPRESS   (2)

typedef struct {
        INTN          Width;
        INTN          Height;
        UINTN         PixelMode;
        UINTN         CompressMode;
  CONST UINT8         *Data;
        UINTN         DataLength;
} EG_EMBEDDED_IMAGE;

typedef struct {
  EG_IMAGE    *Image;
  CHAR16      *Path;
  CHAR16      *Format;
  UINTN       PixelSize; // for .icns
} BUILTIN_ICON;

/* functions */

//VOID
//egInitScreen (
//  IN BOOLEAN    SetMaxResolution
//);

VOID
DumpGOPVideoModes ();

EFI_STATUS
SetMaxResolution ();

EFI_STATUS
SetScreenResolution (
  IN CHAR16     *WidthHeight
);

EFI_STATUS
SetMode (
  INT32     Next
);

VOID
GetScreenSize (
  OUT INTN    *ScreenWidth,
  OUT INTN    *ScreenHeight
);

CHAR16 *
ScreenDescription ();

BOOLEAN
HasGraphicsMode ();

BOOLEAN
IsGraphicsModeEnabled ();

VOID
SetGraphicsModeEnabled (
  IN BOOLEAN    Enable
);

// NOTE: Even when HasGraphicsMode () returns FALSE, you should
//  call SetGraphicsModeEnabled (FALSE) to ensure the system
//  is running in text mode. HasGraphicsMode () only determines
//  if libeg can draw to the screen in graphics mode.

EG_IMAGE *
CreateImage (
  IN INTN       Width,
  IN INTN       Height,
  IN BOOLEAN    HasAlpha
);

EG_IMAGE *
CreateFilledImage (
  IN INTN       Width,
  IN INTN       Height,
  IN BOOLEAN    HasAlpha,
  IN EG_PIXEL   *Color
);

EG_IMAGE *
CopyImage (
  IN EG_IMAGE   *Image
);

EG_IMAGE *
CopyScaledImage (
  IN EG_IMAGE   *OldImage,
  IN INTN       Ratio
);

VOID
FreeImage (
  IN EG_IMAGE   *Image
);

VOID
ScaleImage (
  OUT EG_IMAGE    *NewImage,
  IN EG_IMAGE     *OldImage
);

EG_IMAGE *
LoadImage (
  IN EFI_FILE_HANDLE    BaseDir,
  IN CHAR16             *FileName,
  IN BOOLEAN            WantAlpha
);

EG_IMAGE *
LoadIcon (
  IN EFI_FILE_HANDLE  BaseDir,
  IN CHAR16           *FileName,
  IN UINTN            IconSize
);

EG_IMAGE *
PrepareEmbeddedImage (
  IN EG_EMBEDDED_IMAGE    *EmbeddedImage,
  IN BOOLEAN              WantAlpha
);

VOID
FillImage (
  IN OUT EG_IMAGE     *CompImage,
  IN EG_PIXEL         *Color
);

VOID
FillImageArea (
  IN OUT EG_IMAGE   *CompImage,
  IN INTN           AreaPosX,
  IN INTN           AreaPosY,
  IN INTN           AreaWidth,
  IN INTN           AreaHeight,
  IN EG_PIXEL       *Color
);

VOID
ComposeImage (
  IN OUT EG_IMAGE   *CompImage,
  IN EG_IMAGE       *TopImage,
  IN INTN           PosX,
  IN INTN           PosY
);

VOID
PrepareFont ();

VOID
MeasureText (
  IN  CHAR16    *Text,
  OUT INTN      *Width,
  OUT INTN      *Height
);

INTN
RenderText (
  IN CHAR16         *Text,
  IN OUT EG_IMAGE   *CompImage,
  IN INTN           PosX,
  IN INTN           PosY,
  IN INTN           Cursor,
     BOOLEAN        Selected
);

VOID
ClearScreen (
  IN EG_PIXEL     *Color
);

VOID
DrawImageArea (
  IN EG_IMAGE   *Image,
  IN INTN       AreaPosX,
  IN INTN       AreaPosY,
  IN INTN       AreaWidth,
  IN INTN       AreaHeight,
  IN INTN       ScreenPosX,
  IN INTN       ScreenPosY
);

VOID
TakeImage (
  IN  EG_IMAGE    *Image,
      INTN        ScreenPosX,
      INTN        ScreenPosY,
  IN  INTN        AreaWidth,
  IN  INTN        AreaHeight
);

EFI_STATUS
ScreenShot ();

#endif /* __LIBEG_LIBEG_H__ */

/* EOF */