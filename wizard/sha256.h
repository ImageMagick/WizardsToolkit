/*
  Copyright 1999-2010 ImageMagick Studio LLC, a non-profit organization
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
#ifndef _WIZARDSTOOLKIT_SHA256_H
#define _WIZARDSTOOLKIT_SHA256_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

typedef struct _SHA256Info
  SHA256Info;

extern WizardExport SHA256Info
  *AcquireSHA256Info(void),
  *DestroySHA256Info(SHA256Info *);

extern WizardExport const StringInfo
  *GetSHA256Digest(const SHA256Info *);

extern WizardExport unsigned int
  GetSHA256Blocksize(const SHA256Info *),
  GetSHA256Digestsize(const SHA256Info *);

extern WizardExport void
  InitializeSHA256(SHA256Info *),
  FinalizeSHA256(SHA256Info *),
  UpdateSHA256(SHA256Info *,const StringInfo *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
