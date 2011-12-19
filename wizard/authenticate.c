/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%    AAA  U   U TTTTT H   H EEEEE N   N TTTTT IIIII  CCCC  AAA  TTTTT EEEEE   %
%   A   A U   U   T   H   H E     NN  N   T     I   C     A   A   T   E       %
%   AAAAA U   U   T   HHHHH EEE   N N N   T     I   C     AAAAA   T   EEE     %
%   A   A U   U   T   H   H E     N  NN   T     I   C     A   A   T   E       %
%   A   A  UUU    T   H   H EEEEE N   N   T   IIIII  CCCC A   A   T   EEEEE   %
%                                                                             %
%                                                                             %
%                   Wizard's Toolkit Authentication Methods                   %
%                                                                             %
%                             Software Design                                 %
%                               John Cristy                                   %
%                               March 2003                                    %
%                                                                             %
%                                                                             %
%  Copyright 1999-2012 ImageMagick Studio LLC, a non-profit organization      %
%  dedicated to making software imaging solutions freely available.           %
%                                                                             %
%  You may not use this file except in compliance with the License.  You may  %
%  obtain a copy of the License at                                            %
%                                                                             %
%    http://www.wizards-toolkit.org/script/license.php                        %
%                                                                             %
%  Unless required by applicable law or agreed to in writing, software        %
%  distributed under the License is distributed on an "AS IS" BASIS,          %
%  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   %
%  See the License for the specific language governing permissions and        %
%  limitations under the License.                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%
%
*/

/*
  Include declarations.
*/
#include "wizard/studio.h"
#include "wizard/authenticate.h"
#include "wizard/exception.h"
#include "wizard/exception-private.h"
#include "wizard/memory_.h"
#include "wizard/secret.h"

/*
  Define declarations.
*/
#define SecretKeyLength  1024

/*
  Typedef declarations.
*/
struct _AuthenticateInfo
{
  AuthenticateMethod
    method;

  void
    *handle;

  time_t
    timestamp;

  size_t
    signature;
};

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   A c q u i r e A u t h e n t i c a t e I n f o                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquireAuthenticateInfo() allocates the AuthenticateInfo structure.
%
%  The format of the AcquireAuthenticateInfo method is:
%
%      AuthenticateInfo *AcquireAuthenticateInfo(
%        const AuthenticateMethod method,const char *path,const HashType hash)
%
%  A description of each parameter follows:
%
%    o authenticate: The authenticate type.
%
%    o path: The keyring path.
%
%    o hash: The hash type.
%
*/
WizardExport AuthenticateInfo *AcquireAuthenticateInfo(
  const AuthenticateMethod method,const char *path,const HashType hash)
{
  AuthenticateInfo
    *authenticate_info;

  authenticate_info=(AuthenticateInfo *) AcquireWizardMemory(
    sizeof(*authenticate_info));
  if (authenticate_info == (AuthenticateInfo *) NULL)
    ThrowWizardFatalError(AuthenticateDomain,MemoryError);
  (void) ResetWizardMemory(authenticate_info,0,sizeof(*authenticate_info));
  authenticate_info->method=method;
  switch (method)
  {
    case SecretAuthenticateMethod:
    {
      authenticate_info->handle=(AuthenticateInfo *) AcquireSecretInfo(path,
        hash,SecretKeyLength);
      break;
    }
    default:
      ThrowWizardFatalError(AuthenticateDomain,EnumerateError);
  }
  authenticate_info->timestamp=time((time_t *) NULL);
  authenticate_info->signature=WizardSignature;
  return(authenticate_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   A u t h e n t i c a t e K e y                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AuthenticateKey() returns WizardTrue if the caller is authenticated.
%
%  The format of the AuthenticateKey method is:
%
%      void AuthenticateKey(AuthenticateInfo *authenticate_info,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o authenticate_info: The authenticate info.
%
%    o exception: Return any errors or warnings in this structure.
%
*/
WizardExport WizardBooleanType AuthenticateKey(
  AuthenticateInfo *authenticate_info,ExceptionInfo *exception)
{
  WizardBooleanType
    status;

  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  WizardAssert(AuthenticateDomain,
    authenticate_info != (AuthenticateInfo *) NULL);
  WizardAssert(AuthenticateDomain,
    authenticate_info->signature == WizardSignature);
  WizardAssert(AuthenticateDomain,
    authenticate_info->handle != (AuthenticateInfo *) NULL);
  status=WizardFalse;
  switch (authenticate_info->method)
  {
    case SecretAuthenticateMethod:
    {
      SecretInfo
        *secret_info;

      secret_info=(SecretInfo *) authenticate_info->handle;
      status=AuthenticateSecretKey(secret_info,exception);
      break;
    }
    default:
      ThrowWizardFatalError(AuthenticateDomain,EnumerateError);
  }
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D e s t r o y A u t h e n t i c a t e I n f o                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DestroyAuthenticateInfo() zeros memory associated with the AuthenticateInfo
%  structure.
%
%  The format of the DestroyAuthenticateInfo method is:
%
%      AuthenticateInfo *DestroyAuthenticateInfo(
%        AuthenticateInfo *authenticate_info)
%
%  A description of each parameter follows:
%
%    o authenticate_info: The authenticate info.
%
*/
WizardExport AuthenticateInfo *DestroyAuthenticateInfo(
  AuthenticateInfo *authenticate_info)
{
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  WizardAssert(AuthenticateDomain,
    authenticate_info != (AuthenticateInfo *) NULL);
  WizardAssert(AuthenticateDomain,
    authenticate_info->signature == WizardSignature);
  if (authenticate_info->handle != (AuthenticateInfo *) NULL)
    switch (authenticate_info->method)
    {
      case SecretAuthenticateMethod:
      {
        authenticate_info->handle=DestroySecretInfo((SecretInfo *)
          authenticate_info->handle);
        break;
      }
      default:
        ThrowWizardFatalError(AuthenticateDomain,EnumerateError);
    }
  authenticate_info->signature=(~WizardSignature);
  authenticate_info=(AuthenticateInfo *)
    RelinquishWizardMemory(authenticate_info);
  return(authenticate_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e n e r a t e A u t h e n t i c a t e K e y                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GenerateAuthenticateKey() returns WizardTrue if a secret key is generated and
%  successfully added to the secret key ring.
%
%  The format of the GenerateAuthenticateKey method is:
%
%      WizardBooleanType GenerateAuthenticateKey(
%        AuthenticateInfo *authenticate_info,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o authenticate_info: The authenticate info.
%
%    o exception: Return any errors or warnings in this structure.
%
*/
WizardExport WizardBooleanType GenerateAuthenticateKey(
  AuthenticateInfo *authenticate_info,ExceptionInfo *exception)
{
  WizardBooleanType
    status;

  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  WizardAssert(AuthenticateDomain,
    authenticate_info != (AuthenticateInfo *) NULL);
  WizardAssert(AuthenticateDomain,
    authenticate_info->signature == WizardSignature);
  WizardAssert(AuthenticateDomain,
    authenticate_info->handle != (AuthenticateInfo *) NULL);
  WizardAssert(AuthenticateDomain,exception != (ExceptionInfo *) NULL);
  status=WizardFalse;
  switch (authenticate_info->method)
  {
    case SecretAuthenticateMethod:
    {
      SecretInfo
        *secret_info;

      secret_info=(SecretInfo *) authenticate_info->handle;
      status=GenerateSecretKey(secret_info,exception);
      if (status == WizardFalse)
        break;
      break;
    }
    default:
      ThrowWizardFatalError(AuthenticateDomain,EnumerateError);
  }
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t A u t h e n t i c a t e I d                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetAuthenticateId() returns the authentication id.
%
%  The format of the GetAuthenticateId method is:
%
%      const StringInfo *GetAuthenticateId(
%        const AuthenticateInfo *authenticate_info)
%
%  A description of each parameter follows:
%
%    o authenticate_info: The authenticate info.
%
*/
WizardExport const StringInfo *GetAuthenticateId(
  const AuthenticateInfo *authenticate_info)
{
  const StringInfo
    *id;

  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  WizardAssert(AuthenticateDomain,
    authenticate_info != (AuthenticateInfo *) NULL);
  WizardAssert(AuthenticateDomain,
    authenticate_info->signature == WizardSignature);
  switch (authenticate_info->method)
  {
    case SecretAuthenticateMethod:
    {
      SecretInfo
        *secret_info;

      secret_info=(SecretInfo *) authenticate_info->handle;
      id=GetSecretId(secret_info);
      break;
    }
    default:
      ThrowWizardFatalError(AuthenticateDomain,EnumerateError);
  }
  return(id);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t A u t h e n t i c a t e K e y                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetAuthenticateKey() returns the authenticate key.
%
%  The format of the GetAuthenticateKey method is:
%
%      const StringInfo *GetAuthenticateKey(
%        const AuthenticateInfo *authenticate_info)
%
%  A description of each parameter follows:
%
%    o authenticate_info: The authenticate info.
%
*/
WizardExport const StringInfo *GetAuthenticateKey(
  const AuthenticateInfo *authenticate_info)
{
  const StringInfo
    *key;

  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  WizardAssert(AuthenticateDomain,
    authenticate_info != (AuthenticateInfo *) NULL);
  WizardAssert(AuthenticateDomain,
    authenticate_info->signature == WizardSignature);
  switch (authenticate_info->method)
  {
    case SecretAuthenticateMethod:
    {
      SecretInfo
        *secret_info;

      secret_info=(SecretInfo *) authenticate_info->handle;
      key=GetSecretKey(secret_info);
      break;
    }
    default:
      ThrowWizardFatalError(AuthenticateDomain,EnumerateError);
  }
  return(key);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t A u t h e n t i c a t e K e y L e n g t h                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetAuthenticateKeyLength() returns the authenticate key length.
%
%  The format of the GetAuthenticateKeyLength method is:
%
%      size_t GetAuthenticateKeyLength(
%        const AuthenticateInfo *authenticate_info)
%
%  A description of each parameter follows:
%
%    o authenticate_info: The authenticate info.
%
*/
WizardExport size_t GetAuthenticateKeyLength(
  const AuthenticateInfo *authenticate_info)
{
  size_t
    key_length;

  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  WizardAssert(AuthenticateDomain,
    authenticate_info != (AuthenticateInfo *) NULL);
  WizardAssert(AuthenticateDomain,
    authenticate_info->signature == WizardSignature);
  switch (authenticate_info->method)
  {
    case SecretAuthenticateMethod:
    {
      SecretInfo
        *secret_info;

      secret_info=(SecretInfo *) authenticate_info->handle;
      key_length=GetSecretKeyLength(secret_info);
      break;
    }
    default:
      ThrowWizardFatalError(AuthenticateDomain,EnumerateError);
  }
  return(key_length);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t A u t h e n t i c a t e P a s s p h r a s e                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetAuthenticatePassphrase() returns the authentication passphrase.
%
%  The format of the GetAuthenticatePassphrase method is:
%
%      const char *GetAuthenticatePassphrase(
%        const AuthenticateInfo *authenticate_info)
%
%  A description of each parameter follows:
%
%    o authenticate_info: The authenticate info.
%
*/
WizardExport const char *GetAuthenticatePassphrase(
  const AuthenticateInfo *authenticate_info)
{
  const char
    *passphrase;

  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  WizardAssert(AuthenticateDomain,
    authenticate_info != (AuthenticateInfo *) NULL);
  WizardAssert(AuthenticateDomain,
    authenticate_info->signature == WizardSignature);
  switch (authenticate_info->method)
  {
    case SecretAuthenticateMethod:
    {
      SecretInfo
        *secret_info;

      secret_info=(SecretInfo *) authenticate_info->handle;
      passphrase=GetSecretPassphrase(secret_info);
      break;
    }
    default:
      ThrowWizardFatalError(AuthenticateDomain,EnumerateError);
  }
  return(passphrase);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t A u t h e n t i c a t e I d                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetAuthenticateId() sets the authentication id.
%
%  The format of the SetAuthenticateId method is:
%
%      void SetAuthenticateId(AuthenticateInfo *authenticate_info,
%        const StringInfo *id)
%
%  A description of each parameter follows:
%
%    o authenticate_info: The authentication info.
%
%    o id: The id.
%
*/
WizardExport void SetAuthenticateId(AuthenticateInfo *authenticate_info,
  const StringInfo *id)
{
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  WizardAssert(AuthenticateDomain,
    authenticate_info != (AuthenticateInfo *) NULL);
  WizardAssert(AuthenticateDomain,
    authenticate_info->signature == WizardSignature);
  WizardAssert(AuthenticateDomain,
    authenticate_info->handle != (AuthenticateInfo *) NULL);
  switch (authenticate_info->method)
  {
    case SecretAuthenticateMethod:
    {
      SecretInfo
        *secret_info;

      secret_info=(SecretInfo *) authenticate_info->handle;
      SetSecretId(secret_info,id);
      break;
    }
    default:
      ThrowWizardFatalError(AuthenticateDomain,EnumerateError);
  }
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t A u t h e n t i c a t e K e y L e n g t h                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetAuthenticateKeyLength() sets the authentication method key length.
%
%  The format of the SetAuthenticateKeyLength method is:
%
%      void SetAuthenticateKeyLength(AuthenticateInfo *authenticate_info,
%        const size_t key_length)
%
%  A description of each parameter follows:
%
%    o authenticate_info: The authenticate info.
%
%    o key_length: The key length in bits.
%
*/
WizardExport void SetAuthenticateKeyLength(AuthenticateInfo *authenticate_info,
  const size_t key_length)
{
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  WizardAssert(AuthenticateDomain,
    authenticate_info != (AuthenticateInfo *) NULL);
  WizardAssert(AuthenticateDomain,
    authenticate_info->signature == WizardSignature);
  WizardAssert(AuthenticateDomain,
    authenticate_info->handle != (AuthenticateInfo *) NULL);
  switch (authenticate_info->method)
  {
    case SecretAuthenticateMethod:
    {
      SecretInfo
        *secret_info;

      secret_info=(SecretInfo *) authenticate_info->handle;
      SetSecretKeyLength(secret_info,key_length);
      break;
    }
    default:
      break;
  }
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t A u t h e n t i c a t e P a s s p h r a s e                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetAuthenticatePassphrase() sets the authentication method key file.
%
%  The format of the SetAuthenticatePassphrase method is:
%
%      void SetAuthenticatePassphrase(AuthenticateInfo *authenticate_info,
%        const char *passphrase)
%
%  A description of each parameter follows:
%
%    o authenticate_info: The authenticate info.
%
%    o passphrase: The passphrase.
%
*/
WizardExport void SetAuthenticatePassphrase(AuthenticateInfo *authenticate_info,
  const char *passphrase)
{
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  WizardAssert(AuthenticateDomain,
    authenticate_info != (AuthenticateInfo *) NULL);
  WizardAssert(AuthenticateDomain,
    authenticate_info->signature == WizardSignature);
  WizardAssert(AuthenticateDomain,
    authenticate_info->handle != (AuthenticateInfo *) NULL);
  switch (authenticate_info->method)
  {
    case SecretAuthenticateMethod:
    {
      SecretInfo
        *secret_info;

      secret_info=(SecretInfo *) authenticate_info->handle;
      SetSecretPassphrase(secret_info,passphrase);
      break;
    }
    default:
      break;
  }
}
