/*
  Copyright @ 1999 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.

  You may not use this file except in compliance with the License.
  obtain a copy of the License at

    https://imagemagick.org/script/license.php

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  Wizard the License for the specific language governing permissions and
  limitations under the License.

  Wizard's Toolkit key management methods.
*/
#ifndef _WIZARDSTOOLKIT_KEY_H_
#define _WIZARDSTOOLKIT_KEY_H_

#include "wizard/cipher.h"
#include "wizard/random_.h"
#include "wizard/splay-tree.h"

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

typedef struct _KeyInfo
  KeyInfo;

extern WizardExport KeyInfo
  *AcquireKeyInfo(void),
  *DestroyKeyInfo(KeyInfo *);

extern WizardExport WizardBooleanType
  SetKeyInfo(KeyInfo *,const StringInfo *,const StringInfo *);

extern WizardExport StringInfo
  *GenerateSessionKey(KeyInfo *),
  *GetKeyInfo(KeyInfo *,const StringInfo *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
