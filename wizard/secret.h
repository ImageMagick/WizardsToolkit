/*
  Copyright 1999-2014 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.

  You may not use this file except in compliance with the License.
  obtain a copy of the License at

    http://www.wizards-toolkit.org/script/license.php

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  Wizard's Toolkit Secret authentication methods.
*/
#ifndef _WIZARDSTOOLKIT_SECRET_H_
#define _WIZARDSTOOLKIT_SECRET_H_

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#include "wizard/cipher.h"
#include "wizard/exception.h"
#include "wizard/hash.h"
#include "wizard/hmac.h"
#include "wizard/keyring.h"
#include "wizard/random_.h"

typedef struct _SecretInfo
  SecretInfo;

extern WizardExport const char
  *GetSecretPassphrase(const SecretInfo *);

extern WizardExport const StringInfo
  *GetSecretId(const SecretInfo *),
  *GetSecretKey(const SecretInfo *);

extern WizardExport SecretInfo
  *AcquireSecretInfo(const char *,const HashType,const size_t),
  *DestroySecretInfo(SecretInfo *);

extern WizardExport size_t
  GetSecretKeyLength(const SecretInfo *);

extern WizardExport void
  SetSecretId(SecretInfo *,const StringInfo *),
  SetSecretKeyLength(SecretInfo *,const size_t),
  SetSecretPassphrase(SecretInfo *,const char *);

extern WizardExport WizardBooleanType
  AuthenticateSecretKey(SecretInfo *,ExceptionInfo *),
  GenerateSecretKey(SecretInfo *,ExceptionInfo *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
