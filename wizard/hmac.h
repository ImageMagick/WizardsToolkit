/*
  Copyright 1999-2020 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.

  You may not use this file except in compliance with the License.
  obtain a copy of the License at

    https://imagemagick.org/script/license.php

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  Wizard's Toolkit secure hash algorithm methods.
*/
#ifndef _MAGICK_HMAC_H
#define _MAGICK_HMAC_H

#include "wizard/hash.h"

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

typedef struct _HMACInfo
  HMACInfo;

extern WizardExport HMACInfo
  *AcquireHMACInfo(const HashType),
  *DestroyHMACInfo(HMACInfo *);

extern WizardExport const StringInfo
  *GetHMACDigest(const HMACInfo *);

extern WizardExport size_t
  GetHMACDigestsize(const HMACInfo *);

extern WizardExport void
  ConstructHMAC(HMACInfo *,const StringInfo *,const StringInfo *),
  FinalizeHMAC(HMACInfo *),
  InitializeHMAC(HMACInfo *,const StringInfo *),
  ResetHMAC(HMACInfo *),
  UpdateHMAC(HMACInfo *,const StringInfo *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
