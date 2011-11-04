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

  Wizard's Toolkit string methods.
*/
#ifndef _WIZARDSTOOLKIT_STRING_H_
#define _WIZARDSTOOLKIT_STRING_H_

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#include <stdarg.h>
#include "wizard/exception.h"

typedef struct _StringInfo
  StringInfo;

extern WizardExport char
  *AcquireString(const char *),
  *CloneString(char **,const char *),
  *ConstantString(const char *),
  *DestroyString(char *),
  **DestroyStringList(char **),
  *FileToString(const char *,const size_t,ExceptionInfo *),
  *GetEnvironmentValue(const char *),
  *StringInfoToHexString(const StringInfo *),
  *StringInfoToString(const StringInfo *),
  **StringToArgv(const char *,int *);

extern WizardExport const char
  *GetStringInfoPath(const StringInfo *);

extern WizardExport double
  InterpretSiPrefixValue(const char *restrict,char **restrict);

extern WizardExport int
  CompareStringInfo(const StringInfo *,const StringInfo *),
  LocaleCompare(const char *,const char *),
  LocaleNCompare(const char *,const char *,const size_t);

extern WizardExport size_t
  ConcatenateWizardString(char *,const char *,const size_t)
    wizard_attribute((nonnull)),
  CopyWizardString(char *,const char *,const size_t)
    wizard_attribute((nonnull));

extern WizardExport ssize_t
  FormatWizardSize(const WizardSizeType,const WizardBooleanType,char *),
  FormatLocaleString(char *,const size_t,const char *,...)
    wizard_attribute((format (printf,3,4))),
  FormatLocaleStringList(char *,const size_t,const char *,va_list),
  PrintWizardString(FILE *,const char *,...)
    wizard_attribute((format (printf,2,3))),
  FormatWizardTime(const time_t,const size_t,char *);

extern WizardExport size_t
  GetStringInfoLength(const StringInfo *);

extern WizardExport StringInfo
  *AcquireStringInfo(const size_t),
  *BlobToStringInfo(const void *,const size_t),
  *CloneStringInfo(const StringInfo *),
  *ConfigureFileToStringInfo(const char *),
  *DestroyStringInfo(StringInfo *),
  *FileToStringInfo(const char *,const size_t,ExceptionInfo *),
  *HexStringToStringInfo(const char *),
  *SplitStringInfo(StringInfo *,const size_t),
  *StringToStringInfo(const char *);

extern WizardExport unsigned char
  *GetStringInfoDatum(const StringInfo *);

extern WizardExport WizardBooleanType
  ConcatenateString(char **,const char *) wizard_attribute((nonnull)),
  SubstituteString(char **,const char *,const char *);

extern WizardExport WizardSizeType
  GetStringInfoCRC(const StringInfo *);

extern WizardExport void
  ConcatenateStringInfo(StringInfo *,const StringInfo *)
    wizard_attribute((nonnull)),
  LocaleLower(char *),
  PrintStringInfo(FILE *,const char *,const StringInfo *),
  ResetStringInfo(StringInfo *),
  SetStringInfoDatum(StringInfo *,const unsigned char *),
  SetStringInfo(StringInfo *,const StringInfo *),
  SetStringInfoLength(StringInfo *,const size_t),
  SetStringInfoPath(StringInfo *,const char *),
  StripString(char *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
