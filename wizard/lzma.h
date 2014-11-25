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

  Wizard's Toolkit LZMA entropy methods.
*/
#ifndef _WIZARDSTOOLKIT_LZMA_H_
#define _WIZARDSTOOLKIT_LZMA_H_

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

typedef struct _LZMAInfo
  LZMAInfo;

extern WizardExport const StringInfo
  *GetLZMAChaos(const LZMAInfo *);

extern WizardExport WizardBooleanType
  IncreaseLZMA(LZMAInfo *,const StringInfo *,ExceptionInfo *),
  RestoreLZMA(LZMAInfo *,const size_t,const StringInfo *,ExceptionInfo *);

extern WizardExport LZMAInfo
  *AcquireLZMAInfo(const size_t),
  *DestroyLZMAInfo(LZMAInfo *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
