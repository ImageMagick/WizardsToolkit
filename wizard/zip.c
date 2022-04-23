/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                             ZZZZZ  IIIII  PPPP                              %
%                                ZZ    I    P   P                             %
%                              ZZZ     I    PPPP                              %
%                             ZZ       I    P                                 %
%                             ZZZZZ  IIIII  P                                 %
%                                                                             %
%                                                                             %
%                     Wizard's Toolkit Zip Entropy Methods                    %
%                                                                             %
%                             Software Design                                 %
%                                 Cristy                                      %
%                               March 2003                                    %
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
#include "wizard/zip.h"
#include "zlib.h"

/*
  Typedef declarations.
*/
struct _ZIPInfo
{
  z_stream
    stream;

  StringInfo
    *chaos;

  size_t
    level;

  ssize_t
    timestamp;

  size_t
    signature;
};

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   A c q u i r e Z I P I n f o                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquireZIPInfo() allocates the ZIPInfo structure.
%
%  The format of the AcquireZIPInfo method is:
%
%      ZIPInfo *AcquireZIPInfo(const size_t level)
%
%  A description of each parameter follows:
%
%    o level: entropy level: 1 is best speed, 9 is more entropy.
%
*/
WizardExport ZIPInfo *AcquireZIPInfo(const size_t level)
{
  ZIPInfo
    *zip_info;

  zip_info=(ZIPInfo *) AcquireWizardMemory(sizeof(*zip_info));
  if (zip_info == (ZIPInfo *) NULL)
    ThrowWizardFatalError(EntropyError,MemoryError);
  (void) memset(zip_info,0,sizeof(*zip_info));
  zip_info->chaos=AcquireStringInfo(1);
  zip_info->level=level;
  zip_info->timestamp=(ssize_t) (time((time_t *) NULL)-WizardEpoch);
  zip_info->signature=WizardSignature;
  return(zip_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t Z I P C h a o s                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetZIPChaos() returns ZIP chaos.
%
%  The format of the GetZIPChaos method is:
%
%      const StringInfo *GetZIPChaos(const ZIPInfo *zip_info)
%
%  A description of each parameter follows:
%
%    o zip_info: The zip info.
%
*/
WizardExport const StringInfo *GetZIPChaos(const ZIPInfo *zip_info)
{
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  WizardAssert(EntropyDomain,zip_info != (ZIPInfo *) NULL);
  WizardAssert(EntropyDomain,zip_info->signature == WizardSignature);
  return(zip_info->chaos);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D e s t r o y Z I P I n f o                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DestroyZIPInfo() zeros memory associated with the ZIPInfo structure.
%
%  The format of the DestroyZIPInfo method is:
%
%      ZIPInfo *DestroyZIPInfo(ZIPInfo *zip_info)
%
%  A description of each parameter follows:
%
%    o zip_info: The zip info.
%
*/
WizardExport ZIPInfo *DestroyZIPInfo(ZIPInfo *zip_info)
{
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  WizardAssert(EntropyDomain,zip_info != (ZIPInfo *) NULL);
  WizardAssert(EntropyDomain,zip_info->signature == WizardSignature);
  if (zip_info->chaos != (StringInfo *) NULL)
    zip_info->chaos=DestroyStringInfo(zip_info->chaos);
  zip_info=(ZIPInfo *) RelinquishWizardMemory(zip_info);
  return(zip_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I n c r e a s e Z i p                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IncreaseZIP() compresses the message to increase its entropy.
%
%  The format of the IncreaseZIP method is:
%
%      WizardBooleanType IncreaseZIP(ZIPInfo *zip_info,
%        const StringInfo *message,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o zip_info: The address of a structure of type ZIPInfo.
%
%    o message: The message.
%
%    o exception: Return any errors or warnings in this structure.
%
*/

static voidpf AcquireZIPMemory(voidpf context,uInt items,uInt size)
{
  return((voidpf) AcquireQuantumMemory(items,size));
}

static void RelinquishZIPMemory(voidpf context,voidpf memory)
{
  memory=RelinquishWizardMemory(memory);
}

WizardExport WizardBooleanType IncreaseZIP(ZIPInfo *zip_info,
  const StringInfo *message,ExceptionInfo *exception)
{
  int
    status;

  z_stream
    stream;

  /*
    Increase the message entropy.
  */
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  WizardAssert(EntropyDomain,zip_info != (ZIPInfo *) NULL);
  WizardAssert(EntropyDomain,zip_info->signature == WizardSignature);
  WizardAssert(EntropyDomain,message != (const StringInfo *) NULL);
  (void) memset(&stream,0,sizeof(stream));
  stream.zalloc=AcquireZIPMemory;
  stream.zfree=RelinquishZIPMemory;
  stream.opaque=(voidpf) NULL;
  status=deflateInit(&stream,(int) zip_info->level);
  if (status != Z_OK)
    {
      (void) ThrowWizardException(exception,GetWizardModule(),EntropyError,
        "unable to increase entropy `%s'",strerror(errno));
      return(WizardFalse);
    }
  stream.next_in=(Bytef *) GetStringInfoDatum(message);
  stream.avail_in=(uInt) GetStringInfoLength(message);
  SetStringInfoLength(zip_info->chaos,(size_t) deflateBound(&stream,
    (unsigned long) GetStringInfoLength(message)));
  stream.next_out=(Bytef *) GetStringInfoDatum(zip_info->chaos);
  stream.avail_out=(uInt) GetStringInfoLength(zip_info->chaos);
  status=deflate(&stream,Z_FINISH);
  if (status != Z_STREAM_END)
    {
      (void) ThrowWizardException(exception,GetWizardModule(),EntropyError,
        "unable to increase entropy `%s'",strerror(errno));
      return(WizardFalse);
    }
  SetStringInfoLength(zip_info->chaos,(size_t) stream.total_out);
  status=deflateEnd(&stream);
  if (status != Z_OK)
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
%   R e s t o r e Z i p                                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RestoreZIP() uncompresses the message to restore its original entropy.
%
%  The format of the RestoreZIP method is:
%
%      WizardBooleanType RestoreZIP(ZIPInfo *zip_info,const size_t length,
%        const StringInfo *message,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o zip_info: The address of a structure of type ZIPInfo.
%
%    o length: The total size of the destination buffer, which must be large
%      enough to hold the entire uncompressed data.
%
%    o message: The message.
%
%    o exception: Return any errors or warnings in this structure.
%
*/
WizardExport WizardBooleanType RestoreZIP(ZIPInfo *zip_info,const size_t length,
  const StringInfo *message,ExceptionInfo *exception)
{
  int
    status;

  z_stream
    stream;

  /*
    Restore the message entropy.
  */
  WizardAssert(EntropyDomain,zip_info != (ZIPInfo *) NULL);
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  WizardAssert(EntropyDomain,zip_info->signature == WizardSignature);
  WizardAssert(EntropyDomain,message != (const StringInfo *) NULL);
  (void) memset(&stream,0,sizeof(stream));
  stream.zalloc=AcquireZIPMemory;
  stream.zfree=RelinquishZIPMemory;
  stream.opaque=(voidpf) NULL;
  status=inflateInit(&stream);
  if (status != Z_OK)
    {
      (void) ThrowWizardException(exception,GetWizardModule(),EntropyError,
        "unable to restore entropy `%s'",strerror(errno));
      return(WizardFalse);
    }
  stream.next_in=(Bytef *) GetStringInfoDatum(message);
  stream.avail_in=(uInt) GetStringInfoLength(message);
  SetStringInfoLength(zip_info->chaos,length);
  stream.next_out=(Bytef *) GetStringInfoDatum(zip_info->chaos);
  stream.avail_out=(uInt) GetStringInfoLength(zip_info->chaos);
  status=inflate(&stream,Z_FINISH);
  if (status != Z_STREAM_END)
    {
      (void) ThrowWizardException(exception,GetWizardModule(),EntropyError,
        "unable to restore entropy `%s'",strerror(errno));
      return(WizardFalse);
    }
  SetStringInfoLength(zip_info->chaos,(size_t) stream.total_out);
  status=inflateEnd(&stream);
  if (status != Z_OK)
    {
      (void) ThrowWizardException(exception,GetWizardModule(),EntropyError,
        "unable to restore entropy `%s'",strerror(errno));
      return(WizardFalse);
    }
  return(WizardTrue);
}
