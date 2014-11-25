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

  Wizard's Toolkit Advanced Encipherion Standard cipher methods.
*/
#ifndef _WIZARDSTOOLKIT_AES_H
#define _WIZARDSTOOLKIT_AES_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

typedef struct _AESInfo
  AESInfo;

extern WizardExport AESInfo
  *AcquireAESInfo(void),
  *DestroyAESInfo(AESInfo *);

extern WizardExport unsigned int
  GetAESBlocksize(const AESInfo *);

extern WizardExport void
  DecipherAESBlock(AESInfo *,const unsigned char *,unsigned char *),
  EncipherAESBlock(AESInfo *,const unsigned char *,unsigned char *),
  SetAESKey(AESInfo *,const StringInfo *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
