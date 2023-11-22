/*
  Copyright @ 1999 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.

  You may not use this file except in compliance with the License.
  obtain a copy of the License at

    https://imagemagick.org/script/license.php

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  Wizard's Toolkit Environment BZIP entropy methods.
*/
#ifndef _WIZARDSTOOLKIT_BZIP_H_
#define _WIZARDSTOOLKIT_BZIP_H_

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

typedef struct _BZIPInfo
  BZIPInfo;

extern WizardExport BZIPInfo
  *AcquireBZIPInfo(const size_t),
  *DestroyBZIPInfo(BZIPInfo *);

extern WizardExport const StringInfo
  *GetBZIPChaos(const BZIPInfo *);

extern WizardExport WizardBooleanType
  IncreaseBZIP(BZIPInfo *,const StringInfo *,ExceptionInfo *),
  RestoreBZIP(BZIPInfo *,const size_t,const StringInfo *,ExceptionInfo *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
