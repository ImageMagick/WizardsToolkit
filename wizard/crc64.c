/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                            CCCC  RRRR    CCCC                               %
%                           C      R   R  C                                   %
%                           C      RRRR   C                                   %
%                           C      R R    C                                   %
%                            CCCC  R  R    CCCC                               %
%                                                                             %
%                                                                             %
%               Wizard's Toolkit Cyclic Redunancy Checksum Methods            %
%                                                                             %
%                             Software Design                                 %
%                               John Cristy                                   %
%                               March  2003                                   %
%                                                                             %
%                                                                             %
%  Copyright 1999-2010 ImageMagick Studio LLC, a non-profit organization      %
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
#include "wizard/crc64.h"
#include "wizard/exception.h"
#include "wizard/exception-private.h"
#include "wizard/memory_.h"

/*
  Define declarations.
*/
#define CRC64Blocksize  32
#define CRC64Digestsize  8

/*
  Typedef declarations.
*/
struct _CRC64Info
{   
  unsigned int
    digestsize,
    blocksize;

  StringInfo
    *digest;

  WizardSizeType
    *crc_xor,
    crc;

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
%   A c q u i r e C R C 6 4 I n f o                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquireCRC64Info() allocate the CRC64Info structure.
%
%  The format of the AcquireCRC64Info method is:
%
%      CRC64Info *AcquireCRC64Info(void)
%
*/
WizardExport CRC64Info *AcquireCRC64Info(void)
{
  CRC64Info
    *crc_info;

  crc_info=(CRC64Info *) AcquireWizardMemory(sizeof(*crc_info));
  if (crc_info == (CRC64Info *) NULL)
    ThrowWizardFatalError(HashDomain,MemoryError);
  (void) ResetWizardMemory(crc_info,0,sizeof(*crc_info));
  crc_info->digestsize=CRC64Digestsize;
  crc_info->blocksize=CRC64Blocksize;
  crc_info->digest=AcquireStringInfo(CRC64Digestsize);
  crc_info->crc_xor=(WizardSizeType *) AcquireQuantumMemory(256UL,
    sizeof(*crc_info->crc_xor));
  if (crc_info->crc_xor == (WizardSizeType *) NULL)
    ThrowWizardFatalError(HashDomain,MemoryError);
  crc_info->timestamp=time((time_t *) NULL);
  crc_info->signature=WizardSignature;
  return(crc_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D e s t r o y C R C 6 4 I n f o                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DestroyCRC64Info() zeros memory associated with the CRC64Info structure.
%
%  The format of the DestroyCRC64Info method is:
%
%      CRC64Info *DestroyCRC64Info(CRC64Info *crc_info)
%
%  A description of each parameter follows:
%
%    o crc_info: The cipher crc_info.
%
*/
WizardExport CRC64Info *DestroyCRC64Info(CRC64Info *crc_info)
{
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  assert(crc_info != (CRC64Info *) NULL);
  assert(crc_info->signature == WizardSignature);
  if (crc_info->digest != (StringInfo *) NULL)
    crc_info->digest=DestroyStringInfo(crc_info->digest);
  if (crc_info->crc_xor != (WizardSizeType *) NULL)
    crc_info->crc_xor=(WizardSizeType *)
      RelinquishWizardMemory(crc_info->crc_xor);
  crc_info->signature=(~WizardSignature);
  crc_info=(CRC64Info *) RelinquishWizardMemory(crc_info);
  return(crc_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   F i n a l i z e C R C 6 4                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  FinalizeCRC64() finalizes the CRC64 message digest computation.
%
%  The format of the FinalizeCRC64 method is:
%
%      void FinalizeCRC64(CRC64Info *crc_info)
%
%  A description of each parameter follows:
%
%    o crc_info: The address of a structure of type CRC64Info.
%
%
*/
WizardExport void FinalizeCRC64(CRC64Info *crc_info)
{
  unsigned char
    *datum;

  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  assert(crc_info != (CRC64Info *) NULL);
  assert(crc_info->signature == WizardSignature);
  datum=GetStringInfoDatum(crc_info->digest);
  datum[0]=(unsigned char) (crc_info->crc >> 56);
  datum[1]=(unsigned char) (crc_info->crc >> 48);
  datum[2]=(unsigned char) (crc_info->crc >> 40);
  datum[3]=(unsigned char) (crc_info->crc >> 32);
  datum[4]=(unsigned char) (crc_info->crc >> 24);
  datum[5]=(unsigned char) (crc_info->crc >> 16);
  datum[6]=(unsigned char) (crc_info->crc >> 8);
  datum[7]=(unsigned char) (crc_info->crc >> 0);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t C R C 6 4 B l o c k s i z e                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetCRC64Blocksize() returns the CRC64 blocksize.
%
%  The format of the GetCRC64Blocksize method is:
%
%      unsigned int *GetCRC64Blocksize(const CRC64Info *crc64_info)
%
%  A description of each parameter follows:
%
%    o crc64_info: The crc64 info.
%
*/
WizardExport unsigned int GetCRC64Blocksize(const CRC64Info *crc64_info)
{
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  WizardAssert(CipherDomain,crc64_info != (CRC64Info *) NULL);
  WizardAssert(CipherDomain,crc64_info->signature == WizardSignature);
  return(crc64_info->blocksize);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t C R C 6 4 C y c l i c R e d u n d a n c y C h e c k                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetCRC64CyclicRedundancyCheck() returns the CRC64 cyclic redunancy check.
%
%  The format of the GetCRC64CyclicRedundancyCheck method is:
%
%      WizardSizeType *GetCRC64CyclicRedundancyCheck(
%        const CRC64Info *crc64_info)
%
%  A description of each parameter follows:
%
%    o crc64_info: The crc64 info.
%
*/
WizardExport WizardSizeType GetCRC64CyclicRedundancyCheck(
  const CRC64Info *crc64_info)
{
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  WizardAssert(CipherDomain,crc64_info != (CRC64Info *) NULL);
  WizardAssert(CipherDomain,crc64_info->signature == WizardSignature);
  return(crc64_info->crc);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t C R C 6 4 D i g e s t                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetCRC64Digest() returns the CRC64 digest.
%
%  The format of the GetCRC64Digest method is:
%
%      const StringInfo *GetCRC64Digest(const CRC64Info *crc64_info)
%
%  A description of each parameter follows:
%
%    o crc64_info: The crc64 info.
%
*/
WizardExport const StringInfo *GetCRC64Digest(const CRC64Info *crc64_info)
{
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  WizardAssert(HashDomain,crc64_info != (CRC64Info *) NULL);
  WizardAssert(HashDomain,crc64_info->signature == WizardSignature);
  return(crc64_info->digest);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t C R C 6 4 D i g e s t s i z e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetCRC64Digestsize() returns the CRC64 digest size.
%
%  The format of the GetCRC64Digestsize method is:
%
%      unsigned int *GetCRC64Digestsize(const CRC64Info *crc64_info)
%
%  A description of each parameter follows:
%
%    o crc64_info: The crc64 info.
%
*/
WizardExport unsigned int GetCRC64Digestsize(const CRC64Info *crc64_info)
{
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  WizardAssert(CipherDomain,crc64_info != (CRC64Info *) NULL);
  WizardAssert(CipherDomain,crc64_info->signature == WizardSignature);
  return(crc64_info->digestsize);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I n i t i a l i z e C R C 6 4                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IntializeCRC64() intializes the CRC64 digest.
%
%  The format of the InitializeCRC64 method is:
%
%      void InitializeCRC64(crc_info)
%
%  A description of each parameter follows:
%
%    o crc_info: The address of a structure of type CRC64Info.
%
%
*/
WizardExport void InitializeCRC64(CRC64Info *crc_info)
{
  register ssize_t
    i,
    j;

  WizardSizeType
    alpha;

  /*
    Load magic initialization constants.
  */
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  assert(crc_info != (CRC64Info *) NULL);
  assert(crc_info->signature == WizardSignature);
  crc_info->crc=0;
  for (i=0; i < 256; i++)
  {
    alpha=(WizardSizeType) i;
    for (j=0; j < 8; j++)
      if ((alpha & 0x01) != 0)
        alpha=(WizardSizeType) ((alpha >> 1) ^
          WizardULLConstant(0xd800000000000000));
      else
        alpha>>=1;
    crc_info->crc_xor[i]=alpha;
  }
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U p d a t e C R C 6 4                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UpdateCRC64() updates the CRC64 message digest
%
%  The format of the UpdateCRC64 method is:
%
%      UpdateCRC64(CRC64Info *crc_info,const StringInfo *message)
%
%  A description of each parameter follows:
%
%    o crc_info: The address of a structure of type CRC64Info.
%
*/
WizardExport void UpdateCRC64(CRC64Info *crc_info,const StringInfo *message)
{
  register const unsigned char
    *p;

  register size_t
    i;

  /*
    Update the CRC64 accumulator.
  */
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  assert(crc_info != (CRC64Info *) NULL);
  assert(crc_info->signature == WizardSignature);
  p=GetStringInfoDatum(message);
  for (i=0; i < GetStringInfoLength(message); i++)
  {
    crc_info->crc=(crc_info->crc >> 8) ^
      crc_info->crc_xor[(crc_info->crc ^ (WizardSizeType) *p) & 0xff];
    p++;
  }
}
