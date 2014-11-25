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

  Wizard's Toolkit cyclic redundancy checksum methods.
*/
#ifndef _WIZARDSTOOLKIT_CRC64_H
#define _WIZARDSTOOLKIT_CRC64_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

typedef struct _CRC64Info
  CRC64Info;

extern WizardExport CRC64Info
  *AcquireCRC64Info(void),
  *DestroyCRC64Info(CRC64Info *);

extern WizardExport const StringInfo
  *GetCRC64Digest(const CRC64Info *);

extern WizardExport unsigned int
  GetCRC64Blocksize(const CRC64Info *),
  GetCRC64Digestsize(const CRC64Info *);

extern WizardExport WizardBooleanType
  InitializeCRC64(CRC64Info *),
  FinalizeCRC64(CRC64Info *),
  UpdateCRC64(CRC64Info *,const StringInfo *);

extern WizardExport WizardSizeType
  GetCRC64CyclicRedundancyCheck(const CRC64Info *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
