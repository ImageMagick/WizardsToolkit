/*
  Copyright 1999-2021 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.

  You may not use this file except in compliance with the License.
  obtain a copy of the License at

    https://imagemagick.org/script/license.php

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  Wizard's Toolkit Binary Large OBjects Methods.
*/
#ifndef _WIZARDSTOOLKIT_BLOB_H
#define _WIZARDSTOOLKIT_BLOB_H

#include "wizard/exception.h"

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#if !defined(WizardMaxBufferExtent)
#define WizardMaxBufferExtent  8192
#endif

typedef enum
{
  ReadMode,
  WriteMode,
  IOMode
} MapMode;

typedef struct _BlobInfo
  BlobInfo;

extern WizardExport void
  *FileToBlob(const char *,const size_t,size_t *,ExceptionInfo *),
  *MapBlob(int,const MapMode,const WizardOffsetType,const size_t);

extern WizardExport WizardBooleanType
  UnmapBlob(void *,const size_t);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
