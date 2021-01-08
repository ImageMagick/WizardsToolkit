/*
  Copyright 1999-2021 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.

  You may not use this file except in compliance with the License.
  obtain a copy of the License at

    https://imagemagick.org/script/license.php

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  Wizard's Toolkit Chacha cipher methods.
*/
#ifndef _WIZARDSTOOLKIT_CHACHA_H
#define _WIZARDSTOOLKIT_CHACHA_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

typedef struct _ChachaInfo
  ChachaInfo;

extern WizardExport ChachaInfo
  *AcquireChachaInfo(void),
  *DestroyChachaInfo(ChachaInfo *);

extern WizardExport unsigned int
  GetChachaBlocksize(const ChachaInfo *);

extern WizardExport void
  DecipherChachaBlock(ChachaInfo *,const unsigned char *,unsigned char *),
  EncipherChachaBlock(ChachaInfo *,const unsigned char *,unsigned char *),
  SetChachaKey(ChachaInfo *,const StringInfo *),
  SetChachaNonce(ChachaInfo *,const unsigned char *,const unsigned char *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
