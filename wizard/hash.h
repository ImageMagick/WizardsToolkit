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

  Wizard's Toolkit secure hash algorithm methods.
*/
#ifndef _WIZARDSTOOLKIT_HASH_H
#define _WIZARDSTOOLKIT_HASH_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#include "wizard/string_.h"

typedef enum
{
  UndefinedHash,
  NoHash,
  CRC64Hash,
  MD5Hash,
  SHA1Hash,
  SHA224Hash,
  SHA256Hash,
  SHA384Hash,
  SHA512Hash
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

extern WizardExport void
  InitializeHash(HashInfo *),
  FinalizeHash(HashInfo *),
  UpdateHash(HashInfo *,const StringInfo *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
