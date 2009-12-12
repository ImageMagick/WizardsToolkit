/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                  SSSSS  EEEEE   CCCC  RRRR   EEEEE  TTTTT                   %
%                  SS     E      C      R   R  E        T                     %
%                   SSS   EEE    C      RRRR   EEE      T                     %
%                     SS  E      C      R R    E        T                     %
%                  SSSSS  EEEEE   CCCC  R  R   EEEEE    T                     %
%                                                                             %
%                                                                             %
%               Wizard's Toolkit Secret Authentication Methods                %
%                                                                             %
%                             Software Design                                 %
%                               John Cristy                                   %
%                               March 2003                                    %
%                                                                             %
%                                                                             %
%  Copyright 1999-2009 ImageMagick Studio LLC, a non-profit organization      %
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
#include "wizard/cipher.h"
#include "wizard/exception.h"
#include "wizard/exception-private.h"
#include "wizard/hmac.h"
#include "wizard/keyring.h"
#include "wizard/memory_.h"
#include "wizard/passphrase.h"
#include "wizard/random_.h"
#include "wizard/secret.h"

/*
  Define declarations.
*/
#define SecretKeyCipher  AESCipher
#define SecretKeyMode  CTRMode
#define SecretKeyHash  SHA256Hash
#define SecretRandomHash  SHA256Hash

/*
  Typedef declarations.
*/
struct _SecretInfo
{
  char
    *passphrase;

  StringInfo
    *id,
    *key,
    *digest,
    *nonce;

  size_t
    key_length;

  KeyringInfo
    *keyring_info;

  CipherInfo
    *cipher_info;

  HMACInfo
    *hmac_info;

  RandomInfo
    *random_info;

  time_t
    timestamp;

  unsigned long
    signature;
};

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   A c q u i r e S e c r e t I n f o                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquireSecretInfo() allocates the SecretInfo structure.
%
%  The format of the AcquireSecretInfo method is:
%
%      SecretInfo *AcquireSecretInfo(const char *path,const HashType hash,
%        const unsigned long key_length)
%
%  A description of each parameter follows:
%
%    o path: The secret keyring path.
%
%    o hash: The hash type.
%
%    o key_length: The key length in bits.
*/
WizardExport SecretInfo *AcquireSecretInfo(const char *path,const HashType hash,
  const unsigned long key_length)
{
  SecretInfo
    *secret_info;

  secret_info=(SecretInfo *) AcquireAlignedMemory(1,sizeof(*secret_info));
  if (secret_info == (SecretInfo *) NULL)
    ThrowWizardFatalError(AuthenticateDomain,MemoryError);
  (void) ResetWizardMemory(secret_info,0,sizeof(*secret_info));
  secret_info->key_length=key_length;
  secret_info->keyring_info=AcquireKeyringInfo(path);
  secret_info->cipher_info=AcquireCipherInfo(SecretKeyCipher,SecretKeyMode);
  secret_info->nonce=GenerateCipherNonce(secret_info->cipher_info);
  secret_info->hmac_info=AcquireHMACInfo(hash);
  secret_info->random_info=AcquireRandomInfo(SecretRandomHash);
  secret_info->timestamp=time((time_t *) NULL);
  secret_info->signature=WizardSignature;
  return(secret_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   A u t h e n t i c a t e S e c r e t K e y                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AuthenticateSecretKey() returns WizardTrue if the caller is authenticated.
%
%  The format of the AuthenticateSecretKey method is:
%
%      void AuthenticateSecretKey(SecretInfo *secret_info,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o secret_info: The secret info.
%
%    o exception: Return any errors or warnings in this structure.
%
*/
WizardExport WizardBooleanType AuthenticateSecretKey(SecretInfo *secret_info,
  ExceptionInfo *exception)
{
  WizardBooleanType
    status;

  StringInfo
    *digest,
    *phrase;

  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  WizardAssert(AuthenticateDomain,secret_info != (SecretInfo *) NULL);
  WizardAssert(AuthenticateDomain,secret_info->signature == WizardSignature);
  SetKeyringId(secret_info->keyring_info,secret_info->id);
  status=ExportKeyringKey(secret_info->keyring_info,exception);
  if (status == WizardFalse)
    return(WizardFalse);
  if (secret_info->passphrase == (char *) NULL)
    phrase=GetPassphrase(exception);
  else
    phrase=FileToStringInfo(secret_info->passphrase,~0,exception);
  if (phrase == (StringInfo *) NULL)
    return(WizardFalse);
  SetCipherKey(secret_info->cipher_info,phrase);
  SetCipherNonce(secret_info->cipher_info,GetKeyringNonce(
    secret_info->keyring_info));
  if (secret_info->key != (StringInfo *) NULL)
    secret_info->key=DestroyStringInfo(secret_info->key);
  secret_info->key=CloneStringInfo(GetKeyringKey(secret_info->keyring_info));
  (void) DecipherCipher(secret_info->cipher_info,secret_info->key);
  ConstructHMAC(secret_info->hmac_info,phrase,secret_info->key);
  phrase=DestroyStringInfo(phrase);
  digest=CloneStringInfo(GetHMACDigest(secret_info->hmac_info));
  if (CompareStringInfo(secret_info->id,digest) != 0)
    {
      digest=DestroyStringInfo(digest);
      return(WizardFalse);
    }
  digest=DestroyStringInfo(digest);
  return(WizardTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D e s t r o y S e c r e t I n f o                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DestroySecretInfo() zeros memory associated with the SecretInfo structure.
%
%  The format of the DestroySecretInfo method is:
%
%      SecretInfo *DestroySecretInfo(SecretInfo *secret_info)
%
%  A description of each parameter follows:
%
%    o secret_info: The secret info.
%
*/
WizardExport SecretInfo *DestroySecretInfo(SecretInfo *secret_info)
{
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  WizardAssert(AuthenticateDomain,secret_info != (SecretInfo *) NULL);
  WizardAssert(AuthenticateDomain,secret_info->signature == WizardSignature);
  if (secret_info->random_info != (RandomInfo *) NULL)
    secret_info->random_info=DestroyRandomInfo(secret_info->random_info);
  if (secret_info->hmac_info != (HMACInfo *) NULL)
    secret_info->hmac_info=DestroyHMACInfo(secret_info->hmac_info);
  if (secret_info->cipher_info != (CipherInfo *) NULL)
    secret_info->cipher_info=DestroyCipherInfo(secret_info->cipher_info);
  if (secret_info->keyring_info != (KeyringInfo *) NULL)
    secret_info->keyring_info=DestroyKeyringInfo(secret_info->keyring_info);
  if (secret_info->nonce != (StringInfo *) NULL)
    secret_info->nonce=DestroyStringInfo(secret_info->nonce);
  if (secret_info->key != (StringInfo *) NULL)
    secret_info->key=DestroyStringInfo(secret_info->key);
  if (secret_info->id != (StringInfo *) NULL)
    secret_info->id=DestroyStringInfo(secret_info->id);
  if (secret_info->digest != (StringInfo *) NULL)
    secret_info->digest=DestroyStringInfo(secret_info->digest);
  secret_info->signature=(~WizardSignature);
  secret_info=(SecretInfo *) RelinquishWizardMemory(secret_info);
  return(secret_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e n e r a t e S e c r e t K e y                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GenerateSecretKey() returns WizardTrue if a key is generated and successfully
%  added to the secret key ring.
%
%  The format of the GenerateSecretKey method is:
%
%      WizardBooleanType GenerateSecretKey(SecretInfo *secret_info,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o secret_info: The secret info.
%
%    o exception: Return any errors or warnings in this structure.
%
*/
WizardExport WizardBooleanType GenerateSecretKey(SecretInfo *secret_info,
  ExceptionInfo *exception)
{
  ExceptionInfo
    *sans;

  WizardBooleanType
    status;

  StringInfo
    *key,
    *phrase;

  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  WizardAssert(AuthenticateDomain,secret_info != (SecretInfo *) NULL);
  WizardAssert(AuthenticateDomain,secret_info->signature == WizardSignature);
  WizardAssert(AuthenticateDomain,exception != (ExceptionInfo *) NULL);
  if (secret_info->passphrase == (char *) NULL)
    phrase=GetPassphrase(exception);
  else
    phrase=FileToStringInfo(secret_info->passphrase,~0,exception);
  if (phrase == (StringInfo *) NULL)
    return(WizardFalse);
  sans=AcquireExceptionInfo();
  do
  {
    if (secret_info->key != (StringInfo *) NULL)
      secret_info->key=DestroyStringInfo(secret_info->key);
    secret_info->key=GetRandomKey(secret_info->random_info,
      secret_info->key_length/8);
    ConstructHMAC(secret_info->hmac_info,phrase,secret_info->key);
    if (secret_info->id != (StringInfo *) NULL)
      secret_info->id=DestroyStringInfo(secret_info->id);
    secret_info->id=CloneStringInfo(GetHMACDigest(secret_info->hmac_info));
    SetKeyringId(secret_info->keyring_info,secret_info->id);
    status=ExportKeyringKey(secret_info->keyring_info,sans);
  } while (status != WizardFalse);
  sans=DestroyExceptionInfo(sans);
  SetKeyringKey(secret_info->keyring_info,secret_info->key);
  SetKeyringNonce(secret_info->keyring_info,secret_info->nonce);
  SetCipherKey(secret_info->cipher_info,phrase);
  SetCipherNonce(secret_info->cipher_info,GetKeyringNonce(
    secret_info->keyring_info));
  key=CloneStringInfo(GetKeyringKey(secret_info->keyring_info));
  (void) EncipherCipher(secret_info->cipher_info,key);
  SetKeyringKey(secret_info->keyring_info,key);
  status=ImportKeyringKey(secret_info->keyring_info,exception);
  phrase=DestroyStringInfo(phrase);
  key=DestroyStringInfo(key);
  return(WizardTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t S e c r e t I d                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetSecretId() returns the secret id.
%
%  The format of the GetSecretId method is:
%
%      const StringInfo *GetSecretId(const SecretInfo *secret_info)
%
%  A description of each parameter follows:
%
%    o secret_info: The secret info.
%
*/
WizardExport const StringInfo *GetSecretId(const SecretInfo *secret_info)
{
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  WizardAssert(AuthenticateDomain,secret_info != (SecretInfo *) NULL);
  WizardAssert(AuthenticateDomain,secret_info->signature == WizardSignature);
  return(secret_info->id);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t S e c r e t K e y                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetSecretKey() returns the secret key.
%
%  The format of the GetSecretKey method is:
%
%      const StringInfo *GetSecretKey(const SecretInfo *secret_info)
%
%  A description of each parameter follows:
%
%    o secret_info: The secret info.
%
*/
WizardExport const StringInfo *GetSecretKey(const SecretInfo *secret_info)
{
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  WizardAssert(AuthenticateDomain,secret_info != (SecretInfo *) NULL);
  WizardAssert(AuthenticateDomain,secret_info->signature == WizardSignature);
  return(secret_info->key);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t S e c r e t K e y L e n g t h                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetSecretKeyLength() returns the secret key length.
%
%  The format of the GetSecretKeyLength method is:
%
%      size_t GetSecretKeyLength(const SecretInfo *secret_info)
%
%  A description of each parameter follows:
%
%    o secret_info: The secret info.
%
*/
WizardExport size_t GetSecretKeyLength(const SecretInfo *secret_info)
{
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  WizardAssert(AuthenticateDomain,secret_info != (SecretInfo *) NULL);
  WizardAssert(AuthenticateDomain,secret_info->signature == WizardSignature);
  return(secret_info->key_length);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t S e c r e t P a s s p h r a s e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetSecretPassphrase() returns the secret passphrase.
%
%  The format of the GetSecretPassphrase method is:
%
%      const char *GetSecretPassphrase(const SecretInfo *secret_info)
%
%  A description of each parameter follows:
%
%    o secret_info: The secret info.
%
*/
WizardExport const char *GetSecretPassphrase(const SecretInfo *secret_info)
{
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  WizardAssert(AuthenticateDomain,secret_info != (SecretInfo *) NULL);
  WizardAssert(AuthenticateDomain,secret_info->signature == WizardSignature);
  return(secret_info->passphrase);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t S e c r e t I d                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetSecretId() sets the secret id.
%
%  The format of the SetSecretId method is:
%
%      void SetSecretId(SecretInfo *secret_info,const StringInfo *id)
%
%  A description of each parameter follows:
%
%    o secret_info: The ring info.
%
%    o id: The id.
%
*/
WizardExport void SetSecretId(SecretInfo *secret_info,const StringInfo *id)
{
  if (secret_info->id != (StringInfo *) NULL)
    secret_info->id=DestroyStringInfo(secret_info->id);
  secret_info->id=CloneStringInfo(id);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t S e c r e t K e y L e n g t h                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetSecretKeyLength() sets the authentication method key length.
%
%  The format of the SetSecretKeyLength method is:
%
%      void SetSecretKeyLength(SecretInfo *secret_info,
%        const unsigned long key_length)
%
%  A description of each parameter follows:
%
%    o secret_info: The secret info.
%
%    o key_length: The key length in bits.
%
*/
WizardExport void SetSecretKeyLength(SecretInfo *secret_info,
  const unsigned long key_length)
{
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  WizardAssert(AuthenticateDomain,secret_info != (SecretInfo *) NULL);
  WizardAssert(AuthenticateDomain,secret_info->signature == WizardSignature);
  secret_info->key_length=key_length;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t S e c r e t P a s s p h r a s e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetSecretPassphrase() sets the secret passphrase.
%
%  The format of the SetSecretPassphrase method is:
%
%      void SetSecretPassphrase(SecretInfo *secret_info,const char *passphrase)
%
%  A description of each parameter follows:
%
%    o secret_info: The secret info.
%
%    o passphrase: The passphrase.
%
*/
WizardExport void SetSecretPassphrase(SecretInfo *secret_info,
  const char *passphrase)
{
  if (secret_info->passphrase != (char *) NULL)
    secret_info->passphrase=DestroyString(secret_info->passphrase);
  secret_info->passphrase=ConstantString(passphrase);
}
