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
%             Wizard's Toolkit Secure Hash Algorithm 2-512 Methods            %
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
#include "wizard/sha2512.h"
/*
  Define declarations.
*/
#define SHA2512Blocksize  128
#define SHA2512Digestsize  64

/*
  Typedef declarations.
*/
struct _SHA2512Info
{   
  unsigned int
    digestsize,
    blocksize;

  StringInfo
    *digest,
    *message;

  WizardSizeType
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
  TransformSHA2512(SHA2512Info *);

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
%  AcquireSHA2512Info() allocate the SHA2512Info structure.
%
%  The format of the AcquireSHA2512Info method is:
%
%      SHA2512Info *AcquireSHA2512Info(void)
%
*/
WizardExport SHA2512Info *AcquireSHA2512Info(void)
{
  SHA2512Info
    *sha_info;

  unsigned int
    lsb_first;

  sha_info=(SHA2512Info *) AcquireWizardMemory(sizeof(*sha_info));
  if (sha_info == (SHA2512Info *) NULL)
    ThrowWizardFatalError(HashError,MemoryError);
  (void) ResetWizardMemory(sha_info,0,sizeof(*sha_info));
  sha_info->digestsize=SHA2512Digestsize;
  sha_info->blocksize=SHA2512Blocksize;
  sha_info->digest=AcquireStringInfo(SHA2512Digestsize);
  sha_info->message=AcquireStringInfo(SHA2512Blocksize);
  sha_info->accumulator=(WizardSizeType *) AcquireQuantumMemory(
    SHA2512Blocksize,sizeof(*sha_info->accumulator));
  if (sha_info->accumulator == (WizardSizeType *) NULL)
    ThrowWizardFatalError(HashError,MemoryError);
  lsb_first=1;
  sha_info->lsb_first=(int)
    (*(char *) &lsb_first) == 1 ? WizardTrue : WizardFalse;
  sha_info->timestamp=time((time_t *) NULL);
  sha_info->signature=WizardSignature;
  InitializeSHA2512(sha_info);
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
%  DestroySHA2512Info() zeros memory associated with the SHA2512Info structure.
%
%  The format of the DestroySHA2512Info method is:
%
%      SHA2512Info *DestroySHA2512Info(SHA2512Info *sha_info)
%
%  A description of each parameter follows:
%
%    o sha_info: The cipher sha_info.
%
*/
WizardExport SHA2512Info *DestroySHA2512Info(SHA2512Info *sha_info)
{
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  assert(sha_info != (SHA2512Info *) NULL);
  assert(sha_info->signature == WizardSignature);
  if (sha_info->accumulator != (WizardSizeType *) NULL)
    sha_info->accumulator=(WizardSizeType *)
      RelinquishWizardMemory(sha_info->accumulator);
  if (sha_info->message != (StringInfo *) NULL)
    sha_info->message=DestroyStringInfo(sha_info->message);
  if (sha_info->digest != (StringInfo *) NULL)
    sha_info->digest=DestroyStringInfo(sha_info->digest);
  sha_info->signature=(~WizardSignature);
  sha_info=(SHA2512Info *) RelinquishWizardMemory(sha_info);
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
%  FinalizeSHA2512() finalizes the SHA2512 message accumulator computation.
%
%  The format of the FinalizeSHA2512 method is:
%
%      WizardBooleanType FinalizeSHA2512(SHA2512Info *sha_info)
%
%  A description of each parameter follows:
%
%    o sha_info: The address of a structure of type SHA2512Info.
%
*/
WizardExport WizardBooleanType FinalizeSHA2512(SHA2512Info *sha_info)
{
  register ssize_t
    i;

  register unsigned char
    *q;

  register WizardSizeType
    *p;

  unsigned char
    *datum;

  WizardOffsetType
    count;

  WizardSizeType
    high_order,
    low_order;

  /*
    Add padding and return the message accumulator.
  */
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  assert(sha_info != (SHA2512Info *) NULL);
  assert(sha_info->signature == WizardSignature);
  low_order=sha_info->low_order;
  high_order=sha_info->high_order;
  count=(WizardOffsetType) ((low_order >> 3) & 0x7f);
  datum=GetStringInfoDatum(sha_info->message);
  datum[count++]=(unsigned char) 0x80;
  if (count <= (WizardOffsetType) (GetStringInfoLength(sha_info->message)-16))
    (void) ResetWizardMemory(datum+count,0,(size_t) (GetStringInfoLength(
      sha_info->message)-16-count));
  else
    {
      (void) ResetWizardMemory(datum+count,0,(size_t) (GetStringInfoLength(
        sha_info->message)-count));
      TransformSHA2512(sha_info);
      (void) ResetWizardMemory(datum,0,GetStringInfoLength(sha_info->message)-
        16);
    }
  datum[112]=(unsigned char) (high_order >> 56);
  datum[113]=(unsigned char) (high_order >> 48);
  datum[114]=(unsigned char) (high_order >> 40);
  datum[115]=(unsigned char) (high_order >> 32);
  datum[116]=(unsigned char) (high_order >> 24);
  datum[117]=(unsigned char) (high_order >> 16);
  datum[118]=(unsigned char) (high_order >> 8);
  datum[119]=(unsigned char) high_order;
  datum[120]=(unsigned char) (low_order >> 56);
  datum[121]=(unsigned char) (low_order >> 48);
  datum[122]=(unsigned char) (low_order >> 40);
  datum[123]=(unsigned char) (low_order >> 32);
  datum[124]=(unsigned char) (low_order >> 24);
  datum[125]=(unsigned char) (low_order >> 16);
  datum[126]=(unsigned char) (low_order >> 8);
  datum[127]=(unsigned char) low_order;
  TransformSHA2512(sha_info);
  p=sha_info->accumulator;
  q=GetStringInfoDatum(sha_info->digest);
  for (i=0; i < (SHA2512Digestsize/8); i++)
  {
    *q++=(unsigned char) ((*p >> 56) & 0xff);
    *q++=(unsigned char) ((*p >> 48) & 0xff);
    *q++=(unsigned char) ((*p >> 40) & 0xff);
    *q++=(unsigned char) ((*p >> 32) & 0xff);
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
%   G e t S H A 5 1 2 B l o c k s i z e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetSHA2512Blocksize() returns the SHA2512 blocksize.
%
%  The format of the GetSHA2512Blocksize method is:
%
%      unsigned int *GetSHA2512Blocksize(const SHA2512Info *sha2512_info)
%
%  A description of each parameter follows:
%
%    o sha2512_info: The shaa info.
%
*/
WizardExport unsigned int GetSHA2512Blocksize(const SHA2512Info *sha2512_info)
{
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  WizardAssert(CipherDomain,sha2512_info != (SHA2512Info *) NULL);
  WizardAssert(CipherDomain,sha2512_info->signature == WizardSignature);
  return(sha2512_info->blocksize);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t S H A 5 1 2 D i g e s t                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetSHA2512Digest() returns the SHA2512 digest.
%
%  The format of the GetSHA2512Digest method is:
%
%      const StringInfo *GetSHA2512Digest(const SHA2512Info *sha2512_info)
%
%  A description of each parameter follows:
%
%    o sha2512_info: The shaa info.
%
*/
WizardExport const StringInfo *GetSHA2512Digest(const SHA2512Info *sha2512_info)
{
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  WizardAssert(HashDomain,sha2512_info != (SHA2512Info *) NULL);
  WizardAssert(HashDomain,sha2512_info->signature == WizardSignature);
  return(sha2512_info->digest);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t S H A 5 1 2 D i g e s t s i z e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetSHA2512Digestsize() returns the SHA2512 digest size.
%
%  The format of the GetSHA2512Digestsize method is:
%
%      unsigned int *GetSHA2512Digestsize(const SHA2512Info *sha2512_info)
%
%  A description of each parameter follows:
%
%    o sha2512_info: The shaa info.
%
*/
WizardExport unsigned int GetSHA2512Digestsize(const SHA2512Info *sha2512_info)
{
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  WizardAssert(CipherDomain,sha2512_info != (SHA2512Info *) NULL);
  WizardAssert(CipherDomain,sha2512_info->signature == WizardSignature);
  return(sha2512_info->digestsize);
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
%  IntializeSHA2512() intializes the SHA2512 accumulator.
%
%  The format of the DestroySHA2512Info method is:
%
%      WizardBooleanType InitializeSHA2512Info(SHA2512Info *sha_info)
%
%  A description of each parameter follows:
%
%    o sha_info: The cipher sha_info.
%
*/
WizardExport WizardBooleanType InitializeSHA2512(SHA2512Info *sha_info)
{
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  assert(sha_info != (SHA2512Info *) NULL);
  assert(sha_info->signature == WizardSignature);
  sha_info->accumulator[0]=WizardULLConstant(0x6a09e667f3bcc908);
  sha_info->accumulator[1]=WizardULLConstant(0xbb67ae8584caa73b);
  sha_info->accumulator[2]=WizardULLConstant(0x3c6ef372fe94f82b);
  sha_info->accumulator[3]=WizardULLConstant(0xa54ff53a5f1d36f1);
  sha_info->accumulator[4]=WizardULLConstant(0x510e527fade682d1);
  sha_info->accumulator[5]=WizardULLConstant(0x9b05688c2b3e6c1f);
  sha_info->accumulator[6]=WizardULLConstant(0x1f83d9abfb41bd6b);
  sha_info->accumulator[7]=WizardULLConstant(0x5be0cd19137e2179);
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
%  TransformSHA2512() transforms the SHA2512 message accumulator.
%
%  The format of the TransformSHA2512 method is:
%
%      TransformSHA2512(SHA2512Info *sha_info)
%
%  A description of each parameter follows:
%
%    o sha_info: The address of a structure of type SHA2512Info.
%
*/

static inline WizardSizeType Ch(const WizardSizeType x,const WizardSizeType y,
  const WizardSizeType z)
{
  return((x & y) ^ (~x & z));
}

static inline WizardSizeType Maj(const WizardSizeType x,const WizardSizeType y,
  const WizardSizeType z)
{
  return((x & y) ^ (x & z) ^ (y & z));
}

static inline WizardSizeType Trunc64(const WizardSizeType x)
{
  return((WizardSizeType) (x & WizardULLConstant(0xffffffffffffffff)));
}

static inline WizardSizeType RotateRight(const WizardSizeType x,
  const WizardSizeType n)
{
  return(Trunc64((x >> n) | (x << (64-n))));
}

static void TransformSHA2512(SHA2512Info *sha_info)
{
#define Sigma0(x)  (RotateRight(x,1) ^ RotateRight(x,8) ^ Trunc64((x) >> 7))
#define Sigma1(x)  (RotateRight(x,19) ^ RotateRight(x,61) ^ Trunc64((x) >> 6))
#define Suma0(x)  (RotateRight(x,28) ^ RotateRight(x,34) ^ RotateRight(x,39))
#define Suma1(x)  (RotateRight(x,14) ^ RotateRight(x,18) ^ RotateRight(x,41))

  ssize_t
    j;

  register ssize_t
    i;

  register unsigned char
    *p;

  static WizardSizeType
    K[80] =
    {
      WizardULLConstant(0x428a2f98d728ae22),
      WizardULLConstant(0x7137449123ef65cd),
      WizardULLConstant(0xb5c0fbcfec4d3b2f),
      WizardULLConstant(0xe9b5dba58189dbbc),
      WizardULLConstant(0x3956c25bf348b538),
      WizardULLConstant(0x59f111f1b605d019),
      WizardULLConstant(0x923f82a4af194f9b),
      WizardULLConstant(0xab1c5ed5da6d8118),
      WizardULLConstant(0xd807aa98a3030242),
      WizardULLConstant(0x12835b0145706fbe),
      WizardULLConstant(0x243185be4ee4b28c),
      WizardULLConstant(0x550c7dc3d5ffb4e2),
      WizardULLConstant(0x72be5d74f27b896f),
      WizardULLConstant(0x80deb1fe3b1696b1),
      WizardULLConstant(0x9bdc06a725c71235),
      WizardULLConstant(0xc19bf174cf692694),
      WizardULLConstant(0xe49b69c19ef14ad2),
      WizardULLConstant(0xefbe4786384f25e3),
      WizardULLConstant(0x0fc19dc68b8cd5b5),
      WizardULLConstant(0x240ca1cc77ac9c65),
      WizardULLConstant(0x2de92c6f592b0275),
      WizardULLConstant(0x4a7484aa6ea6e483),
      WizardULLConstant(0x5cb0a9dcbd41fbd4),
      WizardULLConstant(0x76f988da831153b5),
      WizardULLConstant(0x983e5152ee66dfab),
      WizardULLConstant(0xa831c66d2db43210),
      WizardULLConstant(0xb00327c898fb213f),
      WizardULLConstant(0xbf597fc7beef0ee4),
      WizardULLConstant(0xc6e00bf33da88fc2),
      WizardULLConstant(0xd5a79147930aa725),
      WizardULLConstant(0x06ca6351e003826f),
      WizardULLConstant(0x142929670a0e6e70),
      WizardULLConstant(0x27b70a8546d22ffc),
      WizardULLConstant(0x2e1b21385c26c926),
      WizardULLConstant(0x4d2c6dfc5ac42aed),
      WizardULLConstant(0x53380d139d95b3df),
      WizardULLConstant(0x650a73548baf63de),
      WizardULLConstant(0x766a0abb3c77b2a8),
      WizardULLConstant(0x81c2c92e47edaee6),
      WizardULLConstant(0x92722c851482353b),
      WizardULLConstant(0xa2bfe8a14cf10364),
      WizardULLConstant(0xa81a664bbc423001),
      WizardULLConstant(0xc24b8b70d0f89791),
      WizardULLConstant(0xc76c51a30654be30),
      WizardULLConstant(0xd192e819d6ef5218),
      WizardULLConstant(0xd69906245565a910),
      WizardULLConstant(0xf40e35855771202a),
      WizardULLConstant(0x106aa07032bbd1b8),
      WizardULLConstant(0x19a4c116b8d2d0c8),
      WizardULLConstant(0x1e376c085141ab53),
      WizardULLConstant(0x2748774cdf8eeb99),
      WizardULLConstant(0x34b0bcb5e19b48a8),
      WizardULLConstant(0x391c0cb3c5c95a63),
      WizardULLConstant(0x4ed8aa4ae3418acb),
      WizardULLConstant(0x5b9cca4f7763e373),
      WizardULLConstant(0x682e6ff3d6b2b8a3),
      WizardULLConstant(0x748f82ee5defb2fc),
      WizardULLConstant(0x78a5636f43172f60),
      WizardULLConstant(0x84c87814a1f0ab72),
      WizardULLConstant(0x8cc702081a6439ec),
      WizardULLConstant(0x90befffa23631e28),
      WizardULLConstant(0xa4506cebde82bde9),
      WizardULLConstant(0xbef9a3f7b2c67915),
      WizardULLConstant(0xc67178f2e372532b),
      WizardULLConstant(0xca273eceea26619c),
      WizardULLConstant(0xd186b8c721c0c207),
      WizardULLConstant(0xeada7dd6cde0eb1e),
      WizardULLConstant(0xf57d4f7fee6ed178),
      WizardULLConstant(0x06f067aa72176fba),
      WizardULLConstant(0x0a637dc5a2c898a6),
      WizardULLConstant(0x113f9804bef90dae),
      WizardULLConstant(0x1b710b35131c471b),
      WizardULLConstant(0x28db77f523047d84),
      WizardULLConstant(0x32caab7b40c72493),
      WizardULLConstant(0x3c9ebe0a15c9bebc),
      WizardULLConstant(0x431d67c49c100d4c),
      WizardULLConstant(0x4cc5d4becb3e42b6),
      WizardULLConstant(0x597f299cfc657e2a),
      WizardULLConstant(0x5fcb6fab3ad6faec),
      WizardULLConstant(0x6c44198c4a475817)
    };  /* 64-bit fractional part of the cube root of the first 64 primes */

  WizardSizeType
    A,
    B,
    C,
    D,
    E,
    F,
    G,
    H,
    T,
    T1,
    T2,
    W[80];

  p=GetStringInfoDatum(sha_info->message);
  if (sha_info->lsb_first == WizardFalse)
    for (i=0; i < 16; i++)
    {
      T=(*((WizardSizeType *) p));
      p+=8;
      W[i]=(T);
    }
  else
    for (i=0; i < 16; i++)
    {
      T=(*((WizardSizeType *) p));
      p+=8;
      W[i]=(WizardSizeType)
        (((T << 56) & WizardULLConstant(0xff00000000000000)) |
         ((T << 40) & WizardULLConstant(0x00ff000000000000)) |
         ((T << 24) & WizardULLConstant(0x0000ff0000000000)) |
         ((T <<  8) & WizardULLConstant(0x000000ff00000000)) |
         ((T >>  8) & WizardULLConstant(0x00000000ff000000)) |
         ((T >> 24) & WizardULLConstant(0x0000000000ff0000)) |
         ((T >> 40) & WizardULLConstant(0x000000000000ff00)) |
         ((T >> 56) & WizardULLConstant(0x00000000000000ff)));
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
  for (i=16; i < 80; i++)
    W[i]=Trunc64(Sigma1(W[i-2])+W[i-7]+Sigma0(W[i-15])+W[i-16]);
  for (j=0; j < 80; j++)
  {
    T1=Trunc64(H+Suma1(E)+Ch(E,F,G)+K[j]+W[j]);
    T2=Trunc64(Suma0(A)+Maj(A,B,C));
    H=G;
    G=F;
    F=E;
    E=Trunc64(D+T1);
    D=C;
    C=B;
    B=A;
    A=Trunc64(T1+T2);
  }
  /*
    Add registers back to accumulator.
  */
  sha_info->accumulator[0]=Trunc64(sha_info->accumulator[0]+A);
  sha_info->accumulator[1]=Trunc64(sha_info->accumulator[1]+B);
  sha_info->accumulator[2]=Trunc64(sha_info->accumulator[2]+C);
  sha_info->accumulator[3]=Trunc64(sha_info->accumulator[3]+D);
  sha_info->accumulator[4]=Trunc64(sha_info->accumulator[4]+E);
  sha_info->accumulator[5]=Trunc64(sha_info->accumulator[5]+F);
  sha_info->accumulator[6]=Trunc64(sha_info->accumulator[6]+G);
  sha_info->accumulator[7]=Trunc64(sha_info->accumulator[7]+H);
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
%  UpdateSHA2512() updates the SHA2512 message accumulator.
%
%  The format of the UpdateSHA2512 method is:
%
%      WizardBooleanType UpdateSHA2512(SHA2512Info *sha_info,
%        const StringInfo *message)
%
%  A description of each parameter follows:
%
%    o sha_info: The address of a structure of type SHA2512Info.
%
%    o message: The message.
%
*/
WizardExport WizardBooleanType UpdateSHA2512(SHA2512Info *sha_info,
  const StringInfo *message)
{
  register size_t
    i;

  register unsigned char
    *p;

  size_t
    n;

  WizardSizeType
    length;

  /*
    Update the SHA2512 accumulator.
  */
  assert(sha_info != (SHA2512Info *) NULL);
  assert(sha_info->signature == WizardSignature);
  n=GetStringInfoLength(message);
  length=Trunc64(sha_info->low_order+((WizardSizeType) n << 3));
  if (length < sha_info->low_order)
    sha_info->high_order++;
  sha_info->low_order=length;
  sha_info->high_order+=(WizardSizeType) n >> 61;
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
      TransformSHA2512(sha_info);
    }
  while (n >= GetStringInfoLength(sha_info->message))
  {
    SetStringInfoDatum(sha_info->message,p);
    p+=GetStringInfoLength(sha_info->message);
    n-=GetStringInfoLength(sha_info->message);
    TransformSHA2512(sha_info);
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
