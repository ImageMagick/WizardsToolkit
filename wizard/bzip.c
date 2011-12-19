/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                         BBBB   ZZZZZ  IIIII  PPPP                           %
%                         B   B     ZZ    I    P   P                          %
%                         BBBB    ZZZ     I    PPPP                           %
%                         B   B  ZZ       I    P                              %
%                         BBBB   ZZZZZ  IIIII  P                              %
%                                                                             %
%                                                                             %
%                    Wizard's Toolkit BZip Entropy Methods                    %
%                                                                             %
%                             Software Design                                 %
%                               John Cristy                                   %
%                               March 2003                                    %
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
%
%
*/

/*
  Include declarations.
*/
#include "wizard/studio.h"
#include "wizard/bzip.h"
#include "wizard/exception.h"
#include "wizard/exception-private.h"
#include "wizard/memory_.h"
#include "bzlib.h"

/*
  Typedef declaractions;
*/
struct _BZIPInfo
{
  bz_stream
    stream;

  StringInfo
    *chaos;

  size_t
    level;

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
%   A c q u i r e B Z I P I n f o                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquireBZIPInfo() allocates the ZIPInfo structure.
%
%  The format of the AcquireBZIPInfo method is:
%
%      BZIPInfo *AcquireBZIPInfo(const size_t level)
%
%  A description of each parameter follows:
%
%    o level: entropy level: 1 is best speed, 9 is more entropy.
%
*/
WizardExport BZIPInfo *AcquireBZIPInfo(const size_t level)
{
  BZIPInfo
    *bzip_info;

  bzip_info=(BZIPInfo *) AcquireWizardMemory(sizeof(*bzip_info));
  if (bzip_info == (BZIPInfo *) NULL)
    ThrowWizardFatalError(EntropyDomain,MemoryError);
  (void) ResetWizardMemory(bzip_info,0,sizeof(*bzip_info));
  bzip_info->chaos=AcquireStringInfo(1);
  bzip_info->level=level;
  bzip_info->timestamp=time((time_t *) NULL);
  bzip_info->signature=WizardSignature;
  return(bzip_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D e s t r o y B Z I P I n f o                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DestroyBZIPInfo() zeros memory associated with the ZIPInfo structure.
%
%  The format of the DestroyBZIPInfo method is:
%
%      BZIPInfo *DestroyBZIPInfo(BZIPInfo *bzip_info)
%
%  A description of each parameter follows:
%
%    o bzip_info: The bzip info.
%
*/
WizardExport BZIPInfo *DestroyBZIPInfo(BZIPInfo *bzip_info)
{
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  WizardAssert(EntropyDomain,bzip_info != (BZIPInfo *) NULL);
  WizardAssert(EntropyDomain,bzip_info->signature == WizardSignature);
  if (bzip_info->chaos != (StringInfo *) NULL)
    bzip_info->chaos=DestroyStringInfo(bzip_info->chaos);
  bzip_info->signature=(~WizardSignature);
  bzip_info=(BZIPInfo *) RelinquishWizardMemory(bzip_info);
  return(bzip_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t B Z I P C h a o s                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetBZIPChaos() returns BZIP chaos.
%
%  The format of the GetBZIPChaos method is:
%
%      const StringInfo *GetBZIPChaos(const BZIPInfo *bzip_info)
%
%  A description of each parameter follows:
%
%    o bzip_info: The bzip info.
%
*/
WizardExport const StringInfo *GetBZIPChaos(const BZIPInfo *bzip_info)
{
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  WizardAssert(EntropyDomain,bzip_info != (BZIPInfo *) NULL);
  WizardAssert(EntropyDomain,bzip_info->signature == WizardSignature);
  return(bzip_info->chaos);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I n c r e a s e B Z I P                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%

%  IncreaseBZIP() compresses the message to increase its entropy.
%
%  The format of the IncreaseBZIP method is:
%
%      WizardBooleanType IncreaseBZIP(BZIPInfo *bzip_info,
%        const StringInfo *message,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o bzip_info: The address of a structure of type BZIPInfo.
%
%    o message: The message.
%
%    o exception: Return any errors or warnings in this structure.
%
*/

static void *AcquireBZIPMemory(void *context,int items,int size)
{
  return((void *) AcquireQuantumMemory((size_t) items,(size_t) size));
}

WizardExport void bz_internal_error(int error)
{
  ThrowWizardFatalError(EntropyDomain,AssertError);
}

static void RelinquishBZIPMemory(void *context,void *memory)
{
  memory=RelinquishWizardMemory(memory);
}

WizardExport WizardBooleanType IncreaseBZIP(BZIPInfo *bzip_info,
  const StringInfo *message,ExceptionInfo *exception)
{
  int
    status;

  bz_stream
    stream;

  /*
    Increase the message entropy.
  */
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  WizardAssert(EntropyDomain,bzip_info != (BZIPInfo *) NULL);
  WizardAssert(EntropyDomain,bzip_info->signature == WizardSignature);
  WizardAssert(EntropyDomain,message != (const StringInfo *) NULL);
  stream.bzalloc=AcquireBZIPMemory;
  stream.bzfree=RelinquishBZIPMemory;
  stream.opaque=(void *) NULL;
  status=BZ2_bzCompressInit(&stream,(int) bzip_info->level,0,0);
  if (status != BZ_OK)
    {
      (void) ThrowWizardException(exception,GetWizardModule(),EntropyError,
        "unable to increase entropy `%s'",strerror(errno));
      return(WizardFalse);
    }
  stream.next_in=(char *) GetStringInfoDatum(message);
  stream.avail_in=(unsigned int) GetStringInfoLength(message);
  SetStringInfoLength(bzip_info->chaos,(size_t)
    (GetStringInfoLength(message)+GetStringInfoLength(message)/100+600));
  stream.next_out=(char *) GetStringInfoDatum(bzip_info->chaos);
  stream.avail_out=(unsigned int) GetStringInfoLength(bzip_info->chaos);
  status=BZ2_bzCompress(&stream,BZ_FINISH);
  if (status != BZ_STREAM_END)
    {
      (void) ThrowWizardException(exception,GetWizardModule(),EntropyError,
        "unable to increase entropy `%s'",strerror(errno));
      return(WizardFalse);
    }
  SetStringInfoLength(bzip_info->chaos,(size_t) stream.total_out_lo32);
  status=BZ2_bzCompressEnd(&stream);
  if (status != BZ_OK)
    {
      (void) ThrowWizardException(exception,GetWizardModule(),EntropyError,
        "unable to increase entropy `%s'",strerror(errno));
      return(WizardFalse);
    }
  return(WizardTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e s t o r e B Z I P                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RestoreBZIP() uncompresses the message to restore its original entropy.
%
%  The format of the RestoreBZIP method is:
%
%      WizardBooleanType RestoreBZIP(BZIPInfo *bzip_info,const size_t length,
%        const StringInfo *message,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o bzip_info: The address of a structure of type BZIPInfo.
%
%    o length: The total size of the destination buffer, which must be large
%      enough to hold the entire uncompressed data.
%
%    o message: The message.
%
%    o exception: Return any errors or warnings in this structure.
%
*/
WizardExport WizardBooleanType RestoreBZIP(BZIPInfo *bzip_info,
  const size_t length,const StringInfo *message,ExceptionInfo *exception)
{
  int
    status;

  bz_stream
    stream;

  /*
    Restore the message entropy.
  */
  WizardAssert(EntropyDomain,bzip_info != (BZIPInfo *) NULL);
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  WizardAssert(EntropyDomain,bzip_info->signature == WizardSignature);
  WizardAssert(EntropyDomain,message != (const StringInfo *) NULL);
  stream.bzalloc=AcquireBZIPMemory;
  stream.bzfree=RelinquishBZIPMemory;
  stream.opaque=(void *) NULL;
  status=BZ2_bzDecompressInit(&stream,0,0);
  if (status != BZ_OK)
    {
      (void) ThrowWizardException(exception,GetWizardModule(),EntropyError,
        "unable to restore entropy `%s'",strerror(errno));
      return(WizardFalse);
    }
  stream.next_in=(char *) GetStringInfoDatum(message);
  stream.avail_in=(unsigned int) GetStringInfoLength(message);
  SetStringInfoLength(bzip_info->chaos,length);
  stream.next_out=(char *) GetStringInfoDatum(bzip_info->chaos);
  stream.avail_out=(unsigned int) GetStringInfoLength(bzip_info->chaos);
  status=BZ2_bzDecompress(&stream);
  if (status != BZ_STREAM_END)
    {
      (void) ThrowWizardException(exception,GetWizardModule(),EntropyError,
        "unable to restore entropy `%s'",strerror(errno));
      return(WizardFalse);
    }
  SetStringInfoLength(bzip_info->chaos,(size_t) stream.total_out_lo32);
  status=BZ2_bzDecompressEnd(&stream);
  if (status != BZ_OK)
    {
      (void) ThrowWizardException(exception,GetWizardModule(),EntropyError,
        "unable to restore entropy `%s'",strerror(errno));
      return(WizardFalse);
    }
  return(WizardTrue);
}
