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
#ifndef _WIZARDSTOOLKIT_SHA2224_H
#define _WIZARDSTOOLKIT_SHA2224_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

typedef struct _SHA2224Info
  SHA2224Info;

extern WizardExport SHA2224Info
  *AcquireSHA2224Info(void),
  *DestroySHA2224Info(SHA2224Info *);

extern WizardExport const StringInfo
  *GetSHA2224Digest(const SHA2224Info *);

extern WizardExport unsigned int
  GetSHA2224Blocksize(const SHA2224Info *),
  GetSHA2224Digestsize(const SHA2224Info *);

extern WizardExport void
  InitializeSHA2224(SHA2224Info *),
  FinalizeSHA2224(SHA2224Info *),
  UpdateSHA2224(SHA2224Info *,const StringInfo *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
