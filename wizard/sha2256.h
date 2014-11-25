/*
  Copyright 1999-2015 ImageMagick Studio LLC, a non-profit organization
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
#ifndef _WIZARDSTOOLKIT_SHA2256_H
#define _WIZARDSTOOLKIT_SHA2256_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

typedef struct _SHA2256Info
  SHA2256Info;

extern WizardExport SHA2256Info
  *AcquireSHA2256Info(void),
  *DestroySHA2256Info(SHA2256Info *);

extern WizardExport const StringInfo
  *GetSHA2256Digest(const SHA2256Info *);

extern WizardExport unsigned int
  GetSHA2256Blocksize(const SHA2256Info *),
  GetSHA2256Digestsize(const SHA2256Info *);

extern WizardExport WizardBooleanType
  InitializeSHA2256(SHA2256Info *),
  FinalizeSHA2256(SHA2256Info *),
  UpdateSHA2256(SHA2256Info *,const StringInfo *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
