/*
  Copyright 1999-2012 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.

  You may not use this file except in compliance with the License.
  obtain a copy of the License at

    http://www.wizards-toolkit.org/script/license.php

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  Wizard's Toolkit Environment entropy methods.
*/
#ifndef _WIZARDSTOOLKIT_ENTROPY_H_
#define _WIZARDSTOOLKIT_ENTROPY_H_

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

/*
  Enum declarations.
*/
typedef enum
{
  UndefinedEntropy,
  NoEntropy,
  ZIPEntropy,
  BZIPEntropy,
  LZMAEntropy
} EntropyType;

typedef struct _EntropyInfo
  EntropyInfo;

extern WizardExport const StringInfo
  *GetEntropyChaos(const EntropyInfo *);

extern WizardExport EntropyInfo
  *AcquireEntropyInfo(const EntropyType,const size_t),
  *DestroyEntropyInfo(EntropyInfo *);

extern WizardExport WizardBooleanType
  IncreaseEntropy(EntropyInfo *,const StringInfo *,ExceptionInfo *),
  RestoreEntropy(EntropyInfo *,const size_t,const StringInfo *,ExceptionInfo *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
