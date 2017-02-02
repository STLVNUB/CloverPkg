/*
 * libeg/image.c
 * Image handling functions
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

#ifndef DEBUG_ALL
#ifndef DEBUG_IMG
#define DEBUG_IMG -1
#endif
#else
#ifdef DEBUG_IMG
#undef DEBUG_IMG
#endif
#define DEBUG_IMG DEBUG_ALL
#endif

#define DBG(...) DebugLog (DEBUG_IMG, __VA_ARGS__)

//
// Basic image handling
//

EG_IMAGE *
CreateImage (
  IN INTN       Width,
  IN INTN       Height,
  IN BOOLEAN    HasAlpha
) {
  EG_IMAGE    *NewImage;

  NewImage = (EG_IMAGE *)AllocatePool (sizeof (EG_IMAGE));

  if (NewImage == NULL){
    return NULL;
  }

  NewImage->PixelData = (EG_PIXEL *)AllocatePool ((UINTN)(Width * Height * sizeof (EG_PIXEL)));

  if (NewImage->PixelData == NULL) {
    FreePool (NewImage);
    return NULL;
  }

  NewImage->Width = Width;
  NewImage->Height = Height;
  NewImage->HasAlpha = HasAlpha;

  return NewImage;
}

EG_IMAGE *
CreateFilledImage (
  IN INTN       Width,
  IN INTN       Height,
  IN BOOLEAN    HasAlpha,
  IN EG_PIXEL   *Color
) {
  EG_IMAGE    *NewImage;

  NewImage = CreateImage (Width, Height, HasAlpha);

  if (NewImage == NULL) {
    return NULL;
  }

  FillImage (NewImage, Color);

  return NewImage;
}

EG_IMAGE *
CopyImage (
  IN EG_IMAGE   *Image
) {
  EG_IMAGE    *NewImage;

  if (!Image) {
    return NULL;
  }

  NewImage = CreateImage (Image->Width, Image->Height, Image->HasAlpha);

  if (NewImage == NULL) {
    return NULL;
  }

  CopyMem (NewImage->PixelData, Image->PixelData, (UINTN)(Image->Width * Image->Height * sizeof (EG_PIXEL)));

  return NewImage;
}

//Scaling functions
EG_IMAGE *
CopyScaledImage (
  IN EG_IMAGE   *OldImage,
  IN INTN       Ratio
) { //will be N/16
  //(c)Slice 2012
  BOOLEAN     Grey = FALSE;
  EG_IMAGE    *NewImage;
  INTN        x, x0, x1, x2, y, y0, y1, y2,
              NewH, NewW, OldW;
  EG_PIXEL    *Dest, *Src;

  if (Ratio < 0) {
    Ratio = -Ratio;
    Grey = TRUE;
  }

  if (!OldImage) {
    return NULL;
  }

  Src = OldImage->PixelData;
  OldW = OldImage->Width;

  NewW = (OldImage->Width * Ratio) >> 4;
  NewH = (OldImage->Height * Ratio) >> 4;

  if (Ratio == 16) {
    NewImage = CopyImage (OldImage);
  } else {
    NewImage = CreateImage (NewW, NewH, OldImage->HasAlpha);

    if (NewImage == NULL) {
      return NULL;
    }

    Dest = NewImage->PixelData;
    for (y = 0; y < NewH; y++) {
      y1 = (y << 4) / Ratio;
      y0 = ((y1 > 0) ? (y1 - 1) : y1) * OldW;
      y2 = ((y1 < (OldImage->Height - 1)) ? (y1 + 1) : y1) * OldW;
      y1 *= OldW;

      for (x = 0; x < NewW; x++) {
        x1 = (x << 4) / Ratio;
        x0 = (x1 > 0) ? (x1 - 1) : x1;
        x2 = (x1 < (OldW - 1)) ? (x1 + 1) : x1;
        Dest->b = (UINT8)(((INTN)Src[x1 + y1].b * 2 + Src[x0 + y1].b +
                           Src[x2 + y1].b + Src[x1 + y0].b + Src[x1 + y2].b) / 6);
        Dest->g = (UINT8)(((INTN)Src[x1 + y1].g * 2 + Src[x0 + y1].g +
                           Src[x2 + y1].g + Src[x1 + y0].g + Src[x1 + y2].g) / 6);
        Dest->r = (UINT8)(((INTN)Src[x1 + y1].r * 2 + Src[x0 + y1].r +
                           Src[x2 + y1].r + Src[x1 + y0].r + Src[x1 + y2].r) / 6);
        Dest->a = Src[x1 + y1].a;
        Dest++;
      }
    }
  }

  if (Grey) {
    Dest = NewImage->PixelData;
    for (y = 0; y < NewH; y++) {
      for (x = 0; x < NewW; x++) {
        Dest->b = (UINT8)((INTN)((UINTN)Dest->b + (UINTN)Dest->g + (UINTN)Dest->r) / 3);
        Dest->g = Dest->r = Dest->b;
        Dest++;
      }
    }
  }

  return NewImage;
}

BOOLEAN
BigDiff (
  UINT8   a,
  UINT8   b
) {
  if (a > b) {
    if (!GlobalConfig.BackgroundDark) {
      return ((a - b) > (UINT8)(0xFF - GlobalConfig.BackgroundSharp));
    }
  } else if (GlobalConfig.BackgroundDark) {
    return ((b - a) > (UINT8)(0xFF - GlobalConfig.BackgroundSharp));
  }

  return 0;
}

//(c)Slice 2013
#define EDGE(P) \
do { \
  if (BigDiff (a11.P, a10.P)) { \
    if (!BigDiff (a11.P, a01.P) && !BigDiff (a11.P, a21.P)) { \
      a10.P = a11.P; \
    } else if (BigDiff (a11.P, a01.P)) { \
      if ((dx + dy) < cell) { \
        a11.P = a21.P = a12.P = (UINT8)((a10.P * (cell - dy + dx) + a01.P * (cell - dx + dy)) / (cell * 2)); \
      } else { \
        a10.P = a01.P = a11.P; \
      } \
    } else if (BigDiff (a11.P, a21.P)) { \
      if (dx > dy) { \
        a11.P = a01.P = a12.P = (UINT8)((a10.P * (cell * 2 - dy - dx) + a21.P * (dx + dy)) / (cell * 2)); \
      }else { \
        a10.P = a21.P = a11.P; \
      } \
    } \
  } else if (BigDiff (a11.P, a21.P)) { \
    if (!BigDiff (a11.P, a12.P)){ \
      a21.P = a11.P; \
    } else { \
      if ((dx + dy) > cell) { \
        a11.P = a01.P = a10.P = (UINT8)((a21.P * (cell + dx - dy) + a12.P * (cell - dx + dy)) / (cell * 2)); \
      } else { \
        a21.P = a12.P = a11.P; \
      } \
    } \
  } else if (BigDiff (a11.P, a01.P)) { \
    if (!BigDiff (a11.P, a12.P)){ \
      a01.P = a11.P; \
    } else { \
      if (dx < dy) { \
        a11.P = a21.P = a10.P = (UINT8)((a01.P * (cell * 2 - dx - dy) + a12.P * (dy + dx )) / (cell * 2)); \
      } else { \
        a01.P = a12.P = a11.P; \
      } \
    } \
  } else if (BigDiff (a11.P, a12.P)) { \
    a12.P = a11.P; \
  } \
} while (0)

#define SMOOTH(P) \
do { \
  norm = (INTN)a01.P + a10.P + 4 * a11.P + a12.P + a21.P; \
  if (norm == 0) { \
    Dest->P = 0; \
  } else { \
    Dest->P = (UINT8)(a11.P * 2 * (a01.P * (cell - dx) + a10.P * (cell - dy) + \
                      a21.P * dx + a12.P * dy + a11.P * 2 * cell) / (cell * norm)); \
  } \
} while (0)

#define SMOOTH2(P) \
do { \
     Dest->P = (UINT8)((a01.P * (cell - dx) * 3 + a10.P * (cell - dy) * 3 + \
                        a21.P * dx * 3 + a12.P * dy * 3 + a11.P * 2 * cell) / (cell * 8)); \
} while (0)

VOID
ScaleImage (
  OUT EG_IMAGE    *NewImage,
  IN EG_IMAGE     *OldImage
) {
  INTN        W1, W2, H1, H2, i, j, f, cell,
              x, dx, y, y1, dy; //, norm;
  EG_PIXEL    a10, a11, a12, a01, a21,
              *Src = OldImage->PixelData,
              *Dest = NewImage->PixelData;

  W1 = OldImage->Width;
  H1 = OldImage->Height;
  W2 = NewImage->Width;
  H2 = NewImage->Height;

  if (H1 * W2 < H2 * W1) {
    f = (H2 << 12) / H1;
  } else {
    f = (W2 << 12) / W1;
  }

  if (f == 0) {
   return;
  }

  cell = ((f - 1) >> 12) + 1;

  for (j = 0; j < H2; j++) {
    y = (j << 12) / f;
    y1 = y * W1;
    dy = j - ((y * f) >> 12);

    for (i = 0; i < W2; i++) {
      x = (i << 12) / f;
      dx = i - ((x * f) >> 12);
      a11 = Src[x + y1];
      a10 = (y == 0) ? a11 : Src[x + y1 - W1];
      a01 = (x == 0) ? a11 : Src[x + y1 - 1];
      a21 = (x >= W1) ? a11 : Src[x + y1 + 1];
      a12 = (y >= H1) ? a11 : Src[x + y1 + W1];

      if (a11.a == 0) {
        Dest->r = Dest->g = Dest->b = 0x55;
      } else {

        EDGE (r);
        EDGE (g);
        EDGE (b);

        SMOOTH2 (r);
        SMOOTH2 (g);
        SMOOTH2 (b);
      }

      Dest->a = 0xFF;
      Dest++;
    }
  }
}

VOID
FreeImage (
  IN EG_IMAGE   *Image
) {
  if (Image != NULL) {
    if (Image->PixelData != NULL) {
      FreePool (Image->PixelData);
      Image->PixelData = NULL; //FreePool will not zero pointer
    }
    FreePool (Image);
  }
}

//
// Loading images from files and embedded data
//

STATIC
EG_IMAGE *
DecodeAny (
  IN UINT8      *FileData,
  IN UINTN      FileDataLength,
  IN CHAR16     *Format,
  IN UINTN      IconSize,
  IN BOOLEAN    WantAlpha
) {
  EG_DECODE_FUNC    DecodeFunc;
  EG_IMAGE          *NewImage;

  if (Format) {
    // dispatch by extension
    DecodeFunc = NULL;

    if (StriCmp (Format, L"PNG") == 0) {
      //DBG ("decode format PNG\n");
      DecodeFunc = DecodePNG;
    } else if (StriCmp (Format, L"ICNS") == 0){
      //DBG ("decode format ICNS\n");
      DecodeFunc = DecodeICNS;
    } else  if (StriCmp (Format, L"BMP") == 0) {
      //DBG ("decode format BMP\n");
      DecodeFunc = DecodeBMP;
    }
    //  else if (StriCmp (Format, L"TGA") == 0)
    //    DecodeFunc = DecodeTGA;

    if (DecodeFunc == NULL) {
      return NULL;
    }
    //  DBG ("will decode data=%x len=%d icns=%d alpha=%c\n", FileData, FileDataLength, IconSize, WantAlpha?'Y':'N');

    // decode it
    NewImage = DecodeFunc (FileData, FileDataLength, IconSize, WantAlpha);
  } else {
    //automatic choose format
    NewImage = DecodePNG (FileData, FileDataLength, IconSize, WantAlpha);

    if (!NewImage) {
      //DBG (" ..png is wrong try to decode icns\n");
      NewImage = DecodeICNS (FileData, FileDataLength, IconSize, WantAlpha);
    } /* else {
      DBG (" ..decoded as png\n");
    } */

    if (!NewImage) {
      //DBG (" ..png and icns is wrong try to decode bmp\n");
      NewImage = DecodeBMP (FileData, FileDataLength, IconSize, WantAlpha);
    }
  }

//#if DEBUG_IMG == 2
//   PauseForKey (L"After DecodeAny\n");
//#endif

  return NewImage;
}

//caller is responsible for free image
EG_IMAGE *
LoadImage (
  IN EFI_FILE_HANDLE    BaseDir,
  IN CHAR16             *FileName,
  IN BOOLEAN            WantAlpha
) {
  EFI_STATUS    Status;
  UINT8         *FileData = NULL;
  UINTN         FileDataLength = 0;
  EG_IMAGE      *NewImage;

  if (BaseDir == NULL || FileName == NULL) {
    return NULL;
  }

  // load file
  Status = LoadFile (BaseDir, FileName, &FileData, &FileDataLength);
  //DBG ("File=%s loaded with status=%r length=%d\n", FileName, Status, FileDataLength);

  if (EFI_ERROR (Status)) {
    return NULL;
  }

  //DBG ("   extension = %s\n", FindExtension (FileName));
  // decode it
  //NewImage = DecodeAny (FileData, FileDataLength, NULL, /*FindExtension (FileName),*/ 128, WantAlpha);
  NewImage = DecodePNG (FileData, FileDataLength, 128, WantAlpha);
  //DBG ("decoded\n");

  if (!NewImage) {
    DBG ("%s not decoded\n", FileName);
  }

  FreePool (FileData);
  //DBG ("FreePool OK\n");

  return NewImage;
}

//caller is responsible for free image
EG_IMAGE *
LoadIcon (
  IN EFI_FILE_HANDLE  BaseDir,
  IN CHAR16           *FileName,
  IN UINTN            IconSize
) {
  EFI_STATUS    Status;
  UINT8         *FileData;
  UINTN         FileDataLength;
  EG_IMAGE      *NewImage;

  if (BaseDir == NULL || FileName == NULL || IsEmbeddedTheme ()) {
    return NULL;
  }

  //DBG ("egLoadIcon filename: %s\n", FileName);

  // load file
  Status = LoadFile (BaseDir, FileName, &FileData, &FileDataLength);

  if (EFI_ERROR (Status)) {
  //DBG ("egLoadIcon status=%r\n", Status);
    return NULL;
  }

  // decode it
  NewImage = DecodeAny (FileData, FileDataLength, NULL, /*FindExtension (FileName),*/ IconSize, TRUE);
  //NewImage = DecodePNG (FileData, FileDataLength, 128, TRUE);
  FreePool (FileData);

  return NewImage;
}

/*
EG_IMAGE *
DecodeImage (
  IN UINT8      *FileData,
  IN UINTN      FileDataLength,
  IN CHAR16     *Format,
  IN BOOLEAN    WantAlpha
) {
  return DecodeAny (FileData, FileDataLength, Format, 128, WantAlpha);
}
*/

EG_IMAGE *
PrepareEmbeddedImage (
  IN EG_EMBEDDED_IMAGE    *EmbeddedImage,
  IN BOOLEAN              WantAlpha
) {
  EG_IMAGE    *NewImage;
  UINT8       *CompData;
  UINTN       CompLen, PixelCount;

  // sanity check
  if (
    (EmbeddedImage->PixelMode > EG_MAX_EIPIXELMODE) ||
    (
      (EmbeddedImage->CompressMode != EG_EICOMPMODE_NONE) &&
      (EmbeddedImage->CompressMode != EG_EICOMPMODE_RLE))
  ) {
    return NULL;
  }

  // allocate image structure and pixel buffer
  NewImage = CreateImage (EmbeddedImage->Width, EmbeddedImage->Height, WantAlpha);

  if (NewImage == NULL) {
    return NULL;
  }

  CompData = (UINT8 *)EmbeddedImage->Data;   // drop const
  CompLen  = EmbeddedImage->DataLength;
  PixelCount = EmbeddedImage->Width * EmbeddedImage->Height;

  // FUTURE: for EG_EICOMPMODE_EFICOMPRESS, decompress whole data block here

  if (
    (EmbeddedImage->PixelMode == EG_EIPIXELMODE_GRAY) ||
    (EmbeddedImage->PixelMode == EG_EIPIXELMODE_GRAY_ALPHA)
  ) {
    // copy grayscale plane and expand
    if (EmbeddedImage->CompressMode == EG_EICOMPMODE_RLE) {
      DecompressIcnsRLE (&CompData, &CompLen, PLPTR (NewImage, r), PixelCount);
    } else {
      InsertPlane (CompData, PLPTR (NewImage, r), PixelCount);
      CompData += PixelCount;
    }

    CopyPlane (PLPTR (NewImage, r), PLPTR (NewImage, g), PixelCount);
    CopyPlane (PLPTR (NewImage, r), PLPTR (NewImage, b), PixelCount);
  } else if (
    (EmbeddedImage->PixelMode == EG_EIPIXELMODE_COLOR) ||
    (EmbeddedImage->PixelMode == EG_EIPIXELMODE_COLOR_ALPHA)
  ) {
    // copy color planes
    if (EmbeddedImage->CompressMode == EG_EICOMPMODE_RLE) {
      DecompressIcnsRLE (&CompData, &CompLen, PLPTR (NewImage, r), PixelCount);
      DecompressIcnsRLE (&CompData, &CompLen, PLPTR (NewImage, g), PixelCount);
      DecompressIcnsRLE (&CompData, &CompLen, PLPTR (NewImage, b), PixelCount);
    } else {
      InsertPlane (CompData, PLPTR (NewImage, r), PixelCount);
      CompData += PixelCount;
      InsertPlane (CompData, PLPTR (NewImage, g), PixelCount);
      CompData += PixelCount;
      InsertPlane (CompData, PLPTR (NewImage, b), PixelCount);
      CompData += PixelCount;
    }
  } else {
    // set color planes to black
    SetPlane (PLPTR (NewImage, r), 0, PixelCount);
    SetPlane (PLPTR (NewImage, g), 0, PixelCount);
    SetPlane (PLPTR (NewImage, b), 0, PixelCount);
  }

  if (
    WantAlpha &&
    (
      (EmbeddedImage->PixelMode == EG_EIPIXELMODE_GRAY_ALPHA) ||
      (EmbeddedImage->PixelMode == EG_EIPIXELMODE_COLOR_ALPHA) ||
      (EmbeddedImage->PixelMode == EG_EIPIXELMODE_ALPHA))
  ) {
    // copy alpha plane
    if (EmbeddedImage->CompressMode == EG_EICOMPMODE_RLE) {
      DecompressIcnsRLE (&CompData, &CompLen, PLPTR (NewImage, a), PixelCount);
    } else {
      InsertPlane (CompData, PLPTR (NewImage, a), PixelCount);
      //            CompData += PixelCount;
    }
  } else {
    SetPlane (PLPTR (NewImage, a), WantAlpha ? 255 : 0, PixelCount);
  }

  return NewImage;
}

//
// Compositing
//

VOID
RestrictImageArea (
  IN EG_IMAGE     *Image,
  IN INTN         AreaPosX,
  IN INTN         AreaPosY,
  IN OUT INTN     *AreaWidth,
  IN OUT INTN     *AreaHeight
) {
  if (!Image || !AreaWidth || !AreaHeight) {
    return;
  }

  if ((AreaPosX >= Image->Width) || (AreaPosY >= Image->Height)) {
    // out of bounds, operation has no effect
    *AreaWidth  = 0;
    *AreaHeight = 0;
  } else {
    // calculate affected area
    if (*AreaWidth > Image->Width - AreaPosX) {
      *AreaWidth = Image->Width - AreaPosX;
    }

    if (*AreaHeight > Image->Height - AreaPosY) {
      *AreaHeight = Image->Height - AreaPosY;
    }
  }
}

VOID
FillImage (
  IN OUT EG_IMAGE     *CompImage,
  IN EG_PIXEL         *Color
) {
  INTN        i;
  EG_PIXEL    FillColor, *PixelPtr;

  if (!CompImage || !Color) {
    return;
  }

  FillColor = *Color;

  if (!CompImage->HasAlpha) {
    FillColor.a = 0;
  }

  PixelPtr = CompImage->PixelData;
  for (i = 0; i < CompImage->Width * CompImage->Height; i++) {
    *PixelPtr++ = FillColor;
  }
}

VOID
FillImageArea (
  IN OUT EG_IMAGE   *CompImage,
  IN INTN           AreaPosX,
  IN INTN           AreaPosY,
  IN INTN           AreaWidth,
  IN INTN           AreaHeight,
  IN EG_PIXEL       *Color
) {
  INTN        x, y,
              xAreaWidth = AreaWidth,
              xAreaHeight = AreaHeight;
  EG_PIXEL    FillColor, *PixelBasePtr;

  if (!CompImage || !Color) {
    return;
  }

  RestrictImageArea (CompImage, AreaPosX, AreaPosY, &xAreaWidth, &xAreaHeight);

  if (xAreaWidth > 0) {
    FillColor = *Color;

    if (!CompImage->HasAlpha) {
      FillColor.a = 0;
    }

    PixelBasePtr = CompImage->PixelData + AreaPosY * CompImage->Width + AreaPosX;
    for (y = 0; y < xAreaHeight; y++) {
      EG_PIXEL    *PixelPtr = PixelBasePtr;

      for (x = 0; x < xAreaWidth; x++) {
        *PixelPtr++ = FillColor;
      }

      PixelBasePtr += CompImage->Width;
    }
  }
}

VOID
RawCopy (
  IN OUT EG_PIXEL   *CompBasePtr,
  IN EG_PIXEL       *TopBasePtr,
  IN INTN           Width,
  IN INTN           Height,
  IN INTN           CompLineOffset,
  IN INTN           TopLineOffset
) {
  INTN    x, y;

  if (!CompBasePtr || !TopBasePtr) {
    return;
  }

  for (y = 0; y < Height; y++) {
    EG_PIXEL    *TopPtr = TopBasePtr;
    EG_PIXEL    *CompPtr = CompBasePtr;

    for (x = 0; x < Width; x++) {
      *CompPtr = *TopPtr;
      TopPtr++, CompPtr++;
    }

    TopBasePtr += TopLineOffset;
    CompBasePtr += CompLineOffset;
  }
}

VOID
RawCompose (
  IN OUT EG_PIXEL   *CompBasePtr,
  IN EG_PIXEL       *TopBasePtr,
  IN INTN           Width,
  IN INTN           Height,
  IN INTN           CompLineOffset,
  IN INTN           TopLineOffset
) {
  INT64       x, y;
  EG_PIXEL    *TopPtr, *CompPtr;
  //To make native division we need INTN types
  INTN        TopAlpha, Alpha, CompAlpha, RevAlpha, Temp;
  //EG_PIXEL    *CompUp;

  if (!CompBasePtr || !TopBasePtr) {
    return;
  }

  //CompUp = CompBasePtr + Width * Height;
  //Slice - my opinion
  //if TopAlpha=255 then draw Top - non transparent
  //else if TopAlpha=0 then draw Comp - full transparent
  //else draw mixture |-----comp---|--top--|
  //final alpha =(1 - (1 - x) * (1 - y)) =(255 * 255 - (255 - topA) * (255 - compA))/255

  for (y = 0; y < Height; y++) {
    TopPtr = TopBasePtr;
    CompPtr = CompBasePtr;

    for (x = 0; x < Width; x++) {
      TopAlpha = TopPtr->a & 0xFF; //exclude sign
      CompAlpha = CompPtr->a & 0xFF;
      RevAlpha = 255 - TopAlpha;
      //Alpha = 255 * (UINT8)TopAlpha + (UINT8)CompPtr->a * (UINT8)RevAlpha;
      Alpha = (255 * 255 - (255 - TopAlpha) * (255 - CompAlpha)) / 255;

      if (TopAlpha == 0) {
        TopPtr++, CompPtr++; // no need to bother
        continue;
      }

      Temp = (TopPtr->b * TopAlpha) + (CompPtr->b * RevAlpha);
      CompPtr->b = (UINT8)(Temp / 255);

      Temp = (TopPtr->g * TopAlpha) + (CompPtr->g  * RevAlpha);
      CompPtr->g = (UINT8)(Temp / 255);

      Temp = (TopPtr->r * TopAlpha) + (CompPtr->r * RevAlpha);
      CompPtr->r = (UINT8)(Temp / 255);

      CompPtr->a = (UINT8)Alpha;

      TopPtr++, CompPtr++;
    }

    TopBasePtr += TopLineOffset;
    CompBasePtr += CompLineOffset;
  }
}

//
// This is simplified image composing on solid background. ComposeImage will decide which method to use
//
VOID
RawComposeOnFlat (
  IN OUT EG_PIXEL   *CompBasePtr,
  IN EG_PIXEL       *TopBasePtr,
  IN INTN           Width,
  IN INTN           Height,
  IN INTN           CompLineOffset,
  IN INTN           TopLineOffset
) {
  INT64       x, y;
  EG_PIXEL    *TopPtr, *CompPtr;
  UINT32      TopAlpha, RevAlpha;
  UINTN       Temp;

  if (!CompBasePtr || !TopBasePtr) {
    return;
  }

  for (y = 0; y < Height; y++) {
    TopPtr = TopBasePtr;
    CompPtr = CompBasePtr;
    for (x = 0; x < Width; x++) {
      TopAlpha = TopPtr->a;
      RevAlpha = 255 - TopAlpha;

      Temp = ((UINT8)CompPtr->b * RevAlpha) + ((UINT8)TopPtr->b * TopAlpha);
      CompPtr->b = (UINT8)(Temp / 255);

      Temp = ((UINT8)CompPtr->g * RevAlpha) + ((UINT8)TopPtr->g * TopAlpha);
      CompPtr->g = (UINT8)(Temp / 255);

      Temp = ((UINT8)CompPtr->r * RevAlpha) + ((UINT8)TopPtr->r * TopAlpha);
      CompPtr->r = (UINT8)(Temp / 255);

      CompPtr->a = (UINT8)(255);

      TopPtr++, CompPtr++;
    }

    TopBasePtr += TopLineOffset;
    CompBasePtr += CompLineOffset;
  }
}

VOID
ComposeImage (
  IN OUT EG_IMAGE   *CompImage,
  IN EG_IMAGE       *TopImage,
  IN INTN           PosX,
  IN INTN           PosY
) {
  INTN    CompWidth, CompHeight;

  if (!TopImage || !CompImage) {
    return;
  }

  CompWidth  = TopImage->Width;
  CompHeight = TopImage->Height;
  RestrictImageArea (CompImage, PosX, PosY, &CompWidth, &CompHeight);

  // compose
  if (CompWidth > 0) {
    //if (CompImage->HasAlpha && !BackgroundImage) {
    //  CompImage->HasAlpha = FALSE;
    //}

    if (TopImage->HasAlpha) {
      if (CompImage->HasAlpha) {
        RawCompose (
          CompImage->PixelData + PosY * CompImage->Width + PosX,
          TopImage->PixelData,
          CompWidth, CompHeight, CompImage->Width, TopImage->Width
        );
      } else {
        RawComposeOnFlat (
          CompImage->PixelData + PosY * CompImage->Width + PosX,
          TopImage->PixelData,
          CompWidth, CompHeight, CompImage->Width, TopImage->Width
        );
      }
    } else {
      RawCopy (
        CompImage->PixelData + PosY * CompImage->Width + PosX,
        TopImage->PixelData,
        CompWidth, CompHeight, CompImage->Width, TopImage->Width
      );
    }
  }
}

EG_IMAGE *
EnsureImageSize (
  IN EG_IMAGE   *Image,
  IN INTN       Width,
  IN INTN       Height,
  IN EG_PIXEL   *Color
) {
  EG_IMAGE    *NewImage;

  if (Image == NULL) {
    return NULL;
  }

  if ((Image->Width == Width) && (Image->Height == Height)) {
    return Image;
  }

  NewImage = CreateFilledImage (Width, Height, Image->HasAlpha, Color);

  if (NewImage == NULL) {
    FreeImage (Image);
    return NULL;
  }

  ComposeImage (NewImage, Image, 0, 0);
  FreeImage (Image);

  return NewImage;
}

//
// misc internal functions
//

VOID
InsertPlane (
  IN UINT8    *SrcDataPtr,
  IN UINT8    *DestPlanePtr,
  IN UINTN    PixelCount
) {
  UINTN   i;

  if (!SrcDataPtr || !DestPlanePtr) {
    return;
  }

  for (i = 0; i < PixelCount; i++) {
    *DestPlanePtr = *SrcDataPtr++;
    DestPlanePtr += 4;
  }
}

VOID
SetPlane (
  IN UINT8    *DestPlanePtr,
  IN UINT8    Value,
  IN UINT64   PixelCount
) {
  UINT64  i;

  if (!DestPlanePtr) {
    return;
  }

  for (i = 0; i < PixelCount; i++) {
    *DestPlanePtr = Value;
    DestPlanePtr += 4;
  }
}

VOID
CopyPlane (
  IN UINT8    *SrcPlanePtr,
  IN UINT8    *DestPlanePtr,
  IN UINTN    PixelCount
) {
  UINTN i;

  if (!SrcPlanePtr || !DestPlanePtr) {
    return;
  }

  for (i = 0; i < PixelCount; i++) {
    *DestPlanePtr = *SrcPlanePtr;
    DestPlanePtr += 4, SrcPlanePtr += 4;
  }
}

EG_IMAGE *
DecodePNG (
  IN UINT8      *FileData,
  IN UINTN      FileDataLength,
  IN UINTN      IconSize,
  IN BOOLEAN    WantAlpha
) {
  EG_IMAGE    *NewImage = NULL;
  EG_PIXEL    *PixelData, *Pixel, *PixelD;
  UINT32      PNG_error, Width, Height;
  INTN        i, ImageSize;

  PNG_error = lodepng_decode32 ((UINT8 **)&PixelData, &Width, &Height, (CONST UINT8 *)FileData, FileDataLength);

  if (PNG_error) {
    return NULL;
  }

  // allocate image structure and buffer
  NewImage = CreateImage ((INTN)Width, (INTN)Height, WantAlpha);
  if ((NewImage == NULL) || (NewImage->Width != (INTN)Width) || (NewImage->Height != (INTN)Height)) {
    return NULL;
  }

  ImageSize = (NewImage->Height * NewImage->Width);

  Pixel = (EG_PIXEL *)NewImage->PixelData;
  PixelD = PixelData;
  for (i = 0; i < ImageSize; i++, Pixel++, PixelD++) {
    Pixel->b = PixelD->r; //change r <-> b
    Pixel->r = PixelD->b;
    Pixel->g = PixelD->g;
    Pixel->a = PixelD->a; // 255 is opaque, 0 - transparent
  }

  lodepng_free (PixelData);

  return NewImage;
}

/* EOF */