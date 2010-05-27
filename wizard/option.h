/*
  Copyright 1999-2010 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.

  You may not use this file except in compliance with the License.
  obtain a copy of the License at

    http://www.wizards-toolkit.org/script/license.php

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  Wizard's Toolkit option methods.
*/
#ifndef _WIZARDSTOOLKIT_OPTION_H
#define _WIZARDSTOOLKIT_OPTION_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#include "wizard/hash.h"
#include "wizard/log.h"

typedef struct _OptionInfo
{
  const char
    *mnemonic;

  ssize_t
    type;
} OptionInfo;

typedef enum
{
  WizardUndefinedOptions = -1,
  WizardAuthenticateOptions = 0,
  WizardCipherOptions,
  WizardCommandOptions,
  WizardDataTypeOptions,
  WizardDebugOptions,
  WizardEndianOptions,
  WizardEntropyOptions,
  WizardEntropyLevelOptions,
  WizardKeyLengthOptions,
  WizardHashOptions,
  WizardListOptions,
  WizardModeOptions,
  WizardLogEventOptions,
  WizardResourceOptions
} WizardOption;

extern WizardExport char
  **GetWizardOptions(const WizardOption);

extern WizardExport const char
  *WizardOptionToMnemonic(const WizardOption,const ssize_t);

extern WizardExport ssize_t
  ParseWizardOption(const WizardOption,const WizardBooleanType,const char *);

extern WizardExport WizardBooleanType
  IsWizardOption(const char *),
  ListWizardOptions(FILE *,const WizardOption,ExceptionInfo *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
