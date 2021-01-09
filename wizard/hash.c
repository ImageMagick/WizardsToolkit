/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                         H   H   AAA   SSSSS  H   H                          %
%                         H   H  A   A  SS     H   H                          %
%                         HHHHH  AAAAA   SSS   HHHHH                          %
%                         H   H  A   A     SS  H   H                          %
%                         H   H  A   A  SSSSS  H   H                          %
%                                                                             %
%                                                                             %
%                Wizard's Toolkit Secure Hash Algorithm Methods               %
%                                                                             %
%                             Software Design                                 %
%                                 Cristy                                      %
%                               March  2003                                   %
%                                                                             %
%                                                                             %
%  Copyright 1999-2021 ImageMagick Studio LLC, a non-profit organization      %
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
%  See the License for the specific language governing permissions and        %
%  limitations under the License.                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
% See http://csrc.nist.gov/groups/ST/toolkit/index.html.
%
*/

/*
  Include declarations.
*/
#include "wizard/studio.h"
#include "wizard/crc64.h"
#include "wizard/exception.h"
#include "wizard/exception-private.h"
#include "wizard/hash.h"
#include "wizard/memory_.h"
#include "wizard/md5.h"
#include "wizard/sha1.h"
#include "wizard/sha2224.h"
#include "wizard/sha2256.h"
#include "wizard/sha2384.h"
#include "wizard/sha2512.h"
#include "wizard/sha3.h"

/*
  Typedef declarations.
*/
struct _HashInfo
{   
  HashType
    hash;

  StringInfo
    *digest;

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
%   A c q u i r e H a s h I n f o                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquireHashInfo() allocates the HashInfo structure.
%
%  The format of the AcquireHashInfo method is:
%
%      HashInfo *AcquireHashInfo(const HashType hash)
%
%  A description of each parameter follows:
%
%    o hash: The hash type.
%
*/
WizardExport HashInfo *AcquireHashInfo(const HashType hash)
{
  HashInfo
    *hash_info;

  size_t
    digestsize;

  hash_info=(HashInfo *) AcquireWizardMemory(sizeof(*hash_info));
  if (hash_info == (HashInfo *) NULL)
    ThrowWizardFatalError(HashDomain,MemoryError);
  (void) ResetWizardMemory(hash_info,0,sizeof(*hash_info));
  hash_info->hash=hash;
  switch (hash_info->hash)
  {
    case CRC64Hash:
    {
      CRC64Info
        *crc_info;

      crc_info=AcquireCRC64Info();
      hash_info->handle=(HashInfo *) crc_info;
      digestsize=GetCRC64Digestsize(crc_info);
      break;
    }
    case MD5Hash:
    {
      MD5Info
        *md5_info;

      md5_info=AcquireMD5Info();
      hash_info->handle=(HashInfo *) md5_info;
      digestsize=GetMD5Digestsize(md5_info);
      break;
    }
    case SHA1Hash:
    {
      SHA1Info
        *sha_info;

      sha_info=AcquireSHA1Info();
      hash_info->handle=(HashInfo *) sha_info;
      digestsize=GetSHA1Digestsize(sha_info);
      break;
    }
    case SHA2224Hash:
    {
      SHA2224Info
        *sha_info;

      sha_info=AcquireSHA2224Info();
      hash_info->handle=(HashInfo *) sha_info;
      digestsize=GetSHA2224Digestsize(sha_info);
      break;
    }
    case SHA2256Hash:
    case SHA2Hash:
    {
      SHA2256Info
        *sha_info;

      sha_info=AcquireSHA2256Info();
      hash_info->handle=(HashInfo *) sha_info;
      digestsize=GetSHA2256Digestsize(sha_info);
      break;
    }
    case SHA2384Hash:
    {
      SHA2384Info
        *sha_info;

      sha_info=AcquireSHA2384Info();
      hash_info->handle=(HashInfo *) sha_info;
      digestsize=GetSHA2384Digestsize(sha_info);
      break;
    }
    case SHA2512Hash:
    {
      SHA2512Info
        *sha_info;

      sha_info=AcquireSHA2512Info();
      hash_info->handle=(HashInfo *) sha_info;
      digestsize=GetSHA2512Digestsize(sha_info);
      break;
    }
    case SHA3Hash:
    case SHA3224Hash:
    case SHA3256Hash:
    case SHA3384Hash:
    case SHA3512Hash:
    {
      SHA3Info
        *sha_info;

      sha_info=AcquireSHA3Info(hash);
      hash_info->handle=(HashInfo *) sha_info;
      digestsize=GetSHA3Digestsize(sha_info);
      break;
    }
    default:
      ThrowWizardFatalError(HashDomain,EnumerateError);
  }
  hash_info->digest=AcquireStringInfo((size_t) digestsize);
  ResetStringInfo(hash_info->digest);
  hash_info->timestamp=time((time_t *) NULL);
  hash_info->signature=WizardSignature;
  return(hash_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D e s t r o y H a s h I n f o                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DestroyHashInfo() zeros memory associated with the HashInfo structure.
%
%  The format of the DestroyHashInfo method is:
%
%      HashInfo *DestroyHashInfo(HashInfo *hash_info)
%
%  A description of each parameter follows:
%
%    o hash_info: The hash info.
%
*/
WizardExport HashInfo *DestroyHashInfo(HashInfo *hash_info)
{
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  assert(hash_info != (HashInfo *) NULL);
  assert(hash_info->signature == WizardSignature);
  if (hash_info->handle != (HashInfo *) NULL)
    switch (hash_info->hash)
    {
      case CRC64Hash:
      {
        hash_info->handle=(void *) DestroyCRC64Info((CRC64Info *)
          hash_info->handle);
        break;
      }
      case MD5Hash:
      {
        hash_info->handle=(void *) DestroyMD5Info((MD5Info *)
          hash_info->handle);
        break;
      }
      case SHA1Hash:
      {
        hash_info->handle=(void *) DestroySHA1Info((SHA1Info *)
          hash_info->handle);
        break;
      }
      case SHA2224Hash:
      {
        hash_info->handle=(void *) DestroySHA2224Info((SHA2224Info *)
          hash_info->handle);
        break;
      }
      case SHA2256Hash:
      case SHA2Hash:
      {
        hash_info->handle=(void *) DestroySHA2256Info((SHA2256Info *)
          hash_info->handle);
        break;
      }
      case SHA2384Hash:
      {
        hash_info->handle=(void *) DestroySHA2384Info((SHA2384Info *)
          hash_info->handle);
        break;
      }
      case SHA2512Hash:
      {
        hash_info->handle=(void *) DestroySHA2512Info((SHA2512Info *)
          hash_info->handle);
        break;
      }
      case SHA3Hash:
      case SHA3224Hash:
      case SHA3256Hash:
      case SHA3384Hash:
      case SHA3512Hash:
      {
        hash_info->handle=(void *) DestroySHA3Info((SHA3Info *)
          hash_info->handle);
        break;
      }
      default:
        ThrowWizardFatalError(HashDomain,EnumerateError);
    }
  if (hash_info->digest != (StringInfo *) NULL)
    hash_info->digest=DestroyStringInfo(hash_info->digest);
  hash_info->signature=(~WizardSignature);
  hash_info=(HashInfo *) RelinquishWizardMemory(hash_info);
  return(hash_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   F i n a l i z e H a s h                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  FinalizeHash() finalizes the Hash message accumulator computation.
%
%  The format of the FinalizeHash method is:
%
%      WizardBooleanType FinalizeHash(HashInfo *hash_info)
%
%  A description of each parameter follows:
%
%    o hash_info: The address of a structure of type HashInfo.
%
*/
WizardExport WizardBooleanType FinalizeHash(HashInfo *hash_info)
{
  WizardBooleanType
    status;

  /*
    Add padding and return the message accumulator.
  */
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  assert(hash_info != (HashInfo *) NULL);
  assert(hash_info->signature == WizardSignature);
  switch (hash_info->hash)
  {
    case CRC64Hash:
    {
      CRC64Info
        *crc_info;

      crc_info=(CRC64Info *) hash_info->handle;
      status=FinalizeCRC64(crc_info);
      SetStringInfo(hash_info->digest,GetCRC64Digest(crc_info));
      break;
    }
    case MD5Hash:
    {
      MD5Info
        *md5_info;

      md5_info=(MD5Info *) hash_info->handle;
      status=FinalizeMD5(md5_info);
      SetStringInfo(hash_info->digest,GetMD5Digest(md5_info));
      break;
    }
    case SHA1Hash:
    {
      SHA1Info
        *sha_info;

      sha_info=(SHA1Info *) hash_info->handle;
      status=FinalizeSHA1(sha_info);
      SetStringInfo(hash_info->digest,GetSHA1Digest(sha_info));
      break;
    }
    case SHA2224Hash:
    {
      SHA2224Info
        *sha_info;

      sha_info=(SHA2224Info *) hash_info->handle;
      status=FinalizeSHA2224(sha_info);
      SetStringInfo(hash_info->digest,GetSHA2224Digest(sha_info));
      break;
    }
    case SHA2256Hash:
    case SHA2Hash:
    {
      SHA2256Info
        *sha_info;

      sha_info=(SHA2256Info *) hash_info->handle;
      status=FinalizeSHA2256(sha_info);
      SetStringInfo(hash_info->digest,GetSHA2256Digest(sha_info));
      break;
    }
    case SHA2384Hash:
    {
      SHA2384Info
        *sha_info;

      sha_info=(SHA2384Info *) hash_info->handle;
      status=FinalizeSHA2384(sha_info);
      SetStringInfo(hash_info->digest,GetSHA2384Digest(sha_info));
      break;
    }
    case SHA2512Hash:
    {
      SHA2512Info
        *sha_info;

      sha_info=(SHA2512Info *) hash_info->handle;
      status=FinalizeSHA2512(sha_info);
      SetStringInfo(hash_info->digest,GetSHA2512Digest(sha_info));
      break;
    }
    case SHA3Hash:
    case SHA3224Hash:
    case SHA3256Hash:
    case SHA3384Hash:
    case SHA3512Hash:
    {
      SHA3Info
        *sha_info;

      sha_info=(SHA3Info *) hash_info->handle;
      status=FinalizeSHA3(sha_info);
      SetStringInfo(hash_info->digest,GetSHA3Digest(sha_info));
      break;
    }
    default:
    {
      status=WizardFalse;
      break;
    }
  }
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t H a s h B l o c k s i z e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetHashBlocksize() returns the Hash blocksize.
%
%  The format of the GetHashBlocksize method is:
%
%      size_t *GetHashBlocksize(const HashInfo *hash_info)
%
%  A description of each parameter follows:
%
%    o hash_info: The hash info.
%
*/
WizardExport size_t GetHashBlocksize(const HashInfo *hash_info)
{
  size_t
    blocksize;

  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  WizardAssert(CipherDomain,hash_info != (HashInfo *) NULL);
  WizardAssert(CipherDomain,hash_info->signature == WizardSignature);
  switch (hash_info->hash)
  {
    case CRC64Hash:
    {
      CRC64Info
        *crc_info;

      crc_info=(CRC64Info *) hash_info->handle;
      blocksize=GetCRC64Blocksize(crc_info);
      break;
    }
    case MD5Hash:
    {
      MD5Info
        *md5_info;

      md5_info=(MD5Info *) hash_info->handle;
      blocksize=GetMD5Blocksize(md5_info);
      break;
    }
    case SHA1Hash:
    {
      SHA1Info
        *sha_info;

      sha_info=(SHA1Info *) hash_info->handle;
      blocksize=GetSHA1Blocksize(sha_info);
      break;
    }
    case SHA2224Hash:
    {
      SHA2224Info
        *sha_info;

      sha_info=(SHA2224Info *) hash_info->handle;
      blocksize=GetSHA2224Blocksize(sha_info);
      break;
    }
    case SHA2256Hash:
    case SHA2Hash:
    {
      SHA2256Info
        *sha_info;

      sha_info=(SHA2256Info *) hash_info->handle;
      blocksize=GetSHA2256Blocksize(sha_info);
      break;
    }
    case SHA2384Hash:
    {
      SHA2384Info
        *sha_info;

      sha_info=(SHA2384Info *) hash_info->handle;
      blocksize=GetSHA2384Blocksize(sha_info);
      break;
    }
    case SHA2512Hash:
    {
      SHA2512Info
        *sha_info;

      sha_info=(SHA2512Info *) hash_info->handle;
      blocksize=GetSHA2512Blocksize(sha_info);
      break;
    }
    case SHA3Hash:
    case SHA3224Hash:
    case SHA3256Hash:
    case SHA3384Hash:
    case SHA3512Hash:
    {
      SHA3Info
        *sha_info;

      sha_info=(SHA3Info *) hash_info->handle;
      blocksize=GetSHA3Blocksize(sha_info);
      break;
    }
    default:
      ThrowWizardFatalError(HashDomain,EnumerateError);
  }
  return(blocksize);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t H a s h D i g e s t                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetHashDigest() returns the Hash digest.
%
%  The format of the GetHashDigest method is:
%
%      const StringInfo *GetHashDigest(const HashInfo *hash_info)
%
%  A description of each parameter follows:
%
%    o hash_info: The hash info.
%
*/
WizardExport const StringInfo *GetHashDigest(const HashInfo *hash_info)
{
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  WizardAssert(HashDomain,hash_info != (HashInfo *) NULL);
  WizardAssert(HashDomain,hash_info->signature == WizardSignature);
  return(hash_info->digest);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t H a s h D i g e s t s i z e                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetHashDigestsize() returns the Hash digest size.
%
%  The format of the GetHashDigestsize method is:
%
%      unsigned int *GetHashDigestsize(const HashInfo *hash_info)
%
%  A description of each parameter follows:
%
%    o hash_info: The hash info.
%
*/
WizardExport size_t GetHashDigestsize(const HashInfo *hash_info)
{
  size_t
    digestsize;

  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  WizardAssert(CipherDomain,hash_info != (HashInfo *) NULL);
  WizardAssert(CipherDomain,hash_info->signature == WizardSignature);
  switch (hash_info->hash)
  {
    case CRC64Hash:
    {
      CRC64Info
        *crc_info;

      crc_info=(CRC64Info *) hash_info->handle;
      digestsize=GetCRC64Digestsize(crc_info);
      break;
    }
    case MD5Hash:
    {
      MD5Info
        *md5_info;

      md5_info=(MD5Info *) hash_info->handle;
      digestsize=GetMD5Digestsize(md5_info);
      break;
    }
    case SHA1Hash:
    {
      SHA1Info
        *sha_info;

      sha_info=(SHA1Info *) hash_info->handle;
      digestsize=GetSHA1Digestsize(sha_info);
      break;
    }
    case SHA2224Hash:
    {
      SHA2224Info
        *sha_info;

      sha_info=(SHA2224Info *) hash_info->handle;
      digestsize=GetSHA2224Digestsize(sha_info);
      break;
    }
    case SHA2256Hash:
    case SHA2Hash:
    {
      SHA2256Info
        *sha_info;

      sha_info=(SHA2256Info *) hash_info->handle;
      digestsize=GetSHA2256Digestsize(sha_info);
      break;
    }
    case SHA2384Hash:
    {
      SHA2384Info
        *sha_info;

      sha_info=(SHA2384Info *) hash_info->handle;
      digestsize=GetSHA2384Digestsize(sha_info);
      break;
    }
    case SHA2512Hash:
    {
      SHA2512Info
        *sha_info;

      sha_info=(SHA2512Info *) hash_info->handle;
      digestsize=GetSHA2512Digestsize(sha_info);
      break;
    }
    case SHA3Hash:
    case SHA3224Hash:
    case SHA3256Hash:
    case SHA3384Hash:
    case SHA3512Hash:
    {
      SHA3Info
        *sha_info;

      sha_info=(SHA3Info *) hash_info->handle;
      digestsize=GetSHA3Digestsize(sha_info);
      break;
    }
    default:
      ThrowWizardFatalError(HashDomain,EnumerateError);
  }
  return(digestsize);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t H a s h H e x D i g e s t                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetHashHexDigest() returns the hash digest as a hex string.
%
%  The format of the GetHashHexDigest method is:
%
%      char *GetHashHexDigest(const HashInfo *hash_info)
%
%  A description of each parameter follows:
%
%    o hash_info: The hash info.
%
*/
WizardExport char *GetHashHexDigest(const HashInfo *hash_info)
{
  char
    *digest;

   const unsigned char
    *p;

   ssize_t
    i;

  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  WizardAssert(HashDomain,hash_info != (HashInfo *) NULL);
  WizardAssert(HashDomain,hash_info->signature == WizardSignature);
  digest=(char *) AcquireQuantumMemory(2UL*GetHashDigestsize(hash_info)+1UL,
    sizeof(*digest));
  if (digest == (char *) NULL)
    ThrowWizardFatalError(HashDomain,MemoryError);
  p=GetStringInfoDatum(hash_info->digest);
  for (i=0; i < (ssize_t) GetHashDigestsize(hash_info); i++)
    (void) FormatLocaleString(digest+2*i,WizardPathExtent,"%02x",*p++);
  digest[2*i]='\0';
  return(digest);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I n i t i a l i z e H a s h                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IntializeHash() intializes the Hash accumulator.
%
%  The format of the IntializeHash method is:
%
%      WizardBooleanType IntializeHash(HashInfo *hash_info)
%
%  A description of each parameter follows:
%
%    o hash_info: The hash info.
%
*/
WizardExport WizardBooleanType InitializeHash(HashInfo *hash_info)
{
  WizardBooleanType
    status;

  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  assert(hash_info != (HashInfo *) NULL);
  assert(hash_info->signature == WizardSignature);
  switch (hash_info->hash)
  {
    case CRC64Hash:
    {
      status=InitializeCRC64((CRC64Info *) hash_info->handle);
      break;
    }
    case MD5Hash:
    {
      status=InitializeMD5((MD5Info *) hash_info->handle);
      break;
    }
    case SHA1Hash:
    {
      status=InitializeSHA1((SHA1Info *) hash_info->handle);
      break;
    }
    case SHA2224Hash:
    {
      status=InitializeSHA2224((SHA2224Info *) hash_info->handle);
      break;
    }
    case SHA2256Hash:
    case SHA2Hash:
    {
      status=InitializeSHA2256((SHA2256Info *) hash_info->handle);
      break;
    }
    case SHA2384Hash:
    {
      status=InitializeSHA2384((SHA2384Info *) hash_info->handle);
      break;
    }
    case SHA2512Hash:
    {
      status=InitializeSHA2512((SHA2512Info *) hash_info->handle);
      break;
    }
    case SHA3Hash:
    case SHA3224Hash:
    case SHA3256Hash:
    case SHA3384Hash:
    case SHA3512Hash:
    {
      status=InitializeSHA3((SHA3Info *) hash_info->handle);
      break;
    }
    default:
    {
      status=WizardFalse;
      break;
    }
  }
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U p d a t e H a s h                                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UpdateHash() updates the Hash message accumulator.
%
%  The format of the UpdateHash method is:
%
%      WizardBooleanType UpdateHash(HashInfo *hash_info,
%        const StringInfo *message)
%
%  A description of each parameter follows:
%
%    o hash_info: The address of a structure of type HashInfo.
%
%    o message: The message.
%
*/
WizardExport WizardBooleanType UpdateHash(HashInfo *hash_info,
  const StringInfo *message)
{
  WizardBooleanType
    status;

  /*
    Update the Hash accumulator.
  */
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  assert(hash_info != (HashInfo *) NULL);
  assert(hash_info->signature == WizardSignature);
  switch (hash_info->hash)
  {
    case CRC64Hash:
    {
      status=UpdateCRC64((CRC64Info *) hash_info->handle,message);
      break;
    }
    case MD5Hash:
    {
      status=UpdateMD5((MD5Info *) hash_info->handle,message);
      break;
    }
    case SHA1Hash:
    {
      status=UpdateSHA1((SHA1Info *) hash_info->handle,message);
      break;
    }
    case SHA2224Hash:
    {
      status=UpdateSHA2224((SHA2224Info *) hash_info->handle,message);
      break;
    }
    case SHA2256Hash:
    case SHA2Hash:
    {
      status=UpdateSHA2256((SHA2256Info *) hash_info->handle,message);
      break;
    }
    case SHA2384Hash:
    {
      status=UpdateSHA2384((SHA2384Info *) hash_info->handle,message);
      break;
    }
    case SHA2512Hash:
    {
      status=UpdateSHA2512((SHA2512Info *) hash_info->handle,message);
      break;
    }
    case SHA3Hash:
    case SHA3224Hash:
    case SHA3256Hash:
    case SHA3384Hash:
    case SHA3512Hash:
    {
      status=UpdateSHA3((SHA3Info *) hash_info->handle,message);
      break;
    }
    default:
    {
      status=WizardFalse;
      break;
    }
  }
  return(status);
}
