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
%             Wizard's Toolkit Secure Hash Algorithm 2-256 Methods            %
%                                                                             %
%                             Software Design                                 %
%                               John Cristy                                   %
%                               March  2003                                   %
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
% See http://csrc.nist.gov/groups/ST/toolkit/index.html.
%
*/

/*
  Include declarations.
*/
#include "wizard/studio.h"
#include "wizard/exception.h"
#include "wizard/exception-private.h"
#include "wizard/memory_.h"
#include "wizard/sha2256.h"
/*
  Define declarations.
*/
#define SHA2256Blocksize  64
#define SHA2256Digestsize  32

/*
  Typedef declarations.
*/
struct _SHA2256Info
{   
  unsigned int
    digestsize,
    blocksize;

  StringInfo
    *digest,
    *message;

  unsigned int
    *accumulator,
    low_order,
    high_order;

  size_t
    offset;

  WizardBooleanType
    lsb_first;

  time_t
    timestamp;

  size_t
    signature;
};

/*
  Forward declarations.
*/
static void
  TransformSHA2256(SHA2256Info *);

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
%  AcquireSHA2256Info() allocate the SHA2256Info structure.
%
%  The format of the AcquireSHA2256Info method is:
%
%      SHA2256Info *AcquireSHA2256Info(void)
%
*/
WizardExport SHA2256Info *AcquireSHA2256Info(void)
{
  SHA2256Info
    *sha_info;

  unsigned int
    lsb_first;

  sha_info=(SHA2256Info *) AcquireWizardMemory(sizeof(*sha_info));
  if (sha_info == (SHA2256Info *) NULL)
    ThrowWizardFatalError(HashError,MemoryError);
  (void) ResetWizardMemory(sha_info,0,sizeof(*sha_info));
  sha_info->digestsize=SHA2256Digestsize;
  sha_info->blocksize=SHA2256Blocksize;
  sha_info->digest=AcquireStringInfo(SHA2256Digestsize);
  sha_info->message=AcquireStringInfo(SHA2256Blocksize);
  sha_info->accumulator=(unsigned int *) AcquireQuantumMemory(SHA2256Blocksize,
    sizeof(*sha_info->accumulator));
  if (sha_info->accumulator == (unsigned int *) NULL)
    ThrowWizardFatalError(HashError,MemoryError);
  lsb_first=1;
  sha_info->lsb_first=(int) (*(char *) &lsb_first) == 1 ? WizardTrue :
    WizardFalse;
  sha_info->timestamp=time((time_t *) NULL);
  sha_info->signature=WizardSignature;
  InitializeSHA2256(sha_info);
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
%  DestroySHA2256Info() zeros memory associated with the SHA2256Info structure.
%
%  The format of the DestroySHA2256Info method is:
%
%      SHA2256Info *DestroySHA2256Info(SHA2256Info *sha_info)
%
%  A description of each parameter follows:
%
%    o sha_info: The cipher sha_info.
%
*/
WizardExport SHA2256Info *DestroySHA2256Info(SHA2256Info *sha_info)
{
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  assert(sha_info != (SHA2256Info *) NULL);
  assert(sha_info->signature == WizardSignature);
  if (sha_info->accumulator != (unsigned int *) NULL)
    sha_info->accumulator=(unsigned int *) RelinquishWizardMemory(
      sha_info->accumulator);
  if (sha_info->message != (StringInfo *) NULL)
    sha_info->message=DestroyStringInfo(sha_info->message);
  if (sha_info->digest != (StringInfo *) NULL)
    sha_info->digest=DestroyStringInfo(sha_info->digest);
  sha_info->signature=(~WizardSignature);
  sha_info=(SHA2256Info *) RelinquishWizardMemory(sha_info);
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
%  FinalizeSHA2256() finalizes the SHA2256 message accumulator computation.
%
%  The format of the FinalizeSHA2256 method is:
%
%      WizardBooleanType FinalizeSHA2256(SHA2256Info *sha_info)
%
%  A description of each parameter follows:
%
%    o sha_info: The address of a structure of type SHA2256Info.
%
*/
WizardExport WizardBooleanType FinalizeSHA2256(SHA2256Info *sha_info)
{
  register ssize_t
    i;

  register unsigned char
    *q;

  register unsigned int
    *p;

  ssize_t
    count;

  unsigned char
    *datum;

  unsigned int
    high_order,
    low_order;

  /*
    Add padding and return the message accumulator.
  */
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  assert(sha_info != (SHA2256Info *) NULL);
  assert(sha_info->signature == WizardSignature);
  low_order=sha_info->low_order;
  high_order=sha_info->high_order;
  count=(ssize_t) ((low_order >> 3) & 0x3f);
  datum=GetStringInfoDatum(sha_info->message);
  datum[count++]=(unsigned char) 0x80;
  if (count <= (ssize_t) (GetStringInfoLength(sha_info->message)-8))
    (void) ResetWizardMemory(datum+count,0,GetStringInfoLength(
      sha_info->message)-8-count);
  else
    {
      (void) ResetWizardMemory(datum+count,0,GetStringInfoLength(
        sha_info->message)-count);
      TransformSHA2256(sha_info);
      (void) ResetWizardMemory(datum,0,GetStringInfoLength(sha_info->message)-
        8);
    }
  datum[56]=(unsigned char) (high_order >> 24);
  datum[57]=(unsigned char) (high_order >> 16);
  datum[58]=(unsigned char) (high_order >> 8);
  datum[59]=(unsigned char) high_order;
  datum[60]=(unsigned char) (low_order >> 24);
  datum[61]=(unsigned char) (low_order >> 16);
  datum[62]=(unsigned char) (low_order >> 8);
  datum[63]=(unsigned char) low_order;
  TransformSHA2256(sha_info);
  p=sha_info->accumulator;
  q=GetStringInfoDatum(sha_info->digest);
  for (i=0; i < (SHA2256Digestsize/4); i++)
  {
    *q++=(unsigned char) ((*p >> 24) & 0xff);
    *q++=(unsigned char) ((*p >> 16) & 0xff);
    *q++=(unsigned char) ((*p >> 8) & 0xff);
    *q++=(unsigned char) (*p & 0xff);
    p++;
  }
  /*
    Reset working registers.
  */
  count=0;
  high_order=0;
  low_order=0;
  return(WizardTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t S H A 2 5 6 B l o c k s i z e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetSHA2256Blocksize() returns the SHA2256 blocksize.
%
%  The format of the GetSHA2256Blocksize method is:
%
%      unsigned int *GetSHA2256Blocksize(const SHA2256Info *sha2256_info)
%
%  A description of each parameter follows:
%
%    o sha2256_info: The sha2256 info.
%
*/
WizardExport unsigned int GetSHA2256Blocksize(const SHA2256Info *sha2256_info)
{
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  WizardAssert(CipherDomain,sha2256_info != (SHA2256Info *) NULL);
  WizardAssert(CipherDomain,sha2256_info->signature == WizardSignature);
  return(sha2256_info->blocksize);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t S H A 2 5 6 D i g e s t                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetSHA2256Digest() returns the SHA2256 digest.
%
%  The format of the GetSHA2256Digest method is:
%
%      const StringInfo *GetSHA2256Digest(const SHA2256Info *sha2256_info)
%
%  A description of each parameter follows:
%
%    o sha2256_info: The sha2256 info.
%
*/
WizardExport const StringInfo *GetSHA2256Digest(
  const SHA2256Info *sha2256_info)
{
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  WizardAssert(HashDomain,sha2256_info != (SHA2256Info *) NULL);
  WizardAssert(HashDomain,sha2256_info->signature == WizardSignature);
  return(sha2256_info->digest);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t S H A 2 5 6 D i g e s t s i z e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetSHA2256Digestsize() returns the SHA2256 digest size.
%
%  The format of the GetSHA2256Digestsize method is:
%
%      unsigned int *GetSHA2256Digestsize(const SHA2256Info *sha2256_info)
%
%  A description of each parameter follows:
%
%    o sha2256_info: The sha2256 info.
%
*/
WizardExport unsigned int GetSHA2256Digestsize(const SHA2256Info *sha2256_info)
{
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  WizardAssert(CipherDomain,sha2256_info != (SHA2256Info *) NULL);
  WizardAssert(CipherDomain,sha2256_info->signature == WizardSignature);
  return(sha2256_info->digestsize);
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
%  IntializeSHA2256() intializes the SHA2256 accumulator.
%
%  The format of the DestroySHA2256Info method is:
%
%      WizardBooleanType InitializeSHA2256Info(SHA2256Info *sha_info)
%
%  A description of each parameter follows:
%
%    o sha_info: The cipher sha_info.
%
*/
WizardExport WizardBooleanType InitializeSHA2256(SHA2256Info *sha_info)
{
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  assert(sha_info != (SHA2256Info *) NULL);
  assert(sha_info->signature == WizardSignature);
  sha_info->accumulator[0]=0x6a09e667U;
  sha_info->accumulator[1]=0xbb67ae85U;
  sha_info->accumulator[2]=0x3c6ef372U;
  sha_info->accumulator[3]=0xa54ff53aU;
  sha_info->accumulator[4]=0x510e527fU;
  sha_info->accumulator[5]=0x9b05688cU;
  sha_info->accumulator[6]=0x1f83d9abU;
  sha_info->accumulator[7]=0x5be0cd19U;
  sha_info->low_order=0;
  sha_info->high_order=0;
  sha_info->offset=0;
  return(WizardTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   T r a n s f o r m S H A                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  TransformSHA2256() transforms the SHA2256 message accumulator.
%
%  The format of the TransformSHA2256 method is:
%
%      TransformSHA2256(SHA2256Info *sha_info)
%
%  A description of each parameter follows:
%
%    o sha_info: The address of a structure of type SHA2256Info.
%
*/

static inline unsigned int Ch(const unsigned int x,const unsigned int y,
  const unsigned int z)
{
  return((x & y) ^ (~x & z));
}

static inline unsigned int Maj(const unsigned int x,const unsigned int y,
  const unsigned int z)
{
  return((x & y) ^ (x & z) ^ (y & z));
}

static inline unsigned int Trunc32(const unsigned int x)
{
  return((unsigned int) (x & 0xffffffffU));
}

static unsigned int RotateRight(const unsigned int x,const unsigned int n)
{
  return(Trunc32((x >> n) | (x << (32-n))));
}

static void TransformSHA2256(SHA2256Info *sha_info)
{
#define Sigma0(x)  (RotateRight(x,7) ^ RotateRight(x,18) ^ Trunc32((x) >> 3))
#define Sigma1(x)  (RotateRight(x,17) ^ RotateRight(x,19) ^ Trunc32((x) >> 10))
#define Suma0(x)  (RotateRight(x,2) ^ RotateRight(x,13) ^ RotateRight(x,22))
#define Suma1(x)  (RotateRight(x,6) ^ RotateRight(x,11) ^ RotateRight(x,25))

  ssize_t
    j;

  register ssize_t
    i;

  register unsigned char
    *p;

  static unsigned int
    K[64] =
    {
      0x428a2f98U, 0x71374491U, 0xb5c0fbcfU, 0xe9b5dba5U, 0x3956c25bU,
      0x59f111f1U, 0x923f82a4U, 0xab1c5ed5U, 0xd807aa98U, 0x12835b01U,
      0x243185beU, 0x550c7dc3U, 0x72be5d74U, 0x80deb1feU, 0x9bdc06a7U,
      0xc19bf174U, 0xe49b69c1U, 0xefbe4786U, 0x0fc19dc6U, 0x240ca1ccU,
      0x2de92c6fU, 0x4a7484aaU, 0x5cb0a9dcU, 0x76f988daU, 0x983e5152U,
      0xa831c66dU, 0xb00327c8U, 0xbf597fc7U, 0xc6e00bf3U, 0xd5a79147U,
      0x06ca6351U, 0x14292967U, 0x27b70a85U, 0x2e1b2138U, 0x4d2c6dfcU,
      0x53380d13U, 0x650a7354U, 0x766a0abbU, 0x81c2c92eU, 0x92722c85U,
      0xa2bfe8a1U, 0xa81a664bU, 0xc24b8b70U, 0xc76c51a3U, 0xd192e819U,
      0xd6990624U, 0xf40e3585U, 0x106aa070U, 0x19a4c116U, 0x1e376c08U,
      0x2748774cU, 0x34b0bcb5U, 0x391c0cb3U, 0x4ed8aa4aU, 0x5b9cca4fU,
      0x682e6ff3U, 0x748f82eeU, 0x78a5636fU, 0x84c87814U, 0x8cc70208U,
      0x90befffaU, 0xa4506cebU, 0xbef9a3f7U, 0xc67178f2U
    };  /* 32-bit fractional part of the cube root of the first 64 primes */

  unsigned int
    A,
    B,
    C,
    D,
    E,
    F,
    G,
    H,
    shift,
    T,
    T1,
    T2,
    W[64];

  shift=32;
  p=GetStringInfoDatum(sha_info->message);
  if (sha_info->lsb_first == WizardFalse)
    {
      if (sizeof(unsigned int) <= 4)
        for (i=0; i < 16; i++)
        {
          T=(*((unsigned int *) p));
          p+=4;
          W[i]=Trunc32(T);
        }
      else
        for (i=0; i < 16; i+=2)
        {
          T=(*((unsigned int *) p));
          p+=8;
          W[i]=Trunc32(T >> shift);
          W[i+1]=Trunc32(T);
        }
    }
  else
    if (sizeof(unsigned int) <= 4)
      for (i=0; i < 16; i++)
      {
        T=(*((unsigned int *) p));
        p+=4;
        W[i]=((T << 24) & 0xff000000) | ((T << 8) & 0x00ff0000) |
          ((T >> 8) & 0x0000ff00) | ((T >> 24) & 0x000000ff);
      }
    else
      for (i=0; i < 16; i+=2)
      {
        T=(*((unsigned int *) p));
        p+=8;
        W[i]=((T << 24) & 0xff000000) | ((T << 8) & 0x00ff0000) |
          ((T >> 8) & 0x0000ff00) | ((T >> 24) & 0x000000ff);
        T>>=shift;
        W[i+1]=((T << 24) & 0xff000000) | ((T << 8) & 0x00ff0000) |
          ((T >> 8) & 0x0000ff00) | ((T >> 24) & 0x000000ff);
      }
  /*
    Copy accumulator to registers.
  */
  A=sha_info->accumulator[0];
  B=sha_info->accumulator[1];
  C=sha_info->accumulator[2];
  D=sha_info->accumulator[3];
  E=sha_info->accumulator[4];
  F=sha_info->accumulator[5];
  G=sha_info->accumulator[6];
  H=sha_info->accumulator[7];
  for (i=16; i < 64; i++)
    W[i]=Trunc32(Sigma1(W[i-2])+W[i-7]+Sigma0(W[i-15])+W[i-16]);
  for (j=0; j < 64; j++)
  {
    T1=Trunc32(H+Suma1(E)+Ch(E,F,G)+K[j]+W[j]);
    T2=Trunc32(Suma0(A)+Maj(A,B,C));
    H=G;
    G=F;
    F=E;
    E=Trunc32(D+T1);
    D=C;
    C=B;
    B=A;
    A=Trunc32(T1+T2);
  }
  /*
    Add registers back to accumulator.
  */
  sha_info->accumulator[0]=Trunc32(sha_info->accumulator[0]+A);
  sha_info->accumulator[1]=Trunc32(sha_info->accumulator[1]+B);
  sha_info->accumulator[2]=Trunc32(sha_info->accumulator[2]+C);
  sha_info->accumulator[3]=Trunc32(sha_info->accumulator[3]+D);
  sha_info->accumulator[4]=Trunc32(sha_info->accumulator[4]+E);
  sha_info->accumulator[5]=Trunc32(sha_info->accumulator[5]+F);
  sha_info->accumulator[6]=Trunc32(sha_info->accumulator[6]+G);
  sha_info->accumulator[7]=Trunc32(sha_info->accumulator[7]+H);
  /*
    Reset working registers.
  */
  A=0;
  B=0;
  C=0;
  D=0;
  E=0;
  F=0;
  G=0;
  H=0;
  T=0;
  T1=0;
  T2=0;
  (void) ResetWizardMemory(W,0,sizeof(W));
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
%  UpdateSHA2256() updates the SHA2256 message accumulator.
%
%  The format of the UpdateSHA2256 method is:
%
%      WizardBooleanType UpdateSHA2256(SHA2256Info *sha_info,
%        const StringInfo *message)
%
%  A description of each parameter follows:
%
%    o sha_info: The address of a structure of type SHA2256Info.
%
%    o message: The message.
%
*/
WizardExport WizardBooleanType UpdateSHA2256(SHA2256Info *sha_info,
  const StringInfo *message)
{
  register size_t
    i;

  register unsigned char
    *p;

  size_t
    n;

  unsigned int
    length;

  /*
    Update the SHA2256 accumulator.
  */
  assert(sha_info != (SHA2256Info *) NULL);
  assert(sha_info->signature == WizardSignature);
  n=GetStringInfoLength(message);
  length=Trunc32((unsigned int) (sha_info->low_order+(n << 3)));
  if (length < sha_info->low_order)
    sha_info->high_order++;
  sha_info->low_order=length;
  sha_info->high_order+=(unsigned int) n >> 29;
  p=GetStringInfoDatum(message);
  if (sha_info->offset != 0)
    {
      i=GetStringInfoLength(sha_info->message)-sha_info->offset;
      if (i > n)
        i=n;
      (void) CopyWizardMemory(GetStringInfoDatum(sha_info->message)+
        sha_info->offset,p,i);
      n-=i;
      p+=i;
      sha_info->offset+=i;
      if (sha_info->offset != GetStringInfoLength(sha_info->message))
        return(WizardTrue);
      TransformSHA2256(sha_info);
    }
  while (n >= GetStringInfoLength(sha_info->message))
  {
    SetStringInfoDatum(sha_info->message,p);
    p+=GetStringInfoLength(sha_info->message);
    n-=GetStringInfoLength(sha_info->message);
    TransformSHA2256(sha_info);
  }
  (void) CopyWizardMemory(GetStringInfoDatum(sha_info->message),p,n);
  sha_info->offset=n;
  /*
    Reset working registers.
  */
  i=0;
  n=0;
  length=0;
  return(WizardTrue);
}
