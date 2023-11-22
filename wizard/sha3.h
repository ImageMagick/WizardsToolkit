/*
  Copyright @ 1999 ImageMagick Studio LLC, a non-profit organization
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
#ifndef _WIZARDSTOOLKIT_SHA3_H
#define _WIZARDSTOOLKIT_SHA3_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

typedef struct _SHA3Info
  SHA3Info;

extern WizardExport SHA3Info
  *AcquireSHA3Info(const HashType),
  *DestroySHA3Info(SHA3Info *);

extern WizardExport const StringInfo
  *GetSHA3Digest(const SHA3Info *);

extern WizardExport unsigned int
  GetSHA3Blocksize(const SHA3Info *),
  GetSHA3Digestsize(const SHA3Info *);

extern WizardExport WizardBooleanType
  InitializeSHA3(SHA3Info *),
  FinalizeSHA3(SHA3Info *),
  UpdateSHA3(SHA3Info *,const StringInfo *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
