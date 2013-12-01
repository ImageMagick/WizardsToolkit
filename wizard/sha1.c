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
%             Wizard's Toolkit Secure Hash Algorithm 1 Methods                %
%                                                                             %
%                             Software Design                                 %
%                                  Cristy                                     %
%                               March  2003                                   %
%                                                                             %
%                                                                             %
%  Copyright 1999-2014 ImageMagick Studio LLC, a non-profit organization      %
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
% See http://csrc.nist.gov/groups/ST/toolkit/index.html
%
*/

/*
  Include declarations.
*/
#include "wizard/studio.h"
#include "wizard/exception.h"
#include "wizard/exception-private.h"
#include "wizard/memory_.h"
#include "wizard/sha1.h"

/*
  Define declarations.
*/
#define SHA1Blocksize  64
#define SHA1Digestsize  20

/*
  Typedef declarations.
*/
struct _SHA1Info
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
  TransformSHA1(SHA1Info *);

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
%  AcquireSHA1Info() allocate the SHA1Info structure.
%
%  The format of the AcquireSHA1Info method is:
%
%      SHA1Info *AcquireSHA1Info(void)
%
*/
WizardExport SHA1Info *AcquireSHA1Info(void)
{
  SHA1Info
    *sha_info;

  unsigned int
    lsb_first;

  sha_info=(SHA1Info *) AcquireWizardMemory(sizeof(*sha_info));
  if (sha_info == (SHA1Info *) NULL)
    ThrowWizardFatalError(HashDomain,MemoryError);
  (void) ResetWizardMemory(sha_info,0,sizeof(*sha_info));
  sha_info->digestsize=SHA1Digestsize;
  sha_info->blocksize=SHA1Blocksize;
  sha_info->digest=AcquireStringInfo(SHA1Digestsize);
  sha_info->message=AcquireStringInfo(SHA1Blocksize);
  sha_info->accumulator=(unsigned int *) AcquireQuantumMemory(SHA1Blocksize,
    sizeof(*sha_info->accumulator));
  if (sha_info->accumulator == (unsigned int *) NULL)
    ThrowWizardFatalError(HashDomain,MemoryError);
  lsb_first=1;
  sha_info->lsb_first=(int)
    (*(char *) &lsb_first) == 1 ? WizardTrue : WizardFalse;
  sha_info->timestamp=time((time_t *) NULL);
  sha_info->signature=WizardSignature;
  InitializeSHA1(sha_info);
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
%  DestroySHA1Info() zeros memory associated with the SHA1Info structure.
%
%  The format of the DestroySHA1Info method is:
%
%      SHA1Info *DestroySHA1Info(SHA1Info *sha_info)
%
%  A description of each parameter follows:
%
%    o sha_info: The cipher sha_info.
%
*/
WizardExport SHA1Info *DestroySHA1Info(SHA1Info *sha_info)
{
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  assert(sha_info != (SHA1Info *) NULL);
  assert(sha_info->signature == WizardSignature);
  if (sha_info->accumulator != (unsigned int *) NULL)
    sha_info->accumulator=(unsigned int *)
      RelinquishWizardMemory(sha_info->accumulator);
  if (sha_info->message != (StringInfo *) NULL)
    sha_info->message=DestroyStringInfo(sha_info->message);
  if (sha_info->digest != (StringInfo *) NULL)
    sha_info->digest=DestroyStringInfo(sha_info->digest);
  sha_info->signature=(~WizardSignature);
  sha_info=(SHA1Info *) RelinquishWizardMemory(sha_info);
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
%  FinalizeSHA1() finalizes the SHA1 message accumulator computation.
%
%  The format of the FinalizeSHA1 method is:
%
%      WizardBooleanType FinalizeSHA1(SHA1Info *sha_info)
%
%  A description of each parameter follows:
%
%    o sha_info: The address of a structure of type SHA1Info.
%
*/
WizardExport WizardBooleanType FinalizeSHA1(SHA1Info *sha_info)
{
  register size_t
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
  assert(sha_info != (SHA1Info *) NULL);
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
      TransformSHA1(sha_info);
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
  TransformSHA1(sha_info);
  p=sha_info->accumulator;
  q=GetStringInfoDatum(sha_info->digest);
  for (i=0; i < (SHA1Digestsize/4); i++)
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
%   G e t S H A 1 B l o c k s i z e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetSHA1Blocksize() returns the SHA1 blocksize.
%
%  The format of the GetSHA1Blocksize method is:
%
%      unsigned int *GetSHA1Blocksize(const SHA1Info *sha1_info)
%
%  A description of each parameter follows:
%
%    o sha1_info: The sha1 info.
%
*/
WizardExport unsigned int GetSHA1Blocksize(const SHA1Info *sha1_info)
{
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  WizardAssert(CipherDomain,sha1_info != (SHA1Info *) NULL);
  WizardAssert(CipherDomain,sha1_info->signature == WizardSignature);
  return(sha1_info->blocksize);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t S H A 1 D i g e s t                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetSHA1Digest() returns the SHA1 digest.
%
%  The format of the GetSHA1Digest method is:
%
%      const StringInfo *GetSHA1Digest(const SHA1Info *sha1_info)
%
%  A description of each parameter follows:
%
%    o sha1_info: The sha1 info.
%
*/
WizardExport const StringInfo *GetSHA1Digest(const SHA1Info *sha1_info)
{
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  WizardAssert(HashDomain,sha1_info != (SHA1Info *) NULL);
  WizardAssert(HashDomain,sha1_info->signature == WizardSignature);
  return(sha1_info->digest);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t S H A 1 D i g e s t s i z e                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetSHA1Digestsize() returns the SHA1 digest size.
%
%  The format of the GetSHA1Digestsize method is:
%
%      unsigned int *GetSHA1Digestsize(const SHA1Info *sha1_info)
%
%  A description of each parameter follows:
%
%    o sha1_info: The sha1 info.
%
*/
WizardExport unsigned int GetSHA1Digestsize(const SHA1Info *sha1_info)
{
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  WizardAssert(CipherDomain,sha1_info != (SHA1Info *) NULL);
  WizardAssert(CipherDomain,sha1_info->signature == WizardSignature);
  return(sha1_info->digestsize);
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
%  IntializeSHA1() intializes the SHA1 accumulator.
%
%  The format of the DestroySHA1Info method is:
%
%      WizardBooleanType InitializeSHA1Info(SHA1Info *sha_info)
%
%  A description of each parameter follows:
%
%    o sha_info: The cipher sha_info.
%
*/
WizardExport WizardBooleanType InitializeSHA1(SHA1Info *sha_info)
{
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  assert(sha_info != (SHA1Info *) NULL);
  assert(sha_info->signature == WizardSignature);
  sha_info->accumulator[0]=0x67452301U;
  sha_info->accumulator[1]=0xefcdab89U;
  sha_info->accumulator[2]=0x98badcfeU;
  sha_info->accumulator[3]=0x10325476U;
  sha_info->accumulator[4]=0xc3d2e1f0U;
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
%  TransformSHA1() transforms the SHA1 message accumulator.
%
%  The format of the TransformSHA1 method is:
%
%      TransformSHA1(SHA1Info *sha_info)
%
%  A description of each parameter follows:
%
%    o sha_info: The address of a structure of type SHA1Info.
%
*/

static inline unsigned int Trunc32(const unsigned int x)
{
  return(x & 0xffffffffU);
}

static unsigned int RotateLeft(const unsigned int x,const unsigned int n)
{
  return(Trunc32((x << n) | (x >> (32-n))));
}

static void TransformSHA1(SHA1Info *sha_info)
{
  register ssize_t
    i;

  register unsigned char
    *p;

  register unsigned int
    *q;

  unsigned int
    A,
    B,
    C,
    D,
    E,
    shift,
    T,
    W[80];

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
  for (i=16; i < 80; i++)
  {
    W[i]=W[i-3] ^ W[i-8] ^ W[i-14] ^ W[i-16];
    W[i]=RotateLeft(W[i],1);
  }
  q=W;
  for (i=0; i < 20; i++)
  {
    T=Trunc32(RotateLeft(A,5)+((B & C) | (~B & D))+E+(*q)+0x5a827999U);
    E=D;
    D=C;
    C=RotateLeft(B,30);
    B=A;
    A=T;
    q++;
  }
  for ( ; i < 40; i++)
  {
    T=Trunc32(RotateLeft(A,5)+(B ^ C ^ D)+E+(*q)+0x6ed9eba1U);
    E=D;
    D=C;
    C=RotateLeft(B,30);
    B=A;
    A=T;
    q++;
  }
  for ( ; i < 60; i++)
  {
    T=Trunc32(RotateLeft(A,5)+((B & C) | (B & D) | (C & D))+E+(*q)+
      0x8F1bbcdcU);
    E=D;
    D=C;
    C=RotateLeft(B,30);
    B=A;
    A=T;
    q++;
  }
  for ( ; i < 80; i++)
  {
    T=Trunc32(RotateLeft(A,5)+(B ^ C ^ D)+E+(*q)+0xca62c1d6U);
    E=D;
    D=C;
    C=RotateLeft(B,30);
    B=A;
    A=T;
    q++;
  }
  /*
    Add registers back to accumulator.
  */
  sha_info->accumulator[0]=Trunc32(sha_info->accumulator[0]+A);
  sha_info->accumulator[1]=Trunc32(sha_info->accumulator[1]+B);
  sha_info->accumulator[2]=Trunc32(sha_info->accumulator[2]+C);
  sha_info->accumulator[3]=Trunc32(sha_info->accumulator[3]+D);
  sha_info->accumulator[4]=Trunc32(sha_info->accumulator[4]+E);
  /*
    Reset working registers.
  */
  A=0;
  B=0;
  C=0;
  D=0;
  E=0;
  T=0;
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
%  UpdateSHA1() updates the SHA1 message accumulator.
%
%  The format of the UpdateSHA1 method is:
%
%      WizardBooleanType UpdateSHA1(SHA1Info *sha_info,
%        const StringInfo *message)
%
%  A description of each parameter follows:
%
%    o sha_info: The address of a structure of type SHA1Info.
%
%    o message: The message
%
*/
WizardExport WizardBooleanType UpdateSHA1(SHA1Info *sha_info,
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
    Update the SHA1 accumulator.
  */
  assert(sha_info != (SHA1Info *) NULL);
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
      TransformSHA1(sha_info);
    }
  while (n >= GetStringInfoLength(sha_info->message))
  {
    SetStringInfoDatum(sha_info->message,p);
    p+=GetStringInfoLength(sha_info->message);
    n-=GetStringInfoLength(sha_info->message);
    TransformSHA1(sha_info);
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
