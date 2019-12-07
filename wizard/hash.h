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
#ifndef _WIZARDSTOOLKIT_HASH_H
#define _WIZARDSTOOLKIT_HASH_H

#include "wizard/string_.h"

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

typedef enum
{
  UndefinedHash,
  NoHash,
  CRC64Hash,
  MD5Hash,
  SHA1Hash,
  SHA2Hash,
  SHA2224Hash,
  SHA2256Hash,
  SHA2384Hash,
  SHA2512Hash,
  SHA3Hash,
  SHA3224Hash,
  SHA3256Hash,
  SHA3384Hash,
  SHA3512Hash
} HashType;

typedef struct _HashInfo
  HashInfo;

extern WizardExport char
  *GetHashHexDigest(const HashInfo *);

extern WizardExport const StringInfo
  *GetHashDigest(const HashInfo *);

extern WizardExport HashInfo
  *DestroyHashInfo(HashInfo *),
  *AcquireHashInfo(const HashType);

extern WizardExport size_t
  GetHashBlocksize(const HashInfo *),
  GetHashDigestsize(const HashInfo *);

extern WizardExport WizardBooleanType
  InitializeHash(HashInfo *),
  FinalizeHash(HashInfo *),
  UpdateHash(HashInfo *,const StringInfo *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
