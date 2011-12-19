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
#ifndef _WIZARDSTOOLKIT_SHA384_H
#define _WIZARDSTOOLKIT_SHA384_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

typedef struct _SHA384Info
  SHA384Info;

extern WizardExport SHA384Info
  *AcquireSHA384Info(void),
  *DestroySHA384Info(SHA384Info *);

extern WizardExport const StringInfo
  *GetSHA384Digest(const SHA384Info *);

extern WizardExport unsigned int
  GetSHA384Blocksize(const SHA384Info *),
  GetSHA384Digestsize(const SHA384Info *);

extern WizardExport void
  InitializeSHA384(SHA384Info *),
  FinalizeSHA384(SHA384Info *),
  UpdateSHA384(SHA384Info *,const StringInfo *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
