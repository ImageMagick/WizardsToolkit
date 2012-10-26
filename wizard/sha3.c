/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                             SSSSS  H   H   AAA                              %
%                             SS     H   H  A   A                             %
%                              SSS   HHHHH  AAAAA                             %
%                                SS  H   H  A   A                             %
%                             SSSSS  H   H  A   A                             %
%                                                                             %
%                                                                             %
%             Wizard's Toolkit Secure Hash Algorithm 3 Methods                %
%                                                                             %
%                             Software Design                                 %
%                               John Cristy                                   %
%                               March  2012                                   %
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
% SHA-3 uses the "sponge construction", where input is "absorbed" into the hash
% state at a given rate, an output hash is then "squeezed" from it at the same
% rate.  See http://keccak.noekeon.org/.
%
*/

/*
  Include declarations.
*/
#include "wizard/studio.h"
#include "wizard/exception.h"
#include "wizard/exception-private.h"
#include "wizard/memory_.h"
#include "wizard/sha3.h"

/*
  Define declarations.
*/
#define SHA3Blocksize  64
#define SHA3Digestsize  64
#define SHA3Index(x,y)  (((x) % 5)+5*((y) % 5))
#define SHA3Lanes  25
#define SHA3MaximumRate  1536
#define SHA3PermutationSize  1600
#define SHA3RotateLeft(x,offset)  ((offset) != 0 ? ((((WizardSizeType) (x)) << \
  offset) ^ (((WizardSizeType) (x)) >> (64-(offset)))) : (x))
#define SHA3Rounds  24

/*
  Typedef declarations.
*/
struct _SHA3Info
{
  HashType
    hash;

  unsigned int
    digestsize,
    blocksize;

  StringInfo
    *digest;

  unsigned char
    state[SHA3PermutationSize/8],
    message[SHA3MaximumRate/8];

  unsigned int
    rate,
    capacity,
    bits_in_queue;

  size_t
    length;

  WizardBooleanType
    squeeze;

  unsigned int
    squeeze_bits,
    rho[SHA3Lanes];

  WizardSizeType
    rounds[SHA3Rounds];

  WizardBooleanType
    lsb_first;

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
%   A c q u i r e S H A I n f o                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquireSHA3Info() allocate the SHA3Info structure.
%
%  The format of the AcquireSHA3Info method is:
%
%      SHA3Info *AcquireSHA3Info(const HashType hash)
%
%  A description of each parameter follows:
%
%    o hash: The hash type.
%
*/
WizardExport SHA3Info *AcquireSHA3Info(const HashType hash)
{
  SHA3Info
    *sha_info;

  unsigned int
    lsb_first;

  sha_info=(SHA3Info *) AcquireWizardMemory(sizeof(*sha_info));
  if (sha_info == (SHA3Info *) NULL)
    ThrowWizardFatalError(HashDomain,MemoryError);
  (void) ResetWizardMemory(sha_info,0,sizeof(*sha_info));
  sha_info->hash=hash;
  switch (sha_info->hash)
  {
    case SHA3Hash:
    {
      sha_info->digestsize=36;
      break;
    }
    case SHA3224Hash:
    {
      sha_info->digestsize=28;
      break;
    }
    case SHA3256Hash:
    {
      sha_info->digestsize=32;
      break;
    }
    case SHA3384Hash:
    {
      sha_info->digestsize=48;
      break;
    }
    case SHA3512Hash:
    {
      sha_info->digestsize=64;
      break;
    }
    default:
      ThrowWizardFatalError(HashDomain,HashIOError);
  }
  sha_info->blocksize=SHA3Blocksize;
  sha_info->digest=AcquireStringInfo(sha_info->digestsize);
  lsb_first=1;
  sha_info->lsb_first=(int) (*(char *) &lsb_first) == 1 ? WizardTrue :
    WizardFalse;
  sha_info->timestamp=time((time_t *) NULL);
  sha_info->signature=WizardSignature;
  return(sha_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D e s t r o y S H A I n f o                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DestroySHA3Info() zeros memory associated with the SHA3Info structure.
%
%  The format of the DestroySHA3Info method is:
%
%      SHA3Info *DestroySHA3Info(SHA3Info *sha_info)
%
%  A description of each parameter follows:
%
%    o sha_info: The cipher sha_info.
%
*/
WizardExport SHA3Info *DestroySHA3Info(SHA3Info *sha_info)
{
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  assert(sha_info != (SHA3Info *) NULL);
  assert(sha_info->signature == WizardSignature);
  if (sha_info->digest != (StringInfo *) NULL)
    sha_info->digest=DestroyStringInfo(sha_info->digest);
  sha_info->signature=(~WizardSignature);
  sha_info=(SHA3Info *) RelinquishWizardMemory(sha_info);
  return(sha_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   F i n a l i z e S H A                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  FinalizeSHA3() finalizes the SHA3 message accumulator computation.
%
%  The format of the FinalizeSHA3 method is:
%
%      WizardBooleanType FinalizeSHA3(SHA3Info *sha_info)
%
%  A description of each parameter follows:
%
%    o sha_info: The address of a structure of type SHA3Info.
%
*/

static inline void ConvertBytesToWords(const unsigned char *bytes,
  WizardSizeType *words)
{
  register ssize_t
    i;

  for (i=0; i < (SHA3PermutationSize/64); i++)
  {
    register ssize_t
      j;

    words[i]=0;
    for (j=0; j< (64/8); j++)
      words[i]|=(WizardSizeType) (bytes[i*(64/8)+j]) << (8*j);
  }
}

static inline void ConvertWordsToBytes(const WizardSizeType *words,
  unsigned char *bytes)
{
  register ssize_t
    i;

  for (i=0; i < (SHA3PermutationSize/64); i++)
  {
    register ssize_t
      j;

    for (j=0; j < (64/8); j++)
      bytes[i*(64/8)+j]=(unsigned char) ((words[i] >> (8*j)) & 0xFF);
  }
}

static void SHA3PermutationOnWords(const SHA3Info *sha_info,
  WizardSizeType *state)
{
  register ssize_t
    i;

  for (i=0; i < SHA3Rounds; i++)
  {
    ssize_t
      x,
      y;

    WizardSizeType
      C[5],
      D[5],
      T[25];

    /*
      Theta.
    */
    for (x=0; x < 5; x++)
    {
      C[x]=0;
      for (y=0; y < 5; y++)
        C[x]^=state[SHA3Index(x,y)];
    }
    for (x=0; x < 5; x++)
      D[x]=SHA3RotateLeft(C[(x+1) % 5],1) ^ C[(x+4) % 5];
    for (x=0; x < 5; x++)
      for (y=0; y < 5; y++)
        state[SHA3Index(x,y)]^=D[x];
    /*
      Rho.
    */
    for (x=0; x < 5; x++)
      for (y=0; y < 5; y++)
        state[SHA3Index(x,y)]=SHA3RotateLeft(state[SHA3Index(x,y)],
          (WizardSizeType) sha_info->rho[SHA3Index(x,y)]);
    /*
      Pi.
    */
    for (x=0; x < 5; x++)
      for (y=0; y < 5; y++)
        T[SHA3Index(x,y)]=state[SHA3Index(x,y)];
    for (x=0; x < 5; x++)
      for (y=0; y < 5; y++)
        state[SHA3Index(0*x+1*y,2*x+3*y)]=T[SHA3Index(x,y)];
    /*
      Chi.
    */
    for (y=0; y < 5; y++)
    {
      for (x=0; x < 5; x++)
        C[x]=state[SHA3Index(x,y)] ^ ((~state[SHA3Index(x+1,y)]) &
          state[SHA3Index(x+2,y)]);
      for (x=0; x< 5; x++)
        state[SHA3Index(x,y)]=C[x];
     }
    /*
      Iota.
    */
    state[SHA3Index(0,0)]^=sha_info->rounds[i];
  }
}

static inline void SHA3Permutation(const SHA3Info *sha_info,
  unsigned char *bytes)
{
  WizardSizeType
    words[SHA3PermutationSize/64];

  if (sha_info->lsb_first != 0)
    SHA3PermutationOnWords(sha_info,(WizardSizeType *) bytes);
  else
    {
      ConvertBytesToWords(bytes,words);
      SHA3PermutationOnWords(sha_info,words);
      ConvertWordsToBytes(words,bytes);
    }
}

static inline void SHA3PermutationAfterXor(const SHA3Info *sha_info,
  const unsigned char *message,const size_t length,unsigned char *state)
{
  register ssize_t
    i;

  for (i=0; i < (ssize_t) length; i++)
    state[i]^=message[i];
  SHA3Permutation(sha_info,state);
}

static inline void AbsorbQueue(SHA3Info *sha_info)
{
  switch (sha_info->rate)
  {
    case 576:
    {
      SHA3PermutationAfterXor(sha_info,sha_info->message,72,sha_info->state);
      break;
    }
    case 832:
    {
      SHA3PermutationAfterXor(sha_info,sha_info->message,104,sha_info->state);
      break;
    }
    case 1024:
    {
      SHA3PermutationAfterXor(sha_info,sha_info->message,128,sha_info->state);
      break;
    }
    case 1088:
    {
      SHA3PermutationAfterXor(sha_info,sha_info->message,136,sha_info->state);
      break;
    }
    case 1152:
    {
      SHA3PermutationAfterXor(sha_info,sha_info->message,144,sha_info->state);
      break;
    }
    case 1344:
    {
      SHA3PermutationAfterXor(sha_info,sha_info->message,168,sha_info->state);
      break;
    }
    default:
    {
      SHA3PermutationAfterXor(sha_info,sha_info->message,8*sha_info->rate/64,
        sha_info->state);
      break;
    }
  }
  sha_info->bits_in_queue=0;
}

static inline void PadAndSwitchToSqueezingPhase(SHA3Info *sha_info)
{
  if ((sha_info->bits_in_queue+1) == sha_info->rate)
    {
      sha_info->message[sha_info->bits_in_queue/8]|=1 <<
        (sha_info->bits_in_queue % 8);
      AbsorbQueue(sha_info);
      memset(sha_info->message,0,sha_info->rate/8);
    }
  else
    {
      memset(sha_info->message+(sha_info->bits_in_queue+7)/8,0,sha_info->rate/8-
        (sha_info->bits_in_queue+7)/8);
      sha_info->message[sha_info->bits_in_queue/8]|=1 <<
        (sha_info->bits_in_queue % 8);
    }
  sha_info->message[(sha_info->rate-1)/8]|=1 << ((sha_info->rate-1) % 8);
  AbsorbQueue(sha_info);
  memcpy(sha_info->message,sha_info->state,8*sha_info->rate/64);
  sha_info->squeeze_bits=sha_info->rate;
  sha_info->squeeze=WizardTrue;
}

static inline WizardBooleanType Squeeze(SHA3Info *sha_info,const size_t length,
  unsigned char *output)
{
  register ssize_t
    i;

  size_t
    bits;

  /*
    Squeeze output data from the sponge function.
  */
  if (sha_info->squeeze == WizardFalse)
    PadAndSwitchToSqueezingPhase(sha_info);
  if ((length % 8) != 0)
    return(WizardFalse);  /* must be a multiple of 8 */
  for (i=0; i < (ssize_t) length; i+=bits)
  {
    if (sha_info->squeeze_bits == 0)
      {
        SHA3Permutation(sha_info,sha_info->state);
        memcpy(sha_info->message,sha_info->state,8*sha_info->rate/64);
        sha_info->squeeze_bits=sha_info->rate;
      }
    bits=sha_info->squeeze_bits;
    if (bits > (length-i))
      bits=length-i;
    memcpy(output+i/8,sha_info->message+(sha_info->rate-sha_info->squeeze_bits)/
      8,bits/8);
    sha_info->squeeze_bits-=bits;
  }
  return(WizardTrue);
}

WizardExport WizardBooleanType FinalizeSHA3(SHA3Info *sha_info)
{
  register ssize_t
    i;

  register unsigned char
    *p,
    *q;

  SHA3Info
    clone_info;

  unsigned char
    digest[SHA3Digestsize];

  WizardBooleanType
    status;

  /*
    Add padding and return the message accumulator.
  */
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  assert(sha_info != (SHA3Info *) NULL);
  assert(sha_info->signature == WizardSignature);
  clone_info=(*sha_info);
  status=Squeeze(&clone_info,clone_info.length,digest);
  if (status == WizardFalse)
    return(WizardFalse);
  p=digest;
  q=GetStringInfoDatum(sha_info->digest);
  for (i=0; i < (ssize_t) sha_info->digestsize; i++)
    *q++=(*p++);
  return(WizardTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t S H A 3 B l o c k s i z e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetSHA3Blocksize() returns the SHA3 blocksize.
%
%  The format of the GetSHA3Blocksize method is:
%
%      unsigned int *GetSHA3Blocksize(const SHA3Info *sha_info)
%
%  A description of each parameter follows:
%
%    o sha_info: The sha3 info.
%
*/
WizardExport unsigned int GetSHA3Blocksize(const SHA3Info *sha_info)
{
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  WizardAssert(CipherDomain,sha_info != (SHA3Info *) NULL);
  WizardAssert(CipherDomain,sha_info->signature == WizardSignature);
  return(sha_info->blocksize);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t S H A 3 D i g e s t                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetSHA3Digest() returns the SHA3 digest.
%
%  The format of the GetSHA3Digest method is:
%
%      const StringInfo *GetSHA3Digest(const SHA3Info *sha_info)
%
%  A description of each parameter follows:
%
%    o sha_info: The sha3 info.
%
*/
WizardExport const StringInfo *GetSHA3Digest(const SHA3Info *sha_info)
{
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  WizardAssert(HashDomain,sha_info != (SHA3Info *) NULL);
  WizardAssert(HashDomain,sha_info->signature == WizardSignature);
  return(sha_info->digest);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t S H A 3 D i g e s t s i z e                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetSHA3Digestsize() returns the SHA3 digest size.
%
%  The format of the GetSHA3Digestsize method is:
%
%      unsigned int *GetSHA3Digestsize(const SHA3Info *sha_info)
%
%  A description of each parameter follows:
%
%    o sha_info: The sha3 info.
%
*/
WizardExport unsigned int GetSHA3Digestsize(const SHA3Info *sha_info)
{
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  WizardAssert(CipherDomain,sha_info != (SHA3Info *) NULL);
  WizardAssert(CipherDomain,sha_info->signature == WizardSignature);
  return(sha_info->digestsize);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I n i t i a l i z e S H A                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IntializeSHA3() intializes the SHA3 accumulator.
%
%  The format of the DestroySHA3Info method is:
%
%      WizardBooleanType InitializeSHA3Info(SHA3Info *sha_info)
%
%  A description of each parameter follows:
%
%    o sha_info: The cipher sha_info.
%
*/

static inline int LFSR86540(unsigned char *byte)
{
  int
    result;

  /*
    Primitive polynomial over GF(2): x^8+x^6+x^5+x^4+1.
  */
  result=((*byte) & 0x01) != 0;
  if (((*byte) & 0x80) != 0)
    (*byte)=((*byte) << 1) ^ 0x71;
  else
    (*byte)<<=1;
  return(result);
}

static inline void SHA3InitializeRoundConstants(SHA3Info *sha_info)
{
  register ssize_t
    i;

  unsigned char
    byte;

  byte=0x01;
  for (i=0; i < SHA3Rounds; i++)
  {
    register ssize_t
      j;

    ssize_t
      offset;

    sha_info->rounds[i]=0;
    for (j=0; j < 7; j++)
    {
      offset=(ssize_t) (1 << j)-1;
      if (LFSR86540(&byte) != 0)
        sha_info->rounds[i]^=(WizardSizeType) 1 << offset;
    }
  }
}

static inline void SHA3InitializeRho(SHA3Info *sha_info)
{
  register ssize_t
    i;

  ssize_t
    x,
    y;

  sha_info->rho[SHA3Index(0,0)]=0;
  x=1;
  y=0;
  for (i=0; i < 24; i++)
  {
    ssize_t
      z;

    sha_info->rho[SHA3Index(x,y)]=(unsigned int) (((i+1)*(i+2)/2) % 64);
    z=x;
    x=(0*x+1*y) % 5;
    y=(2*z+3*y) % 5;
  }
}

static inline WizardBooleanType InitializeSponge(SHA3Info *sha_info,
  const unsigned int rate,const unsigned int capacity)
{
  if (rate+capacity != 1600)
    return(WizardFalse);
  if ((rate <= 0) || (rate >= 1600) || ((rate % 64) != 0))
    return(WizardFalse);
  SHA3InitializeRoundConstants(sha_info);
  SHA3InitializeRho(sha_info);
  sha_info->rate=rate;
  sha_info->capacity=capacity;
  sha_info->length=0;
  memset(sha_info->state,0,SHA3PermutationSize/8);
  memset(sha_info->message,0,SHA3MaximumRate/8);
  sha_info->bits_in_queue=0;
  sha_info->squeeze=WizardFalse;
  sha_info->squeeze_bits=0;
  sha_info->length=capacity/2;
  return(WizardTrue);
}

WizardExport WizardBooleanType InitializeSHA3(SHA3Info *sha_info)
{
  WizardBooleanType
    status;

  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  assert(sha_info != (SHA3Info *) NULL);
  assert(sha_info->signature == WizardSignature);
  switch (sha_info->hash)
  {
    case SHA3Hash:
    {
      status=InitializeSponge(sha_info,1024,576);
      break;
    }
    case SHA3224Hash:
    {
      status=InitializeSponge(sha_info,1152,448);
      break;
    }
    case SHA3256Hash:
    {
      status=InitializeSponge(sha_info,1088,512);
      break;
    }
    case SHA3384Hash:
    {
      status=InitializeSponge(sha_info,832,768);
      break;
    }
    case SHA3512Hash:
    {
      status=InitializeSponge(sha_info,576,1024);
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
%   U p d a t e S H A                                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UpdateSHA3() updates the SHA3 message accumulator.
%
%  The format of the UpdateSHA3 method is:
%
%      WizardbooleanType UpdateSHA3(SHA3Info *sha_info,
%        const StringInfo *message)
%
%  A description of each parameter follows:
%
%    o sha_info: The address of a structure of type SHA3Info.
%
%    o message: The message
%
*/

static inline WizardBooleanType Absorb(SHA3Info *sha_info,
  const unsigned char *message,const size_t length)
{
  register const unsigned char
    *p;

  register ssize_t
    i;

  size_t
    bits,
    blocks,
    byte;

  /*
    Give input message for the sponge function to absorb.
  */
  if ((sha_info->bits_in_queue % 8) != 0)
    return(WizardFalse);  /* only the last call may contain a partial byte */
  if (sha_info->squeeze != WizardFalse)
    return(WizardFalse);  /* too late for additional input */
  for (i=0; i < (ssize_t) length; )
  {
    if ((sha_info->bits_in_queue != 0) || (length < sha_info->rate) ||
        (i > (ssize_t) (length-sha_info->rate)))
      {
        bits=(unsigned int) (length-i);
        if ((bits+sha_info->bits_in_queue) > sha_info->rate)
          bits=sha_info->rate-sha_info->bits_in_queue;
        byte=bits % 8;
        bits-=byte;
        memcpy(sha_info->message+sha_info->bits_in_queue/8,message+i/8,bits/8);
        sha_info->bits_in_queue+=bits;
        i+=bits;
        if (sha_info->bits_in_queue == sha_info->rate)
          AbsorbQueue(sha_info);
        if (byte > 0)
          {
            unsigned char
              mask;

            mask=(unsigned char) ((1 << byte)-1);
            sha_info->message[sha_info->bits_in_queue/8]=message[i/8] & mask;
            sha_info->bits_in_queue+=byte;
            i+=byte;
          }
      }
    else
      {
        register ssize_t
          j;

        blocks=(size_t) ((length-i)/sha_info->rate);
        p=message+i/8;
        switch (sha_info->rate)
        {
          case 576:
          {
            for (j=0; j < (ssize_t) blocks; j++)
            {
              SHA3PermutationAfterXor(sha_info,p,72,sha_info->state);
              p+=576/8;
            }
            break;
          }
          case 832:
          {
            for (j=0; j < (ssize_t) blocks; j++)
            {
              SHA3PermutationAfterXor(sha_info,p,104,sha_info->state);
              p+=832/8;
            }
            break;
          }
          case 1024:
          {
            for (j=0; j < (ssize_t) blocks; j++)
            {
              SHA3PermutationAfterXor(sha_info,p,128,sha_info->state);
              p+=1024/8;
            }
            break;
          }
          case 1088:
          {
            for (j=0; j < (ssize_t) blocks; j++)
            {
              SHA3PermutationAfterXor(sha_info,p,136,sha_info->state);
              p+=1088/8;
            }
            break;
          }
          case 1152:
          {
            for (j=0; j < (ssize_t) blocks; j++)
            {
              SHA3PermutationAfterXor(sha_info,p,144,sha_info->state);
              p+=1152/8;
            }
            break;
          }
          case 1344:
          {
            for (j=0; j < (ssize_t) blocks; j++)
            {
              SHA3PermutationAfterXor(sha_info,p,168,sha_info->state);
              p+=1344/8;
            }
            break;
          }
          default:
          {
            for (j=0; j < (ssize_t) blocks; j++)
            {
              SHA3PermutationAfterXor(sha_info,sha_info->message,8*
                sha_info->rate/64,sha_info->state);
              p+=sha_info->rate/8;
            }
            break;
          }
        }
        i+=blocks*sha_info->rate;
      }
  }
  return(WizardTrue);
}

WizardExport WizardBooleanType UpdateSHA3(SHA3Info *sha_info,
  const StringInfo *message)
{
  WizardBooleanType
    status;

  assert(sha_info != (SHA3Info *) NULL);
  assert(sha_info->signature == WizardSignature);
  status=Absorb(sha_info,GetStringInfoDatum(message),8*
    GetStringInfoLength(message));
  return(status);
}
