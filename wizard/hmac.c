/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                        H   H  M   M   AAA    CCCC                           %
%                        H   H  MM MM  A   A  C                               %
%                        HHHHH  M M M  AAAAA  C                               %
%                        H   H  M   M  A   A  C                               %
%                        H   H  M   M  A   A   CCCC                           %
%                                                                             %
%                                                                             %
%           Wizard's Toolkit Keyed-Hashing for Message Authentication         %
%                                                                             %
%                             Software Design                                 %
%                               John Cristy                                   %
%                               March 2003                                    %
%                                                                             %
%                                                                             %
%  Copyright 1999-2011 ImageMagick Studio LLC, a non-profit organization      %
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
#include "wizard/exception.h"
#include "wizard/exception-private.h"
#include "wizard/hash.h"
#include "wizard/hmac.h"
#include "wizard/memory_.h"

/*
  Typedef declarations.
*/
struct _HMACInfo
{
  HashInfo
    *hash_info;

  StringInfo
    *digest,
    *final_nonce,
    *initial_nonce;

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
%   A c q u i r e H M A C I n f o                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquireHMACInfo() allocate the HMACInfo structure.
%
%  The format of the AcquireHMACInfo method is:
%
%      HMACInfo *AcquireHMACInfo(const HashType hash)
%
%  A description of each parameter follows:
%
%    o hash: The hash type.
%
*/
WizardExport HMACInfo *AcquireHMACInfo(const HashType hash)
{
  HMACInfo
    *hmac_info;

  hmac_info=(HMACInfo *) AcquireWizardMemory(sizeof(*hmac_info));
  if (hmac_info == (HMACInfo *) NULL)
    ThrowWizardFatalError(MACDomain,MemoryError);
  (void) ResetWizardMemory(hmac_info,0,sizeof(*hmac_info));
  hmac_info->hash_info=AcquireHashInfo(hash);
  hmac_info->digest=AcquireStringInfo((size_t) GetHashDigestsize(
    hmac_info->hash_info));
  hmac_info->initial_nonce=AcquireStringInfo((size_t) GetHashBlocksize(
    hmac_info->hash_info));
  hmac_info->final_nonce=AcquireStringInfo((size_t) GetHashBlocksize(
    hmac_info->hash_info));
  hmac_info->timestamp=time((time_t *) NULL);
  hmac_info->signature=WizardSignature;
  return(hmac_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   C o n s t r u c t H M A C                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ConstructHMAC() constructs the HMAC digest.
%
%  The format of the ConstructHMAC method is:
%
%      ConstructHMAC(HMACInfo *hmac_info,const StringInfo *key,
%        const StringInfo *message)
%
%  A description of each parameter follows:
%
%    o hmac_info: The address of a structure of type HMACInfo.
%
%    o key: The key.
%
%    o message: The message.
%
*/
WizardExport void ConstructHMAC(HMACInfo *hmac_info,const StringInfo *key,
  const StringInfo *message)
{
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  assert(hmac_info != (HMACInfo *) NULL);
  assert(hmac_info->signature == WizardSignature);
  assert(key != (StringInfo *) NULL);
  assert(message != (StringInfo *) NULL);
  InitializeHMAC(hmac_info,key);
  UpdateHMAC(hmac_info,message);
  FinalizeHMAC(hmac_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D e s t r o y H M A C I n f o                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DestroyHMACInfo() zeros memory associated with the HMACInfo structure.
%
%  The format of the DestroyHMACInfo method is:
%
%      HMACInfo *DestroyHMACInfo(HMACInfo *hmac_info)
%
%  A description of each parameter follows:
%
%    o hmac_info: The cipher hmac_info.
%
*/
WizardExport HMACInfo *DestroyHMACInfo(HMACInfo *hmac_info)
{
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  assert(hmac_info != (HMACInfo *) NULL);
  assert(hmac_info->signature == WizardSignature);
  if (hmac_info->final_nonce != (StringInfo *)  NULL)
    hmac_info->final_nonce=DestroyStringInfo(hmac_info->final_nonce);
  if (hmac_info->initial_nonce != (StringInfo *)  NULL)
    hmac_info->initial_nonce=DestroyStringInfo(hmac_info->initial_nonce);
  if (hmac_info->digest != (StringInfo *) NULL)
    hmac_info->digest=DestroyStringInfo(hmac_info->digest);
  if (hmac_info->hash_info != (HashInfo *) NULL)
    hmac_info->hash_info=DestroyHashInfo(hmac_info->hash_info);
  hmac_info->signature=(~WizardSignature);
  hmac_info=(HMACInfo *) RelinquishWizardMemory(hmac_info);
  return(hmac_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   F i n a l i z e H M A C                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  FinalizeHMAC() finalizes the HMAC message digest computation.
%
%  The format of the FinalizeHMAC method is:
%
%      FinalizeHMAC(HMACInfo *hmac_info)
%
%  A description of each parameter follows:
%
%    o hmac_info: The address of a structure of type HMACInfo.
%
*/
WizardExport void FinalizeHMAC(HMACInfo *hmac_info)
{
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  assert(hmac_info != (HMACInfo *) NULL);
  assert(hmac_info->signature == WizardSignature);
  FinalizeHash(hmac_info->hash_info);
  SetStringInfo(hmac_info->digest,GetHashDigest(hmac_info->hash_info));
  InitializeHash(hmac_info->hash_info);
  UpdateHash(hmac_info->hash_info,hmac_info->final_nonce);
  UpdateHash(hmac_info->hash_info,hmac_info->digest);
  FinalizeHash(hmac_info->hash_info);
  SetStringInfo(hmac_info->digest,GetHashDigest(hmac_info->hash_info));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t H M A C D i g e s t                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetHMACDigest() returns the hmac digest.
%
%  The format of the GetHMACDigest method is:
%
%      const StringInfo *GetHMACDigest(const hmacInfo *hmac_info)
%
%  A description of each parameter follows:
%
%    o hmac_info: The hmac info.
%
*/
WizardExport const StringInfo *GetHMACDigest(const HMACInfo *hmac_info)
{
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  WizardAssert(MACDomain,hmac_info != (HMACInfo *) NULL);
  WizardAssert(MACDomain,hmac_info->signature == WizardSignature);
  return(hmac_info->digest);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t H M A C D i g e s t s i z e                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetHMACDigestsize() returns the HMAC digest size.
%
%  The format of the GetHMACDigestsize method is:
%
%      unsigned int *GetHMACDigestsize(const HMACInfo *hmac_info)
%
%  A description of each parameter follows:
%
%    o hmac_info: The hmac info.
%
*/
WizardExport size_t GetHMACDigestsize(const HMACInfo *hmac_info)
{
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  WizardAssert(CipherDomain,hmac_info != (HMACInfo *) NULL);
  WizardAssert(CipherDomain,hmac_info->signature == WizardSignature);
  return(GetHashDigestsize(hmac_info->hash_info));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I n i t i a l i z e H M A C                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IntializeHMAC() intializes the HMAC digest.
%
%  The format of the DestroyHMACInfo method is:
%
%      void InitializeHMACInfo(HMACInfo *hmac_info,const StringInfo *key)
%
%  A description of each parameter follows:
%
%    o hmac_info: The message authentication info.
%
%    o key: The key.
%
*/
WizardExport void InitializeHMAC(HMACInfo *hmac_info,const StringInfo *key)
{
  register size_t
    i;

  unsigned char
    *datum;

  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  assert(hmac_info != (HMACInfo *) NULL);
  assert(hmac_info->signature == WizardSignature);
  (void) ResetStringInfo(hmac_info->initial_nonce);
  if (GetStringInfoLength(key) <= GetStringInfoLength(hmac_info->initial_nonce))
    SetStringInfo(hmac_info->initial_nonce,key);
  else
    {
      InitializeHash(hmac_info->hash_info);
      UpdateHash(hmac_info->hash_info,key);
      FinalizeHash(hmac_info->hash_info);
      SetStringInfo(hmac_info->initial_nonce,GetHashDigest(
        hmac_info->hash_info));
    }
  SetStringInfo(hmac_info->final_nonce,hmac_info->initial_nonce);
  datum=GetStringInfoDatum(hmac_info->initial_nonce);
  for (i=0; i < GetStringInfoLength(hmac_info->initial_nonce); i++)
    datum[i]^=0x36;
  datum=GetStringInfoDatum(hmac_info->final_nonce);
  for (i=0; i < GetStringInfoLength(hmac_info->final_nonce); i++)
    datum[i]^=0x5c;
  ResetHMAC(hmac_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e s e t H M A C                                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ResetHMAC() resets the HMAC message digest computation.
%
%  The format of the ResetHMAC method is:
%
%      ResetHMAC(HMACInfo *hmac_info)
%
%  A description of each parameter follows:
%
%    o hmac_info: The address of a structure of type HMACInfo.
%
*/
WizardExport void ResetHMAC(HMACInfo *hmac_info)
{
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  assert(hmac_info != (HMACInfo *) NULL);
  assert(hmac_info->signature == WizardSignature);
  InitializeHash(hmac_info->hash_info);
  UpdateHash(hmac_info->hash_info,hmac_info->initial_nonce);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U p d a t e H M A C                                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UpdateHMAC() updates the HMAC message digest.
%
%  The format of the UpdateHMAC method is:
%
%      UpdateHMAC(HMACInfo *hmac_info,const StringInfo *message)
%
%  A description of each parameter follows:
%
%    o hmac_info: The address of a structure of type HMACInfo.
%
%    o message: The message
%
*/
WizardExport void UpdateHMAC(HMACInfo *hmac_info,const StringInfo *message)
{
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  assert(hmac_info != (HMACInfo *) NULL);
  assert(hmac_info->signature == WizardSignature);
  UpdateHash(hmac_info->hash_info,message);
}
