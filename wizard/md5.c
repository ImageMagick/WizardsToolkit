/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                           M   M  DDDD   55555                               %
%                           MM MM  D   D  5                                   %
%                           M M M  D   D  55555                               %
%                           M   M  D   D      5                               %
%                           M   M  DDDD   55555                               %
%                                                                             %
%                                                                             %
%             Wizard's Toolkit Message Digest Algorithm-5 Methods             %
%                                                                             %
%                             Software Design                                 %
%                                 Cristy                                      %
%                               March  2003                                   %
%                                                                             %
%                                                                             %
%  Copyright 1999-2016 ImageMagick Studio LLC, a non-profit organization      %
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
*/

/*
  Include declarations.
*/
#include "wizard/studio.h"
#include "wizard/exception.h"
#include "wizard/exception-private.h"
#include "wizard/memory_.h"
#include "wizard/md5.h"

/*
  Define declarations.
*/
#define MD5Blocksize  64
#define MD5Digestsize  16

/*
  Typedef declarations.
*/
struct _MD5Info
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

  time_t
    timestamp;

  size_t
    signature;
};

/*
  Forward declaraction.
*/
static void
  TransformMD5(MD5Info *,const unsigned int *);

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   A c q u i r e M D 5 I n f o                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquireMD5Info() allocate the MD5Info structure.
%
%  The format of the AcquireMD5Info method is:
%
%      MD5Info *AcquireMD5Info(void)
%
*/
WizardExport MD5Info *AcquireMD5Info(void)
{
  MD5Info
    *md5_info;

  md5_info=(MD5Info *) AcquireWizardMemory(sizeof(*md5_info));
  if (md5_info == (MD5Info *) NULL)
    ThrowWizardFatalError(HashDomain,MemoryError);
  (void) ResetWizardMemory(md5_info,0,sizeof(*md5_info));
  md5_info->digestsize=MD5Digestsize;
  md5_info->blocksize=MD5Blocksize;
  md5_info->digest=AcquireStringInfo(MD5Digestsize);
  md5_info->message=AcquireStringInfo(MD5Blocksize);
  md5_info->accumulator=(unsigned int *) AcquireQuantumMemory(4UL,
    sizeof(*md5_info->accumulator));
  if (md5_info->accumulator == (unsigned int *) NULL)
    ThrowWizardFatalError(HashDomain,MemoryError);
  md5_info->timestamp=time((time_t *) NULL);
  md5_info->signature=WizardSignature;
  return(md5_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D e s t r o y M D 5 I n f o                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DestroyMD5Info() zeros memory associated with the MD5Info structure.
%
%  The format of the DestroyMD5Info method is:
%
%      MD5Info *DestroyMD5Info(MD5Info *md5_info)
%
%  A description of each parameter follows:
%
%    o md5_info: The cipher md5_info.
%
*/
WizardExport MD5Info *DestroyMD5Info(MD5Info *md5_info)
{
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  assert(md5_info != (MD5Info *) NULL);
  assert(md5_info->signature == WizardSignature);
  if (md5_info->accumulator != (unsigned int *) NULL)
    md5_info->accumulator=(unsigned int *)
      RelinquishWizardMemory(md5_info->accumulator);
  if (md5_info->message != (StringInfo *) NULL)
    md5_info->message=DestroyStringInfo(md5_info->message);
  if (md5_info->digest != (StringInfo *) NULL)
    md5_info->digest=DestroyStringInfo(md5_info->digest);
  md5_info->signature=(~WizardSignature);
  md5_info=(MD5Info *) RelinquishWizardMemory(md5_info);
  return(md5_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   F i n a l i z e M D 5                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  FinalizeMD5() finalizes the MD5 message digest computation.
%
%  The format of the FinalizeMD5 method is:
%
%      WizardBooleanType FinalizeMD5(MD5Info *md5_info)
%
%  A description of each parameter follows:
%
%    o md5_info: The address of a structure of type MD5Info.
%
*/
WizardExport WizardBooleanType FinalizeMD5(MD5Info *md5_info)
{
  ssize_t
    number_bytes;

  register ssize_t
    i;

  register unsigned char
    *p;

  StringInfo
    *pad;

  unsigned char
    *datum;

  unsigned int
    message[16];

  /*
    Save number of bits.
  */
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  assert(md5_info != (MD5Info *) NULL);
  assert(md5_info->signature == WizardSignature);
  message[14]=md5_info->low_order;
  message[15]=md5_info->high_order;
  /*
    Final number of bytes mod 64.
  */
  number_bytes=(ssize_t) ((md5_info->low_order >> 3) & 0x3F);
  /*
    Pad message to 56 mod 64.
  */
  pad=AcquireStringInfo((size_t) ((number_bytes < 56) ? (56-number_bytes) :
    (120-number_bytes)));
  datum=GetStringInfoDatum(pad);
  datum[0]=(unsigned char) 0x80;
  for (i=1; i < (ssize_t) GetStringInfoLength(pad); i++)
    datum[i]=(unsigned char) 0x0;
  UpdateMD5(md5_info,pad);
  pad=DestroyStringInfo(pad);
  /*
    Append length in bits and transform.
  */
  p=GetStringInfoDatum(md5_info->message);
  for (i=0; i < 14; i++)
  {
    message[i]=(unsigned int) (*p++);
    message[i]|=((unsigned int) (*p++)) << 8;
    message[i]|=((unsigned int) (*p++)) << 16;
    message[i]|=((unsigned int) (*p++)) << 24;
  }
  TransformMD5(md5_info,message);
  /*
    Store message in digest.
  */
  p=GetStringInfoDatum(md5_info->digest);
  for (i=0; i < (MD5Digestsize/4); i++)
  {
    *p++=(unsigned char) (md5_info->accumulator[i] & 0xff);
    *p++=(unsigned char) ((md5_info->accumulator[i] >> 8) & 0xff);
    *p++=(unsigned char) ((md5_info->accumulator[i] >> 16) & 0xff);
    *p++=(unsigned char) ((md5_info->accumulator[i] >> 24) & 0xff);
  }
  /*
    Reset working registers.
  */
  number_bytes=0;
  (void) ResetWizardMemory(message,0,sizeof(message));
  return(WizardTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t M D 5 B l o c k s i z e                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetMD5Blocksize() returns the MD5 blocksize.
%
%  The format of the GetMD5Blocksize method is:
%
%      unsigned int *GetMD5Blocksize(const MD5Info *md5_info)
%
%  A description of each parameter follows:
%
%    o md5_info: The md5 info.
%
*/
WizardExport unsigned int GetMD5Blocksize(const MD5Info *md5_info)
{
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  WizardAssert(CipherDomain,md5_info != (MD5Info *) NULL);
  WizardAssert(CipherDomain,md5_info->signature == WizardSignature);
  return(md5_info->blocksize);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t M D 5 D i g e s t                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetMD5Digest() returns the MD5 digest.
%
%  The format of the GetMD5Digest method is:
%
%      const StringInfo *GetMD5Digest(const MD5Info *md5_info)
%
%  A description of each parameter follows:
%
%    o md5_info: The md5 info.
%
*/
WizardExport const StringInfo *GetMD5Digest(const MD5Info *md5_info)
{
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  WizardAssert(HashDomain,md5_info != (MD5Info *) NULL);
  WizardAssert(HashDomain,md5_info->signature == WizardSignature);
  return(md5_info->digest);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t M D 5 D i g e s t s i z e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetMD5Digestsize() returns the MD5 digest size.
%
%  The format of the GetMD5Digestsize method is:
%
%      unsigned int *GetMD5Digestsize(const MD5Info *md5_info)
%
%  A description of each parameter follows:
%
%    o md5_info: The md5 info.
%
*/
WizardExport unsigned int GetMD5Digestsize(const MD5Info *md5_info)
{
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  WizardAssert(CipherDomain,md5_info != (MD5Info *) NULL);
  WizardAssert(CipherDomain,md5_info->signature == WizardSignature);
  return(md5_info->digestsize);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I n i t i a l i z e M D 5                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IntializeMD5() intializes the MD5 digest.
%
%  The format of the InitializeMD5 method is:
%
%      WizardBooleanType InitializeMD5(md5_info)
%
%  A description of each parameter follows:
%
%    o md5_info: The address of a structure of type MD5Info.
%
*/
WizardExport WizardBooleanType InitializeMD5(MD5Info *md5_info)
{
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  assert(md5_info != (MD5Info *) NULL);
  assert(md5_info->signature == WizardSignature);
  md5_info->low_order=(unsigned int) 0;
  md5_info->high_order=(unsigned int) 0;
  /*
    Load magic initialization constants.
  */
  md5_info->accumulator[0]=(unsigned int) 0x67452301;
  md5_info->accumulator[1]=(unsigned int) 0xefcdab89;
  md5_info->accumulator[2]=(unsigned int) 0x98badcfe;
  md5_info->accumulator[3]=(unsigned int) 0x10325476;
  return(WizardTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   T r a n s f o r m M D 5                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  TransformMD5() transforms the MD5 message digest.
%
%  The format of the TransformMD5 method is:
%
%      TransformMD5(MD5Info *md5_info)
%
%  A description of each parameter follows:
%
%    o md5_info: The address of a structure of type MD5Info.
%
%
*/

static inline unsigned int F(const unsigned int x,const unsigned int y,
  const unsigned int z)
{
  return((x & y) | (~x & z));
}

static inline unsigned int G(const unsigned int x,const unsigned int y,
  const unsigned int z)
{
  return((x & z) | (y & ~z));
}

static inline unsigned int H(const unsigned int x,const unsigned int y,
  const unsigned int z)
{
  return(x ^ y ^ z);
}

static inline unsigned int I(const unsigned int x,const unsigned int y,
  const unsigned int z)
{
  return(y ^ (x | ~z));
}

static inline unsigned int Trunc32(const unsigned int x)
{
  return((unsigned int) (x & 0xffffffffUL));
}

static inline unsigned int RotateLeft(const unsigned int x,const unsigned int n)
{
  return(Trunc32((x << n) | (x >> (32-n))));
}

static void TransformMD5(MD5Info *md5_info,const unsigned int *message)
{
  register const unsigned int
    *p;

  register ssize_t
    i;

  register unsigned int
    j;

  static const unsigned int
    K[64]=
    {
      0xd76aa478U, 0xe8c7b756U, 0x242070dbU, 0xc1bdceeeU, 0xf57c0fafU,
      0x4787c62aU, 0xa8304613U, 0xfd469501U, 0x698098d8U, 0x8b44f7afU,
      0xffff5bb1U, 0x895cd7beU, 0x6b901122U, 0xfd987193U, 0xa679438eU,
      0x49b40821U, 0xf61e2562U, 0xc040b340U, 0x265e5a51U, 0xe9b6c7aaU,
      0xd62f105dU, 0x02441453U, 0xd8a1e681U, 0xe7d3fbc8U, 0x21e1cde6U,
      0xc33707d6U, 0xf4d50d87U, 0x455a14edU, 0xa9e3e905U, 0xfcefa3f8U,
      0x676f02d9U, 0x8d2a4c8aU, 0xfffa3942U, 0x8771f681U, 0x6d9d6122U,
      0xfde5380cU, 0xa4beea44U, 0x4bdecfa9U, 0xf6bb4b60U, 0xbebfbc70U,
      0x289b7ec6U, 0xeaa127faU, 0xd4ef3085U, 0x04881d05U, 0xd9d4d039U,
      0xe6db99e5U, 0x1fa27cf8U, 0xc4ac5665U, 0xf4292244U, 0x432aff97U,
      0xab9423a7U, 0xfc93a039U, 0x655b59c3U, 0x8f0ccc92U, 0xffeff47dU,
      0x85845dd1U, 0x6fa87e4fU, 0xfe2ce6e0U, 0xa3014314U, 0x4e0811a1U,
      0xf7537e82U, 0xbd3af235U, 0x2ad7d2bbU, 0xeb86d391U,
    };  /* 4294967296*abs(sin(i)), i in radians */

  unsigned int
    A,
    B,
    C,
    D;

  /*
    Copy accumulator to registers
  */
  A=md5_info->accumulator[0];
  B=md5_info->accumulator[1];
  C=md5_info->accumulator[2];
  D=md5_info->accumulator[3];
  /*
    a=b+((a+F(b,c,d)+X[k]+t) <<< s).
  */
  p=K;
  j=0;
  for (i=0; i < 4; i++)
  {
    A+=F(B,C,D)+message[j & 0x0f]+(*p++);
    A=RotateLeft(A,7)+B;
    j++;
    D+=F(A,B,C)+message[j & 0x0f]+(*p++);
    D=RotateLeft(D,12)+A;
    j++;
    C+=F(D,A,B)+message[j & 0x0f]+(*p++);
    C=RotateLeft(C,17)+D;
    j++;
    B+=F(C,D,A)+message[j & 0x0f]+(*p++);
    B=RotateLeft(B,22)+C;
    j++;
  }
  /*
    a=b+((a+G(b,c,d)+X[k]+t) <<< s).
  */
  j=1;
  for (i=0; i < 4; i++)
  {
    A+=G(B,C,D)+message[j & 0x0f]+(*p++);
    A=RotateLeft(A,5)+B;
    j+=5;
    D+=G(A,B,C)+message[j & 0x0f]+(*p++);
    D=RotateLeft(D,9)+A;
    j+=5;
    C+=G(D,A,B)+message[j & 0x0f]+(*p++);
    C=RotateLeft(C,14)+D;
    j+=5;
    B+=G(C,D,A)+message[j & 0x0f]+(*p++);
    B=RotateLeft(B,20)+C;
    j+=5;
  }
  /*
    a=b+((a+H(b,c,d)+X[k]+t) <<< s).
  */
  j=5;
  for (i=0; i < 4; i++)
  {
    A+=H(B,C,D)+message[j & 0x0f]+(*p++);
    A=RotateLeft(A,4)+B;
    j+=3;
    D+=H(A,B,C)+message[j & 0x0f]+(*p++);
    D=RotateLeft(D,11)+A;
    j+=3;
    C+=H(D,A,B)+message[j & 0x0f]+(*p++);
    C=RotateLeft(C,16)+D;
    j+=3;
    B+=H(C,D,A)+message[j & 0x0f]+(*p++);
    B=RotateLeft(B,23)+C;
    j+=3;
  }
  /*
    a=b+((a+I(b,c,d)+X[k]+t) <<< s).
  */
  j=0;
  for (i=0; i < 4; i++)
  {
    A+=I(B,C,D)+message[j & 0x0f]+(*p++);
    A=RotateLeft(A,6)+B;
    j+=7;
    D+=I(A,B,C)+message[j & 0x0f]+(*p++);
    D=RotateLeft(D,10)+A;
    j+=7;
    C+=I(D,A,B)+message[j & 0x0f]+(*p++);
    C=RotateLeft(C,15)+D;
    j+=7;
    B+=I(C,D,A)+message[j & 0x0f]+(*p++);
    B=RotateLeft(B,21)+C;
    j+=7;
  }
  /*
    Add registers back to accumulator.
  */
  md5_info->accumulator[0]=Trunc32(md5_info->accumulator[0]+A);
  md5_info->accumulator[1]=Trunc32(md5_info->accumulator[1]+B);
  md5_info->accumulator[2]=Trunc32(md5_info->accumulator[2]+C);
  md5_info->accumulator[3]=Trunc32(md5_info->accumulator[3]+D);
  /*
    Reset working registers.
  */
  A=0;
  B=0;
  C=0;
  D=0;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U p d a t e M D 5                                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UpdateMD5() updates the MD5 message digest
%
%  The format of the UpdateMD5 method is:
%
%      WizardBooleanType UpdateMD5(MD5Info *md5_info,
%        const unsigned char *message)
%
%  A description of each parameter follows:
%
%    o md5_info: The address of a structure of type MD5Info.
%
*/
WizardExport WizardBooleanType UpdateMD5(MD5Info *md5_info,
  const StringInfo *message)
{
  register unsigned char
    *p;

  register ssize_t
    i,
    j;

  unsigned char
    *datum;

  unsigned int
    buffer[16],
    number_bits,
    number_bytes;

  /*
    Update the MD5 accumulator.
  */
  assert(md5_info != (MD5Info *) NULL);
  assert(md5_info->signature == WizardSignature);
  number_bytes=(unsigned int) ((md5_info->low_order >> 3) & 0x3F);
  number_bits=(unsigned int) (md5_info->low_order+(GetStringInfoLength(message)
    << 3));
  if ((number_bits & 0xffffffff) < md5_info->low_order)
    md5_info->high_order++;
  md5_info->low_order+=(unsigned int) (GetStringInfoLength(message) << 3);
  md5_info->high_order+=(unsigned int) (GetStringInfoLength(message) >> 29);
  datum=GetStringInfoDatum(message);
  for (i=0; i < (ssize_t) GetStringInfoLength(message); i++)
  {
    p=GetStringInfoDatum(md5_info->message);
    p[number_bytes++]=datum[i];
    if (number_bytes != 0x40)
      continue;
    for (j=0; j < 16; j++)
    {
      buffer[j]=(unsigned int) (*p++);
      buffer[j]|=((unsigned int) (*p++)) << 8;
      buffer[j]|=((unsigned int) (*p++)) << 16;
      buffer[j]|=((unsigned int) (*p++)) << 24;
    }
    TransformMD5(md5_info,buffer);
    number_bytes=0;
  }
  /*
    Reset working registers.
  */
  number_bits=0;
  number_bytes=0;
  (void) ResetWizardMemory(buffer,0,sizeof(buffer));
  return(WizardTrue);
}
