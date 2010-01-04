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
#ifndef _WIZARDSTOOLKIT_SHA224_H
#define _WIZARDSTOOLKIT_SHA224_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

typedef struct _SHA224Info
  SHA224Info;

extern WizardExport SHA224Info
  *AcquireSHA224Info(void),
  *DestroySHA224Info(SHA224Info *);

extern WizardExport const StringInfo
  *GetSHA224Digest(const SHA224Info *);

extern WizardExport unsigned int
  GetSHA224Blocksize(const SHA224Info *),
  GetSHA224Digestsize(const SHA224Info *);

extern WizardExport void
  InitializeSHA224(SHA224Info *),
  FinalizeSHA224(SHA224Info *),
  UpdateSHA224(SHA224Info *,const StringInfo *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
