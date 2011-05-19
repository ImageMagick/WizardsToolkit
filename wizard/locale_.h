/*
  Copyright 1999-2011 ImageWizard Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.
  
  You may not use this file except in compliance with the License.
  obtain a copy of the License at
  
    http://www.imagewizard.org/script/license.php
  
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  WizardCore locale methods.
*/
#ifndef _MAGICKCORE_LOCALE_H
#define _MAGICKCORE_LOCALE_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#include "wizard/hashmap.h"

extern WizardExport double
  InterpretLocaleValue(const char *,char **);

extern WizardExport WizardBooleanType
  LocaleComponentGenesis(void);

extern WizardExport ssize_t
  FormatLocaleFile(FILE *,const char *,...)
    wizard_attribute((format (printf,2,3))),
  FormatLocaleFileList(FILE *,const char *,va_list)
    wizard_attribute((format (printf,2,0))),
  FormatLocaleString(char *,const size_t,const char *,...)
    wizard_attribute((format (printf,3,4))),
  FormatLocaleStringList(char *,const size_t,const char *,va_list)
    wizard_attribute((format (printf,3,0)));

extern WizardExport void
  LocaleComponentTerminus(void);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
