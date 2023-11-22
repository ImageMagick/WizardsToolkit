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

  ImageMagick configure methods.
*/
#ifndef _WIZARDSTOOLKIT_CONFIGURE_H
#define _WIZARDSTOOLKIT_CONFIGURE_H

#include "wizard/hashmap.h"

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

typedef struct _ConfigureInfo
{
  char
    *path,
    *name,
    *value;
                                                                                
  WizardBooleanType
    exempt,
    stealth;
                                                                                
  size_t
    signature;
} ConfigureInfo;

extern WizardExport char
  **GetConfigureList(const char *,size_t *,ExceptionInfo *);

extern WizardExport const char
  *GetConfigureValue(const ConfigureInfo *);

extern WizardExport const ConfigureInfo
  *GetConfigureInfo(const char *,ExceptionInfo *),
  **GetConfigureInfoList(const char *,size_t *,ExceptionInfo *);

extern WizardExport LinkedListInfo
  *DestroyConfigureOptions(LinkedListInfo *),
  *GetConfigurePaths(const char *,ExceptionInfo *),
  *GetConfigureOptions(const char *,ExceptionInfo *);

extern WizardExport WizardBooleanType
  ConfigureComponentGenesis(void),
  ListConfigureInfo(FILE *,ExceptionInfo *);

extern WizardExport void
  ConfigureComponentTerminus(void);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
