/*
  Copyright @ 1999 ImageMagick Studio LLC, a non-profit organization
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
#ifndef _WIZARDSTOOLKIT_BLOB_PRIVATE_H
#define _WIZARDSTOOLKIT_BLOB_PRIVATE_H

#include <wizard/blob.h>

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#if defined(WIZARDSTOOLKIT_HAVE_FSEEKO)
# define fseek  fseeko
# define ftell  ftello
#endif

typedef enum
{
  UndefinedBlobMode,
  ReadBlobMode,
  ReadBinaryBlobMode,
  WriteBlobMode,
  WriteBinaryBlobMode
} BlobMode;

extern WizardExport BlobInfo
  *DestroyBlob(BlobInfo *),
  *OpenBlob(const char *,const BlobMode,const WizardBooleanType,
    ExceptionInfo *);

extern WizardExport const char
  *GetBlobFilename(const BlobInfo *);

extern WizardExport const struct stat
  *GetBlobProperties(const BlobInfo *);

extern WizardExport int
  EOFBlob(BlobInfo *),
  ReadBlobByte(BlobInfo *),
  SyncBlob(BlobInfo *);

extern WizardExport ssize_t
  ReadBlob(BlobInfo *,const size_t,void *),
  ReadBlobChunk(BlobInfo *,const size_t,void *),
  WriteBlob(BlobInfo *,const size_t,const void *),
  WriteBlobByte(BlobInfo *,const unsigned char),
  WriteBlobChunk(BlobInfo *,const size_t,const void *),
  WriteBlobString(BlobInfo *,const char *);

extern WizardExport WizardBooleanType
  CloseBlob(BlobInfo *),
  SetBlobExtent(BlobInfo *,const WizardSizeType);

extern WizardExport WizardOffsetType
  TellBlob(const BlobInfo *);

extern WizardExport WizardSizeType
  GetBlobSize(BlobInfo *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
