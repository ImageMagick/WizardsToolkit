/*
  Copyright 1999-2021 ImageMagick Studio LLC, a non-profit organization
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
#ifndef _WIZARDSTOOLKIT_SHA2384_H
#define _WIZARDSTOOLKIT_SHA2384_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

typedef struct _SHA2384Info
  SHA2384Info;

extern WizardExport SHA2384Info
  *AcquireSHA2384Info(void),
  *DestroySHA2384Info(SHA2384Info *);

extern WizardExport const StringInfo
  *GetSHA2384Digest(const SHA2384Info *);

extern WizardExport unsigned int
  GetSHA2384Blocksize(const SHA2384Info *),
  GetSHA2384Digestsize(const SHA2384Info *);

extern WizardExport WizardBooleanType
  InitializeSHA2384(SHA2384Info *),
  FinalizeSHA2384(SHA2384Info *),
  UpdateSHA2384(SHA2384Info *,const StringInfo *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
