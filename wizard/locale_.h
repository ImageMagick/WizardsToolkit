/*
  Copyright 1999-2015 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.
  
  You may not use this file except in compliance with the License.
  obtain a copy of the License at
  
    http://www.wizards-toolkit.org/script/license.php
  
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  WizardsToolkit locale methods.
*/
#ifndef _WIZARDSTOOLKIT_LOCALE_H
#define _WIZARDSTOOLKIT_LOCALE_H

#include "wizard/hashmap.h"

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

typedef struct _LocaleInfo
{
  char
    *path,
    *tag,
    *message;
                                                                                
  WizardBooleanType
    stealth;
                                                                                
  struct _LocaleInfo
    *previous,
    *next;  /* deprecated, use GetLocaleInfoList() */

  size_t
    signature;
} LocaleInfo;

extern WizardExport char
  **GetLocaleList(const char *,size_t *,ExceptionInfo *);

extern WizardExport const char
  *GetLocaleMessage(const char *);

extern WizardExport const LocaleInfo
  *GetLocaleInfo_(const char *,ExceptionInfo *),
  **GetLocaleInfoList(const char *,size_t *,ExceptionInfo *);

extern WizardExport double
  InterpretLocaleValue(const char *restrict,char **restrict);

extern WizardExport LinkedListInfo
  *DestroyLocaleOptions(LinkedListInfo *),
  *GetLocaleOptions(const char *,ExceptionInfo *);

extern WizardExport WizardBooleanType
  ListLocaleInfo(FILE *,ExceptionInfo *),
  LocaleComponentGenesis(void);

extern WizardExport ssize_t
  FormatLocaleFile(FILE *,const char *restrict,...)
    wizard_attribute((__format__ (__printf__,2,3))),
  FormatLocaleFileList(FILE *,const char *restrict,va_list)
    wizard_attribute((__format__ (__printf__,2,0))),
  FormatLocaleString(char *restrict,const size_t,const char *restrict,...)
    wizard_attribute((__format__ (__printf__,3,4))),
  FormatLocaleStringList(char *restrict,const size_t,const char *restrict,
    va_list) wizard_attribute((__format__ (__printf__,3,0)));

extern WizardExport void
  LocaleComponentTerminus(void);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
