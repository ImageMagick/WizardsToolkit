/*
  Copyright 1999-2011 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.

  You may not use this file except in compliance with the License.
  obtain a copy of the License at

    http://www.wizards-toolkit.org/script/license.php

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  Wizard's Toolkit file methods.
*/
#ifndef _WIZARDSTOOLKIT_FILE_H_
#define _WIZARDSTOOLKIT_FILE_H_

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#include "wizard/exception.h"

typedef enum
{
  ReadFileMode,
  WriteFileMode,
} FileMode;

typedef struct _FileInfo
  FileInfo;

extern WizardExport const char
  *GetFilePath(const FileInfo *);

extern WizardExport const struct stat
  *GetFileProperties(const FileInfo *);

extern WizardExport FileInfo
  *AcquireFileInfo(const char *,const char *,const FileMode mode,
    ExceptionInfo *),
  *DestroyFileInfo(FileInfo *,ExceptionInfo *);

extern WizardExport int
  GetFileDescriptor(const FileInfo *),
  ReadFileByte(FileInfo *);

extern WizardExport WizardBooleanType
  DestroyFile(FileInfo *,ExceptionInfo *),
  ReadFile16Bits(FileInfo *,unsigned short *),
  ReadFile32Bits(FileInfo *,size_t *),
  ReadFile64Bits(FileInfo *,WizardSizeType *),
  ReadFileChunk(FileInfo *,void *,const size_t),
  WriteFileChunk(FileInfo *,const void *,const size_t),
  WriteFile16Bits(FileInfo *,const unsigned short),
  WriteFile32Bits(FileInfo *,const size_t),
  WriteFile64Bits(FileInfo *,const WizardSizeType);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
