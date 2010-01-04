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

  Wizard's Toolkit authentication methods.
*/
#ifndef _WIZARDSTOOLKIT_AUTHENTICATE_H_
#define _WIZARDSTOOLKIT_AUTHENTICATE_H_

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#include "wizard/exception.h"
#include "wizard/hash.h"
#include "wizard/string_.h"

/*
  Enum declarations.
*/
typedef enum
{
  UndefinedAuthenticate,
  SecretAuthenticateMethod,
  PublicAuthenticateMethod
} AuthenticateMethod;

typedef struct _AuthenticateInfo
  AuthenticateInfo;

extern WizardExport AuthenticateInfo
  *AcquireAuthenticateInfo(const AuthenticateMethod,const char *,
    const HashType),
  *DestroyAuthenticateInfo(AuthenticateInfo *);

extern WizardExport const char
  *GetAuthenticatePassphrase(const AuthenticateInfo *);

extern WizardExport const StringInfo
  *GetAuthenticateId(const AuthenticateInfo *),
  *GetAuthenticateKey(const AuthenticateInfo *);

extern WizardExport size_t
  GetAuthenticateKeyLength(const AuthenticateInfo *);

extern WizardExport void
  SetAuthenticateId(AuthenticateInfo *,const StringInfo *),
  SetAuthenticateKeyLength(AuthenticateInfo *,const unsigned long),
  SetAuthenticatePassphrase(AuthenticateInfo *,const char *);

extern WizardExport WizardBooleanType
  AuthenticateKey(AuthenticateInfo *,ExceptionInfo *),
  GenerateAuthenticateKey(AuthenticateInfo *,ExceptionInfo *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
