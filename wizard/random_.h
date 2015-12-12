/*
  Copyright 1999-2016 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.
  
  You may not use this file except in compliance with the License.
  obtain a copy of the License at
  
    http://www.wizards-toolkit.org/script/license.php
  
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  Wizard's Toolkit random number methods.
*/
#ifndef _WIZARDSTOOLKIT_RANDOM_H
#define _WIZARDSTOOLKIT_RANDOM_H

#include "wizard/hmac.h"

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

/*
  Typedef declarations.
*/
typedef struct _RandomInfo
  RandomInfo;

/*
  Method declarations.
*/
extern WizardExport double
  GetPseudoRandomValue(RandomInfo *),
  GetRandomValue(RandomInfo *);

extern WizardExport RandomInfo
  *AcquireRandomInfo(const HashType),
  *DestroyRandomInfo(RandomInfo *);

extern WizardExport StringInfo
  *GetRandomKey(RandomInfo *,const size_t);

extern WizardExport unsigned long
  GetRandomSecretKey(const RandomInfo *);

extern WizardExport void
  RandomComponentTerminus(void),
  SetRandomKey(RandomInfo *,const size_t,unsigned char *),
  SetRandomSecretKey(const unsigned long),
  SetRandomTrueRandom(const WizardBooleanType);

extern WizardExport WizardBooleanType
  RandomComponentGenesis(void);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
