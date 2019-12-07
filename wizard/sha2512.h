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
#ifndef _WIZARDSTOOLKIT_SHA2512_H
#define _WIZARDSTOOLKIT_SHA2512_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

typedef struct _SHA2512Info
  SHA2512Info;

extern WizardExport SHA2512Info
  *AcquireSHA2512Info(void),
  *DestroySHA2512Info(SHA2512Info *);

extern WizardExport const StringInfo
  *GetSHA2512Digest(const SHA2512Info *);

extern WizardExport unsigned int
  GetSHA2512Blocksize(const SHA2512Info *),
  GetSHA2512Digestsize(const SHA2512Info *);

extern WizardExport WizardBooleanType
  InitializeSHA2512(SHA2512Info *),
  FinalizeSHA2512(SHA2512Info *),
  UpdateSHA2512(SHA2512Info *,const StringInfo *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
