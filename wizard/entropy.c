/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%              EEEEE  N   N  TTTTT  RRRR    OOO   PPPP    Y   Y               %
%              E      NN  N    T    R   R  O   O  P   P    Y Y                %
%              EEE    N N N    T    RRRR   O   O  PPPP      Y                 %
%              E      N  NN    T    R R    O   O  P         Y                 %
%              EEEEE  N   N    T    R  R    OOO   P         Y                 %
%                                                                             %
%                                                                             %
%                       Wizard's Toolkit Entropy Methods                      %
%                                                                             %
%                             Software Design                                 %
%                                 Cristy                                      %
%                               March 2003                                    %
%                                                                             %
%                                                                             %
%  Copyright 1999-2020 ImageMagick Studio LLC, a non-profit organization      %
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
#include "wizard/bzip.h"
#include "wizard/entropy.h"
#include "wizard/exception.h"
#include "wizard/exception-private.h"
#include "wizard/lzma.h"
#include "wizard/memory_.h"
#include "wizard/zip.h"

/*
  Typedef declarations.
*/
struct _EntropyInfo
{
  EntropyType
    entropy;

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
%   A c q u i r e E n t r o p y I n f o                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquireEntropyInfo() allocates the EntropyInfo structure.
%
%  The format of the AcquireEntropyInfo method is:
%
%      EntropyInfo *AcquireEntropyInfo(const EntropyType entropy,
%        const size_t level)
%
%  A description of each parameter follows:
%
%    o entropy: The entropy type.
%
%    o level: entropy level: 1 is best speed, 9 is more entropy.
%
*/
WizardExport EntropyInfo *AcquireEntropyInfo(const EntropyType entropy,
  const size_t level)
{
  EntropyInfo
    *entropy_info;

  entropy_info=(EntropyInfo *) AcquireWizardMemory(sizeof(*entropy_info));
  if (entropy_info == (EntropyInfo *) NULL)
    ThrowWizardFatalError(EntropyDomain,MemoryError);
  (void) ResetWizardMemory(entropy_info,0,sizeof(*entropy_info));
  entropy_info->entropy=entropy;
  switch (entropy_info->entropy)
  {
    case BZIPEntropy:
    {
      entropy_info->handle=(EntropyInfo *) AcquireBZIPInfo(level);
      break;
    }
    case LZMAEntropy:
    {
      entropy_info->handle=(EntropyInfo *) AcquireLZMAInfo(level);
      break;
    }
    case ZIPEntropy:
    {
      entropy_info->handle=(EntropyInfo *) AcquireZIPInfo(level);
      break;
    }
    default:
      ThrowWizardFatalError(EntropyDomain,EnumerateError);
  }
  entropy_info->timestamp=time((time_t *) NULL);
  entropy_info->signature=WizardSignature;
  return(entropy_info);
}

/*
e%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D e s t r o y E n t r o p y I n f o                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DestroyEntropyInfo() zeros memory associated with the EntropyInfo
%  structure.
%
%  The format of the DestroyEntropyInfo method is:
%
%      EntropyInfo *DestroyEntropyInfo(EntropyInfo *entropy_info)
%
%  A description of each parameter follows:
%
%    o entropy_info: The entropy info.
%
*/
WizardExport EntropyInfo *DestroyEntropyInfo(EntropyInfo *entropy_info)
{
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  WizardAssert(EntropyDomain,entropy_info != (EntropyInfo *) NULL);
  WizardAssert(EntropyDomain,entropy_info->signature == WizardSignature);
  if (entropy_info->handle != (EntropyInfo *) NULL)
    switch (entropy_info->entropy)
    {
      case BZIPEntropy:
      {
        entropy_info->handle=(void *) DestroyBZIPInfo((BZIPInfo *)
          entropy_info->handle);
        break;
      }
      case LZMAEntropy:
      {
        entropy_info->handle=(void *) DestroyLZMAInfo((LZMAInfo *)
          entropy_info->handle);
        break;
      }
      case ZIPEntropy:
      {
        entropy_info->handle=(void *) DestroyZIPInfo((ZIPInfo *)
          entropy_info->handle);
        break;
      }
      default:
        break;
    }
  entropy_info->signature=(~WizardSignature);
  entropy_info=(EntropyInfo *) RelinquishWizardMemory(entropy_info);
  return(entropy_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t E n t r o p y C h a o s                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetEntropyChaos() returns Entropy chaos.
%
%  The format of the GetEntropyChaos method is:
%
%      const StringInfo *GetEntropyChaos(const EntropyInfo *entropy_info)
%
%  A description of each parameter follows:
%
%    o entropy_info: The entropy info.
%
*/
WizardExport const StringInfo *GetEntropyChaos(const EntropyInfo *entropy_info)
{
  const StringInfo
    *chaos;

  /*
    Increase the message entropy.
  */
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  WizardAssert(EntropyDomain,entropy_info != (EntropyInfo *) NULL);
  WizardAssert(EntropyDomain,entropy_info->signature == WizardSignature);
  switch (entropy_info->entropy)
  {
    case BZIPEntropy:
    {
      BZIPInfo
        *bzip_info;

      bzip_info=(BZIPInfo *) entropy_info->handle;
      chaos=GetBZIPChaos(bzip_info);
      break;
    }
    case LZMAEntropy:
    {
      LZMAInfo
        *lzma_info;

      lzma_info=(LZMAInfo *) entropy_info->handle;
      chaos=GetLZMAChaos(lzma_info);
      break;
    }
    case ZIPEntropy:
    {
      ZIPInfo
        *zip_info;

      zip_info=(ZIPInfo *) entropy_info->handle;
      chaos=GetZIPChaos(zip_info);
      break;
    }
    default:
      ThrowWizardFatalError(EntropyDomain,EnumerateError);
  }
  return(chaos);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I n c r e a s e E n t r o p y                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IncreaseEntropy() increases the entropy of a message.
%
%  The format of the IncreaseEntropy method is:
%
%      WizardBooleanType IncreaseEntropy(EntropyInfo *entropy_info,
%        const StringInfo *message,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o entropy_info: The address of a structure of type EntropyInfo.
%
%    o message: The message.
%
%    o exception: Return any errors or warnings in this structure.
%
*/
WizardExport WizardBooleanType IncreaseEntropy(EntropyInfo *entropy_info,
  const StringInfo *message,ExceptionInfo *exception)
{
  WizardBooleanType
    status;

  /*
    Increase the message entropy.
  */
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  WizardAssert(EntropyDomain,entropy_info != (EntropyInfo *) NULL);
  WizardAssert(EntropyDomain,entropy_info->signature == WizardSignature);
  WizardAssert(EntropyDomain,message != (const StringInfo *) NULL);
  status=WizardFalse;
  switch (entropy_info->entropy)
  {
    case BZIPEntropy:
    {
      BZIPInfo
        *bzip_info;

      bzip_info=(BZIPInfo *) entropy_info->handle;
      status=IncreaseBZIP(bzip_info,message,exception);
      break;
    }
    case LZMAEntropy:
    {
      LZMAInfo
        *lzma_info;

      lzma_info=(LZMAInfo *) entropy_info->handle;
      status=IncreaseLZMA(lzma_info,message,exception);
      break;
    }
    case ZIPEntropy:
    {
      ZIPInfo
        *zip_info;

      zip_info=(ZIPInfo *) entropy_info->handle;
      status=IncreaseZIP(zip_info,message,exception);
      break;
    }
    default:
      ThrowWizardFatalError(EntropyDomain,EnumerateError);
  }
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e s t o r e E n t r o p y                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RestoreEntropy() restores the messages to its original entropy.
%
%  The format of the RestoreEntropy method is:
%
%      unsigned ing RestoreEntropy(EntropyInfo *entropy_info,
%        const size_t length,const StringInfo *message,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o entropy_info: The address of a structure of type EntropyInfo.
%
%    o length: The  the total size of the destination buffer, which must be
%      large enough to hold the entire uncompressed data.
%
%    o message: The message.
%
%    o exception: Return any errors or warnings in this structure.
%
*/
WizardExport WizardBooleanType RestoreEntropy(EntropyInfo *entropy_info,
  const size_t length,const StringInfo *message,ExceptionInfo *exception)
{
  WizardBooleanType
    status;

  /*
    Restore the message entropy.
  */
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  WizardAssert(EntropyDomain,entropy_info != (EntropyInfo *) NULL);
  WizardAssert(EntropyDomain,entropy_info->signature == WizardSignature);
  WizardAssert(EntropyDomain,message != (const StringInfo *) NULL);
  status=WizardFalse;
  switch (entropy_info->entropy)
  {
    case BZIPEntropy:
    {
      BZIPInfo
        *bzip_info;

      bzip_info=(BZIPInfo *) entropy_info->handle;
      status=RestoreBZIP(bzip_info,length,message,exception);
      break;
    }
    case LZMAEntropy:
    {
      LZMAInfo
        *lzma_info;

      lzma_info=(LZMAInfo *) entropy_info->handle;
      status=RestoreLZMA(lzma_info,length,message,exception);
      break;
    }
    case ZIPEntropy:
    {
      ZIPInfo
        *zip_info;

      zip_info=(ZIPInfo *) entropy_info->handle;
      status=RestoreZIP(zip_info,length,message,exception);
      break;
    }
    default:
      ThrowWizardFatalError(EntropyDomain,EnumerateError);
  }
  return(status);
}
