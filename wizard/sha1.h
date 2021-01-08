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
#ifndef _WIZARDSTOOLKIT_SHA1_H
#define _WIZARDSTOOLKIT_SHA1_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

typedef struct _SHA1Info
  SHA1Info;

extern WizardExport SHA1Info
  *AcquireSHA1Info(void),
  *DestroySHA1Info(SHA1Info *);

extern WizardExport const StringInfo
  *GetSHA1Digest(const SHA1Info *);

extern WizardExport unsigned int
  GetSHA1Blocksize(const SHA1Info *),
  GetSHA1Digestsize(const SHA1Info *);

extern WizardExport WizardBooleanType
  InitializeSHA1(SHA1Info *),
  FinalizeSHA1(SHA1Info *),
  UpdateSHA1(SHA1Info *,const StringInfo *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
