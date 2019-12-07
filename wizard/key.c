/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                             K   K  EEEEE  Y   Y                             %
%                             K  K   E      Y   Y                             %
%                             KKK    EEE     Y Y                              %
%                             K  K   E        Y                               %
%                             K   K  EEEEE    Y                               %
%                                                                             %
%                                                                             %
%                          Wizard's Toolkit Key Methods                       %
%                                                                             %
%                             Software Design                                 %
%                                 Cristy                                      %
%                               March 2003                                    %
%                                                                             %
%                                                                             %
%  Copyright 1999-2020 ImageMagick Studio LLC, a non-profit organization      %
%  dedicated to making software imaging solutions freely available.           %
%                                                                             %
%  You may not use this file except in compliance with the License.  You may  %
%  obtain a copy of the License at                                            %
%                                                                             %
%    https://imagemagick.org/script/license.php                               %
%                                                                             %
%  Unless required by applicable law or agreed to in writing, software        %
%  distributed under the License is distributed on an "AS IS" BASIS,          %
%  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   %
%  Wizard the License for the specific language governing permissions and        %
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
#include "wizard/key.h"
#include "wizard/memory_.h"
#include "wizard/random_.h"
#include "wizard/splay-tree.h"

/*
  Define declarations.
*/
#define KeymapCipher  AESCipher
#define KeymapMode  CTRMode
#define SessionKeyHash  SHA2256Hash
#define SessionKeyLength  512

/*
  Typedef declarations.
*/
struct _KeyInfo
{
  SplayTreeInfo
    *key_map;

  StringInfo
    *id,
    *nonce;

  CipherInfo
    *cipher_info;

  RandomInfo
    *random_info;

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
%   A c q u i r e K e y I n f o                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquireKeyInfo() allocates the KeyInfo structure.
%
%  The format of the AcquireKeyInfo method is:
%
%      KeyInfo *AcquireKeyInfo()
%
*/

static void *DestroyNode(void *entry)
{
  entry=(void *) DestroyStringInfo((StringInfo *) entry);
  return((void *) NULL);
}

WizardExport KeyInfo *AcquireKeyInfo(void)
{
  KeyInfo
    *key_info;

  key_info=(KeyInfo *) AcquireWizardMemory(sizeof(*key_info));
  if (key_info == (KeyInfo *) NULL)
    ThrowWizardFatalError(KeymapDomain,MemoryError);
  (void) ResetWizardMemory(key_info,0,sizeof(*key_info));
  key_info->key_map=NewSplayTree(CompareSplayTreeStringInfo,DestroyNode,
    DestroyNode);
  key_info->cipher_info=AcquireCipherInfo(KeymapCipher,KeymapMode);
  key_info->nonce=GenerateCipherNonce(key_info->cipher_info);
  key_info->random_info=AcquireRandomInfo(SessionKeyHash);
  key_info->timestamp=time((time_t *) NULL);
  key_info->signature=WizardSignature;
  key_info->id=GenerateSessionKey(key_info);
  return(key_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D e s t r o y K e y I n f o                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DestroyKeyInfo() zeros memory associated with the KeyInfo
%  structure.
%
%  The format of the DestroyKeyInfo method is:
%
%      KeyInfo *DestroyKeyInfo(KeyInfo *key_info)
%
%  A description of each parameter follows:
%
%    o key_info: The key info.
%
*/
WizardExport KeyInfo *DestroyKeyInfo(KeyInfo *key_info)
{
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  WizardAssert(CipherDomain,key_info != (KeyInfo *) NULL);
  WizardAssert(CipherDomain,key_info->signature == WizardSignature);
  if (key_info->cipher_info != (CipherInfo *) NULL)
    key_info->cipher_info=DestroyCipherInfo(key_info->cipher_info);
  if (key_info->random_info != (RandomInfo *) NULL)
    key_info->random_info=DestroyRandomInfo(key_info->random_info);
  if (key_info->nonce != (StringInfo *) NULL)
    key_info->nonce=DestroyStringInfo(key_info->nonce);
  if (key_info->id != (StringInfo *) NULL)
    key_info->id=DestroyStringInfo(key_info->id);
  if (key_info->key_map != (SplayTreeInfo *) NULL)
    key_info->key_map=DestroySplayTree(key_info->key_map);
  key_info->signature=(~WizardSignature);
  key_info=(KeyInfo *) RelinquishWizardMemory(key_info);
  return(key_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e n e r a t e S e s s i o n K e y                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GenerateSessionKey() generate a session key.
%
%  The format of the GenerateSessionKey method is:
%
%      void GenerateSessionKey(KeyInfo *key_info)
%
%  A description of each parameter follows:
%
%    o key_info: The key info.
%
*/
WizardExport StringInfo *GenerateSessionKey(KeyInfo *key_info)
{
  HMACInfo
    *hmac_info;

  WizardBooleanType
    status;

  StringInfo
    *id,
    *key,
    *mac_key;

  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  WizardAssert(CipherDomain,key_info != (KeyInfo *) NULL);
  WizardAssert(CipherDomain,key_info->signature == WizardSignature);
  mac_key=GetRandomKey(key_info->random_info,SessionKeyLength/8);
  key=GetRandomKey(key_info->random_info,SessionKeyLength/8);
  hmac_info=AcquireHMACInfo(SessionKeyHash);
  ConstructHMAC(hmac_info,mac_key,key);
  id=CloneStringInfo(GetHMACDigest(hmac_info));
  hmac_info=DestroyHMACInfo(hmac_info);
  mac_key=DestroyStringInfo(mac_key);
  if (key_info->id != (StringInfo *) NULL)
    status=SetKeyInfo(key_info,id,key);
  else
    status=AddValueToSplayTree(key_info->key_map,CloneStringInfo(id),
      CloneStringInfo(key));
  key=DestroyStringInfo(key);
  if (status == WizardFalse)
    ThrowWizardFatalError(KeymapDomain,KeyError);
  key_info->timestamp=time((time_t *) NULL);
  return(id);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t K e y I n f o                                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetKeyInfo() gets a key from the key splay-tree.
%
%  The format of the GetKeyInfo method is:
%
%      StringInfo *GetKeyInfo(KeyInfo *key_info,const StringInfo *id)
%
%  A description of each parameter follows:
%
%    o key_info: The key info.
%
%    o id: The key id.
%
*/
WizardExport StringInfo *GetKeyInfo(KeyInfo *key_info,const StringInfo *id)
{
  StringInfo
    *key,
    *session_key;

  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  WizardAssert(CipherDomain,key_info != (KeyInfo *) NULL);
  WizardAssert(CipherDomain,id != (const StringInfo *) NULL);
  session_key=(StringInfo *) GetValueFromSplayTree(key_info->key_map,
    key_info->id);
  if (session_key == (StringInfo *) NULL)
    ThrowWizardFatalError(KeymapDomain,KeyError);
  key=(StringInfo *) GetValueFromSplayTree(key_info->key_map,id);
  if (key == (StringInfo *) NULL)
    ThrowWizardFatalError(KeymapDomain,KeyError);
  SetCipherKey(key_info->cipher_info,session_key);
  SetCipherNonce(key_info->cipher_info,key_info->nonce);
  key=CloneStringInfo(key);
  (void) DecipherCipher(key_info->cipher_info,key);
  return(key);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t K e y I n f o                                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetKeyInfo() saves a key to the splay-tree.
%
%  The format of the SetKeyInfo
%
%      WizardBooleanType SetKeyInfo(KeyInfo *key_info,const StringInfo *id,
%        const StringInfo *key)
%
%  A description of each parameter follows:
%
%    o key_info: The key info.
%
%    o id: The key id.
%
%    o key: The key.
%
*/
WizardExport WizardBooleanType SetKeyInfo(KeyInfo *key_info,const StringInfo *id,
  const StringInfo *key)
{
  WizardBooleanType
    status;

  StringInfo
    *cipherkey,
    *session_key;

  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  WizardAssert(CipherDomain,key_info != (KeyInfo *) NULL);
  WizardAssert(CipherDomain,id != (const StringInfo *) NULL);
  WizardAssert(CipherDomain,key != (const StringInfo *) NULL);
  session_key=(StringInfo *) GetValueFromSplayTree(key_info->key_map,
    key_info->id);
  if (session_key == (StringInfo *) NULL)
    ThrowWizardFatalError(KeymapDomain,KeyError);
  SetCipherKey(key_info->cipher_info,session_key);
  SetCipherNonce(key_info->cipher_info,key_info->nonce);
  cipherkey=CloneStringInfo(key);
  (void) EncipherCipher(key_info->cipher_info,cipherkey);
  status=AddValueToSplayTree(key_info->key_map,CloneStringInfo(id),cipherkey);
  return(status);
}
