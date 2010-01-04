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

  Wizard's Toolkit Serpent cipher methods.
*/
#ifndef _WIZARDSTOOLKIT_SERPENT_H
#define _WIZARDSTOOLKIT_SERPENT_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

typedef struct _SerpentInfo
  SerpentInfo;

extern WizardExport SerpentInfo
  *AcquireSerpentInfo(void),
  *DestroySerpentInfo(SerpentInfo *);

extern WizardExport unsigned int
  GetSerpentBlocksize(const SerpentInfo *);

extern WizardExport void
  DecipherSerpentBlock(SerpentInfo *,const unsigned char *,unsigned char *),
  EncipherSerpentBlock(SerpentInfo *,const unsigned char *,unsigned char *),
  SetSerpentKey(SerpentInfo *,const StringInfo *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
