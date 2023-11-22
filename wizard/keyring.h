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

  Wizard's Toolkit secret key ring methods.
*/
#ifndef _WIZARDSTOOLKIT_RING_H_
#define _WIZARDSTOOLKIT_RING_H_

#include <wizard/blob.h>
#include <wizard/exception.h>
#include <wizard/string_.h>

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

typedef struct _KeyringInfo
  KeyringInfo;

extern WizardExport const StringInfo
  *GetKeyringKey(const KeyringInfo *),
  *GetKeyringNonce(const KeyringInfo *);

extern WizardExport KeyringInfo
  *AcquireKeyringInfo(const char *),
  *DestroyKeyringInfo(KeyringInfo *);

extern WizardExport void
  SetKeyringId(KeyringInfo *,const StringInfo *),
  SetKeyringKey(KeyringInfo *,const StringInfo *),
  SetKeyringNonce(KeyringInfo *,const StringInfo *),
  SetKeyringPath(KeyringInfo *,const char *);

extern WizardExport WizardBooleanType
  ExportKeyringKey(KeyringInfo *,ExceptionInfo *),
  ImportKeyringKey(KeyringInfo *,ExceptionInfo *),
  PrintKeyringProperties(const char *,BlobInfo *,ExceptionInfo *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
