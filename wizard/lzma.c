/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                        L      ZZZZZ  M   M   AAA                            %
%                        L         ZZ  MM MM  A   A                           %
%                        L       ZZZ   M M M  AAAAA                           %
%                        L      ZZ     M   M  A   A                           %
%                        LLLLL  ZZZZZ  M   M  A   A                           %
%                                                                             %
%                                                                             %
%                    Wizard's Toolkit LZMA Entropy Methods                    %
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
#include "wizard/exception.h"
#include "wizard/exception-private.h"
#include "wizard/memory_.h"
#include "wizard/lzma.h"
#if defined(WIZARDSTOOLKIT_LZMA_DELEGATE)
#include <lzma.h>
#endif

/*
  Typedef declarations.
*/
struct _LZMAInfo
{
#if defined(WIZARDSTOOLKIT_LZMA_DELEGATE)
  lzma_stream
    stream;
#endif

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
%   A c q u i r e L Z M A I n f o                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquireLZMAInfo() allocates the LZMAInfo structure.
%
%  The format of the AcquireLZMAInfo method is:
%
%      LZMAInfo *AcquireLZMAInfo(const size_t level)
%
%  A description of each parameter follows:
%
%    o level: entropy level: 1 is best speed, 9 is more entropy.
%
*/
WizardExport LZMAInfo *AcquireLZMAInfo(const size_t level)
{
  LZMAInfo
    *lzma_info;

  lzma_info=(LZMAInfo *) AcquireWizardMemory(sizeof(*lzma_info));
  if (lzma_info == (LZMAInfo *) NULL)
    ThrowWizardFatalError(EntropyError,MemoryError);
  (void) ResetWizardMemory(lzma_info,0,sizeof(*lzma_info));
  lzma_info->chaos=AcquireStringInfo(1);
  lzma_info->level=level;
  lzma_info->timestamp=(ssize_t) (time((time_t *) NULL)-WizardEpoch);
  lzma_info->signature=WizardSignature;
  return(lzma_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t L Z M A C h a o s                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetLZMAChaos() returns LZMA chaos.
%
%  The format of the GetLZMAChaos method is:
%
%      const StringInfo *GetLZMAChaos(const LZMAInfo *lzma_info)
%
%  A description of each parameter follows:
%
%    o lzma_info: The lzma info.
%
*/
WizardExport const StringInfo *GetLZMAChaos(const LZMAInfo *lzma_info)
{
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  WizardAssert(EntropyDomain,lzma_info != (LZMAInfo *) NULL);
  WizardAssert(EntropyDomain,lzma_info->signature == WizardSignature);
  return(lzma_info->chaos);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D e s t r o y L Z M A I n f o                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DestroyLZMAInfo() zeros memory associated with the LZMAInfo structure.
%
%  The format of the DestroyLZMAInfo method is:
%
%      LZMAInfo *DestroyLZMAInfo(LZMAInfo *lzma_info)
%
%  A description of each parameter follows:
%
%    o lzma_info: The lzma info.
%
*/
WizardExport LZMAInfo *DestroyLZMAInfo(LZMAInfo *lzma_info)
{
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  WizardAssert(EntropyDomain,lzma_info != (LZMAInfo *) NULL);
  WizardAssert(EntropyDomain,lzma_info->signature == WizardSignature);
  if (lzma_info->chaos != (StringInfo *) NULL)
    lzma_info->chaos=DestroyStringInfo(lzma_info->chaos);
  lzma_info=(LZMAInfo *) RelinquishWizardMemory(lzma_info);
  return(lzma_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I n c r e a s e L Z M A                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IncreaseLZMA() compresses the message to increase its entropy.
%
%  The format of the IncreaseLZMA method is:
%
%      WizardBooleanType IncreaseLZMA(LZMAInfo *lzma_info,
%        const StringInfo *message,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o lzma_info: The address of a structure of type LZMAInfo.
%
%    o message: The message.
%
%    o exception: Return any errors or warnings in this structure.
%
*/

static void *AcquireLZMAMemory(void *context,size_t items,size_t size)
{
  return((void *) AcquireQuantumMemory(items,size));
}

static void RelinquishLZMAMemory(void *context,void *memory)
{
  memory=RelinquishWizardMemory(memory);
}

WizardExport WizardBooleanType IncreaseLZMA(LZMAInfo *lzma_info,
  const StringInfo *message,ExceptionInfo *exception)
{
#if defined(WIZARDSTOOLKIT_LZMA_DELEGATE)
#define LZMAMaxExtent(x)  ((x)+((x)/3)+128)

  int
    status;

  lzma_allocator
    allocator;

  lzma_stream
    initialize_lzma = LZMA_STREAM_INIT,
    stream;

  /*
    Increase the message entropy.
  */
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  WizardAssert(EntropyDomain,lzma_info != (LZMAInfo *) NULL);
  WizardAssert(EntropyDomain,lzma_info->signature == WizardSignature);
  WizardAssert(EntropyDomain,message != (const StringInfo *) NULL);
  (void) ResetWizardMemory(&allocator,0,sizeof(allocator));
  allocator.alloc=AcquireLZMAMemory;
  allocator.free=RelinquishLZMAMemory;
  stream=initialize_lzma;
  stream.allocator=&allocator;
  stream.next_in=GetStringInfoDatum(message);
  stream.avail_in=GetStringInfoLength(message);
  SetStringInfoLength(lzma_info->chaos,(size_t) LZMAMaxExtent(
    GetStringInfoLength(message)));
  stream.next_out=GetStringInfoDatum(lzma_info->chaos);
  stream.avail_out=GetStringInfoLength(lzma_info->chaos);
  status=lzma_easy_encoder(&stream,lzma_info->level,LZMA_CHECK_SHA256);
  if (status != LZMA_OK)
    {
      (void) ThrowWizardException(exception,GetWizardModule(),EntropyError,
        "unable to increase entropy `%s'",strerror(errno));
      lzma_end(&stream);
      return(WizardFalse);
    }
  status=lzma_code(&stream,LZMA_RUN);
  if (status != LZMA_OK)
    {
      (void) ThrowWizardException(exception,GetWizardModule(),EntropyError,
        "unable to increase entropy `%s'",strerror(errno));
      lzma_end(&stream);
      return(WizardFalse);
    }
  status=lzma_code(&stream,LZMA_FINISH);
  if ((status != LZMA_STREAM_END) && (status != LZMA_OK))
    {
      (void) ThrowWizardException(exception,GetWizardModule(),EntropyError,
        "unable to restore entropy `%s'",strerror(errno));
      lzma_end(&stream);
      return(WizardFalse);
    }
  SetStringInfoLength(lzma_info->chaos,(size_t) stream.total_out);
  lzma_end(&stream);
  return(WizardTrue);
#else
  (void) ThrowWizardException(exception,GetWizardModule(),EntropyError,
    "unable to increase entropy `%s'","LZMA delegate support not builtin");
  return(WizardFalse);
#endif
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e s t o r e L Z M A                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RestoreLZMA() uncompresses the message to restore its original entropy.
%
%  The format of the RestoreLZMA method is:
%
%      WizardBooleanType RestoreLZMA(LZMAInfo *lzma_info,const size_t length,
%        const StringInfo *message,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o lzma_info: The address of a structure of type LZMAInfo.
%
%    o length: The total size of the destination buffer, which must be large
%      enough to hold the entire uncompressed data.
%
%    o message: The message.
%
%    o exception: Return any errors or warnings in this structure.
%
*/
WizardExport WizardBooleanType RestoreLZMA(LZMAInfo *lzma_info,
  const size_t length,const StringInfo *message,ExceptionInfo *exception)
{
#if defined(WIZARDSTOOLKIT_LZMA_DELEGATE)
  int
    status;

  lzma_allocator
    allocator;

  lzma_stream
    initialize_lzma = LZMA_STREAM_INIT,
    stream;

  /*
    Restore the message entropy.
  */
  WizardAssert(EntropyDomain,lzma_info != (LZMAInfo *) NULL);
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  WizardAssert(EntropyDomain,lzma_info->signature == WizardSignature);
  WizardAssert(EntropyDomain,message != (const StringInfo *) NULL);
  (void) ResetWizardMemory(&allocator,0,sizeof(allocator));
  allocator.alloc=AcquireLZMAMemory;
  allocator.free=RelinquishLZMAMemory;
  stream=initialize_lzma;
  stream.allocator=&allocator;
  stream.next_in=GetStringInfoDatum(message);
  stream.avail_in=GetStringInfoLength(message);
  SetStringInfoLength(lzma_info->chaos,length);
  stream.next_out=GetStringInfoDatum(lzma_info->chaos);
  stream.avail_out=GetStringInfoLength(lzma_info->chaos);
  status=lzma_auto_decoder(&stream,-1,0);
  if (status != LZMA_OK)
    {
      (void) ThrowWizardException(exception,GetWizardModule(),EntropyError,
        "unable to restore entropy `%s'",strerror(errno));
      lzma_end(&stream);
      return(WizardFalse);
    }
  status=lzma_code(&stream,LZMA_RUN);
  if (status < 0)
    {
      (void) ThrowWizardException(exception,GetWizardModule(),EntropyError,
        "unable to restore entropy `%s'",strerror(errno));
      lzma_end(&stream);
      return(WizardFalse);
    }
  status=lzma_code(&stream,LZMA_FINISH);
  if ((status != LZMA_STREAM_END) && (status != LZMA_OK))
    {
      (void) ThrowWizardException(exception,GetWizardModule(),EntropyError,
        "unable to restore entropy `%s'",strerror(errno));
      lzma_end(&stream);
      return(WizardFalse);
    }
  SetStringInfoLength(lzma_info->chaos,(size_t) stream.total_out);
  lzma_end(&stream);
  return(WizardTrue);
#else
  (void) ThrowWizardException(exception,GetWizardModule(),EntropyError,
    "unable to restore entropy `%s'","LZMA delegate support not builtin");
  return(WizardFalse);
#endif
}
