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

  Wizard's Toolkit ZIP entropy methods.
*/
#ifndef _WIZARDSTOOLKIT_ZIP_H_
#define _WIZARDSTOOLKIT_ZIP_H_

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

typedef struct _ZIPInfo
  ZIPInfo;

extern WizardExport const StringInfo
  *GetZIPChaos(const ZIPInfo *);

extern WizardExport WizardBooleanType
  IncreaseZIP(ZIPInfo *,const StringInfo *,ExceptionInfo *),
  RestoreZIP(ZIPInfo *,const size_t,const StringInfo *,ExceptionInfo *);

extern WizardExport ZIPInfo
  *AcquireZIPInfo(const size_t),
  *DestroyZIPInfo(ZIPInfo *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
