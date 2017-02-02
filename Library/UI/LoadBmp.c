/*
 * libeg/load_bmp.c
 * Loading function for BMP images
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

#include <Library/Platform/Platform.h>

//
// Load BMP image
//

EG_IMAGE *
DecodeBMP (
  IN UINT8    *FileData,
  IN UINTN    FileDataLength,
  IN UINTN    IconSize,
  IN BOOLEAN  WantAlpha
) {
  EG_IMAGE            *NewImage;
  BMP_IMAGE_HEADER    *BmpHeader;
  BMP_COLOR_MAP       *BmpColorMap;
  EG_PIXEL            *PixelPtr;
  UINTN               x, y, ImageLineOffset, Index, BitIndex;
  UINT8               *ImagePtr, *ImagePtrBase, AlphaValue, ImageValue = 0;

  // read and check header
  if ((FileDataLength < sizeof (BMP_IMAGE_HEADER)) || (FileData == NULL)) {
    return NULL;
  }

  BmpHeader = (BMP_IMAGE_HEADER *)FileData;

  if ((BmpHeader->CharB != 'B') || (BmpHeader->CharM != 'M')) {
    return NULL;
  }

  if (BmpHeader->CompressionType != 0) {
    return NULL;
  }

  if (
    (BmpHeader->BitPerPixel != 1) && (BmpHeader->BitPerPixel != 4) &&
    (BmpHeader->BitPerPixel != 8) && (BmpHeader->BitPerPixel != 24)
  ) {
    return NULL;
  }

  // calculate parameters
  ImageLineOffset = BmpHeader->PixelWidth;

  if (BmpHeader->BitPerPixel == 24) {
    ImageLineOffset *= 3;
  } else if (BmpHeader->BitPerPixel == 1) {
    ImageLineOffset = (ImageLineOffset + 7) >> 3;
  } else if (BmpHeader->BitPerPixel == 4) {
    ImageLineOffset = (ImageLineOffset + 1) >> 1;
  }

  if ((ImageLineOffset % 4) != 0) {
    ImageLineOffset = ImageLineOffset + (4 - (ImageLineOffset % 4));
  }

  // check bounds
  if (BmpHeader->ImageOffset + ImageLineOffset * BmpHeader->PixelHeight > FileDataLength) {
    return NULL;
  }

  // allocate image structure and buffer
  NewImage = CreateImage (BmpHeader->PixelWidth, BmpHeader->PixelHeight, WantAlpha);

  if (NewImage == NULL) {
    return NULL;
  }

  AlphaValue = WantAlpha ? 255 : 0;

  // convert image
  BmpColorMap = (BMP_COLOR_MAP *)(FileData + sizeof (BMP_IMAGE_HEADER));
  ImagePtrBase = FileData + BmpHeader->ImageOffset;

  for (y = 0; y < BmpHeader->PixelHeight; y++) {
    ImagePtr = ImagePtrBase;
    ImagePtrBase += ImageLineOffset;
    PixelPtr = NewImage->PixelData + (BmpHeader->PixelHeight - 1 - y) * BmpHeader->PixelWidth;

    switch (BmpHeader->BitPerPixel) {
      case 1:
        for (x = 0; x < BmpHeader->PixelWidth; x++) {
          BitIndex = x & 0x07;
          if (BitIndex == 0) {
            ImageValue = *ImagePtr++;
          }

          Index = (ImageValue >> (7 - BitIndex)) & 0x01;
          PixelPtr->b = BmpColorMap[Index].Blue;
          PixelPtr->g = BmpColorMap[Index].Green;
          PixelPtr->r = BmpColorMap[Index].Red;
          PixelPtr->a = AlphaValue;
          PixelPtr++;
        }
        break;

      case 4:
        for (x = 0; x <= BmpHeader->PixelWidth - 2; x += 2) {
          ImageValue = *ImagePtr++;

          Index = ImageValue >> 4;
          PixelPtr->b = BmpColorMap[Index].Blue;
          PixelPtr->g = BmpColorMap[Index].Green;
          PixelPtr->r = BmpColorMap[Index].Red;
          PixelPtr->a = AlphaValue;
          PixelPtr++;

          Index = ImageValue & 0x0f;
          PixelPtr->b = BmpColorMap[Index].Blue;
          PixelPtr->g = BmpColorMap[Index].Green;
          PixelPtr->r = BmpColorMap[Index].Red;
          PixelPtr->a = AlphaValue;
          PixelPtr++;
        }

        if (x < BmpHeader->PixelWidth) {
          ImageValue = *ImagePtr++;

          Index = ImageValue >> 4;
          PixelPtr->b = BmpColorMap[Index].Blue;
          PixelPtr->g = BmpColorMap[Index].Green;
          PixelPtr->r = BmpColorMap[Index].Red;
          PixelPtr->a = AlphaValue;
          PixelPtr++;
        }
        break;

      case 8:
        for (x = 0; x < BmpHeader->PixelWidth; x++) {
          Index = *ImagePtr++;
          PixelPtr->b = BmpColorMap[Index].Blue;
          PixelPtr->g = BmpColorMap[Index].Green;
          PixelPtr->r = BmpColorMap[Index].Red;
          PixelPtr->a = AlphaValue;
          PixelPtr++;
        }
        break;

      case 24:
        for (x = 0; x < BmpHeader->PixelWidth; x++) {
          PixelPtr->b = *ImagePtr++;
          PixelPtr->g = *ImagePtr++;
          PixelPtr->r = *ImagePtr++;
          PixelPtr->a = AlphaValue;
          PixelPtr++;
        }
        break;
    }
  }

  return NewImage;
}

//
// Save BMP image
//

VOID
EncodeBMP (
  IN  EG_IMAGE  *Image,
  OUT UINT8     **FileDataReturn,
  OUT UINTN     *FileDataLengthReturn
) {
  BMP_IMAGE_HEADER    *BmpHeader;
  EG_PIXEL            *PixelPtr;
  UINT8               *FileData, *ImagePtr, *ImagePtrBase;
  INT64               x, y;
  UINT64              FileDataLength, ImageLineOffset;

  ImageLineOffset = MultU64x32 (Image->Width, 3);
  //if ((ImageLineOffset % 4) != 0) {
  //  ImageLineOffset = ImageLineOffset + (4 - (ImageLineOffset % 4));
  //}

  if ((ImageLineOffset & 3) != 0) {
    ImageLineOffset = ImageLineOffset + (4 - (ImageLineOffset & 3));
  }

  // allocate buffer for file data
  FileDataLength = sizeof (BMP_IMAGE_HEADER) + MultU64x64 (Image->Height, ImageLineOffset);
  FileData = AllocateZeroPool ((UINTN)FileDataLength);
  if (FileData == NULL) {
    Print (L"Error allocate %d bytes\n", FileDataLength);
    *FileDataReturn = NULL;
    *FileDataLengthReturn = 0;
    return;
  }

  // fill header
  BmpHeader = (BMP_IMAGE_HEADER *)FileData;
  BmpHeader->CharB = 'B';
  BmpHeader->CharM = 'M';
  BmpHeader->Size = (UINT32)FileDataLength;
  BmpHeader->ImageOffset = sizeof (BMP_IMAGE_HEADER);
  BmpHeader->HeaderSize = 40;
  BmpHeader->PixelWidth = (UINT32)Image->Width;
  BmpHeader->PixelHeight = (UINT32)Image->Height;
  BmpHeader->Planes = 1;
  BmpHeader->BitPerPixel = 24;
  BmpHeader->CompressionType = 0;
  BmpHeader->XPixelsPerMeter = 0xb13;
  BmpHeader->YPixelsPerMeter = 0xb13;

  // fill pixel buffer
  ImagePtrBase = FileData + BmpHeader->ImageOffset;
  for (y = 0; y < Image->Height; y++) {
    ImagePtr = ImagePtrBase;
    ImagePtrBase += ImageLineOffset;
    //PixelPtr = Image->PixelData + (Image->Height - 1 - y) * Image->Width;
    PixelPtr = Image->PixelData + (INT32)(Image->Height - 1 - y) * (INT32)Image->Width;

    for (x = 0; x < Image->Width; x++) {
      *ImagePtr++ = PixelPtr->b;
      *ImagePtr++ = PixelPtr->g;
      *ImagePtr++ = PixelPtr->r;
      PixelPtr++;
    }
  }

  *FileDataReturn = FileData;
  *FileDataLengthReturn = (UINTN)FileDataLength;
}

/* EOF */