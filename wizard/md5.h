/*
  Copyright 1999-2018 ImageMagick Studio LLC, a non-profit organization
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
#ifndef _WIZARDSTOOLKIT_MD5_H
#define _WIZARDSTOOLKIT_MD5_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

typedef struct _MD5Info
 MD5Info;

extern WizardExport MD5Info
  *AcquireMD5Info(void),
  *DestroyMD5Info(MD5Info *);

extern WizardExport const StringInfo
  *GetMD5Digest(const MD5Info *);

extern WizardExport unsigned int
  GetMD5Blocksize(const MD5Info *),
  GetMD5Digestsize(const MD5Info *);

extern WizardExport WizardBooleanType
  InitializeMD5(MD5Info *),
  FinalizeMD5(MD5Info *),
  UpdateMD5(MD5Info *,const StringInfo *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
