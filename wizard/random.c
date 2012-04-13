/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                 RRRR    AAA   N   N  DDDD    OOO   M   M                    %
%                 R   R  A   A  NN  N  D   D  O   O  MM MM                    %
%                 RRRR   AAAAA  N N N  D   D  O   O  M M M                    %
%                 R R    A   A  N  NN  D   D  O   O  M   M                    %
%                 R  R   A   A  N   N  DDDD    OOO   M   M                    %
%                                                                             %
%                                                                             %
%                   Wizard's Toolkit Random Numbers Methods                   %
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
#if defined(__MINGW32__)
#include <sys/time.h>
#endif
#include "wizard/studio.h"
#include "wizard/entropy.h"
#include "wizard/exception.h"
#include "wizard/exception-private.h"
#include "wizard/file.h"
#include "wizard/hash.h"
#include "wizard/magick.h"
#include "wizard/memory_.h"
#include "wizard/random_.h"
#include "wizard/thread_.h"
#include "wizard/thread-private.h"
#include "wizard/semaphore.h"
#include "wizard/utility-private.h"

/*
  Define declarations.
*/
#define PseudoRandomHash  SHA256Hash
#define RandomEntropyLevel  9
#define RandomFilename  "reservoir.xdm"
#define RandomFiletype  "random"
#define RandomProtocolMajorVersion  1
#define RandomProtocolMinorVersion  1

/*
  Typedef declarations.
*/
struct _RandomInfo
{
  HMACInfo
    *hmac_info;

  StringInfo
    *nonce,
    *reservoir;

  size_t
    i;

  unsigned long
    seed[4];

  double
    normalize;

  unsigned short
    protocol_major,
    protocol_minor;

  SemaphoreInfo
    *semaphore;

  time_t
    timestamp;

  size_t
    signature;
};

/*
  External declarations.
*/
#if defined(__APPLE__) && !defined(TARGET_OS_IPHONE)
#include <crt_externs.h>
#define environ (*_NSGetEnviron())
#endif

extern char
  **environ;

/*
  Global declarations.
*/
static SemaphoreInfo
  *random_semaphore = (SemaphoreInfo *) NULL;

static unsigned long
  random_seed = ~0UL;

static WizardBooleanType
  gather_true_random = WizardFalse;

/*
  Forward declarations.
*/
static WizardBooleanType
  SaveEntropyToReservoir(RandomInfo *,ExceptionInfo *);

static StringInfo
  *GenerateEntropicChaos(RandomInfo *,ExceptionInfo *),
  *GetEntropyFromReservoir(RandomInfo *,ExceptionInfo *);

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   A c q u i r e R a n d o m I n f o                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquireRandomInfo() allocates the RandomInfo structure.
%
%  The format of the AcquireRandomInfo method is:
%
%      RandomInfo *AcquireRandomInfo(const RandomType hash)
%
%  A description of each parameter follows:
%
%    o hash: the hash type.
%
*/

static inline size_t WizardMin(const size_t x,const size_t y)
{
  if (x < y)
    return(x);
  return(y);
}

WizardExport RandomInfo *AcquireRandomInfo(const HashType hash)
{
  ExceptionInfo
    *exception;

  RandomInfo
    *random_info;

  WizardBooleanType
    status;

  StringInfo
    *entropy,
    *key,
    *nonce;

  random_info=(RandomInfo *) AcquireWizardMemory(sizeof(*random_info));
  if (random_info == (RandomInfo *) NULL)
    ThrowWizardFatalError(HashDomain,MemoryError);
  (void) ResetWizardMemory(random_info,0,sizeof(*random_info));
  random_info->hmac_info=AcquireHMACInfo(hash);
  random_info->nonce=AcquireStringInfo(2*GetHMACDigestsize(
    random_info->hmac_info));
  ResetStringInfo(random_info->nonce);
  random_info->reservoir=AcquireStringInfo(GetHMACDigestsize(
    random_info->hmac_info));
  ResetStringInfo(random_info->reservoir);
  random_info->normalize=1.0/(~0UL);
  random_info->semaphore=AllocateSemaphoreInfo();
  random_info->protocol_major=RandomProtocolMajorVersion;
  random_info->protocol_minor=RandomProtocolMinorVersion;
  random_info->timestamp=time(0);
  random_info->signature=WizardSignature;
  /*
    Seed random nonce.
  */
  exception=AcquireExceptionInfo();
  nonce=GenerateEntropicChaos(random_info,exception);
  if (nonce == (StringInfo *) NULL)
    ThrowFatalException(RandomFatalError,"Failed to initialize random "
      "reservoir `%s'");
  InitializeHMAC(random_info->hmac_info,nonce);
  SetStringInfoLength(nonce,(GetHMACDigestsize(random_info->hmac_info)+1)/2);
  SetStringInfo(nonce,GetHMACDigest(random_info->hmac_info));
  SetStringInfo(random_info->nonce,nonce);
  nonce=DestroyStringInfo(nonce);
  /*
    Seed random reservoir with entropic data.
  */
  entropy=GenerateEntropicChaos(random_info,exception);
  if (entropy == (StringInfo *) NULL)
    ThrowFatalException(RandomFatalError,"Failed to initialize random "
      "reservoir `%s'");
  InitializeHMAC(random_info->hmac_info,entropy);
  SetStringInfo(random_info->reservoir,GetHMACDigest(random_info->hmac_info));
  status=SaveEntropyToReservoir(random_info,exception);
  entropy=DestroyStringInfo(entropy);
  exception=DestroyExceptionInfo(exception);
  if (status == WizardFalse)
    {
      random_info=DestroyRandomInfo(random_info);
      return((RandomInfo *) NULL);
    }
  /*
    Seed pseudo random number generator.
  */
  if (random_seed == ~0UL)
    {
      key=GetRandomKey(random_info,sizeof(random_seed));
      (void) CopyWizardMemory(random_info->seed,GetStringInfoDatum(key),
        GetStringInfoLength(key));
      key=DestroyStringInfo(key);
    }
  else
    {
      const StringInfo
        *digest;

      HashInfo
        *signature_info;

      signature_info=AcquireHashInfo(PseudoRandomHash);
      key=AcquireStringInfo(sizeof(random_seed));
      SetStringInfoDatum(key,(unsigned char *) &random_seed);
      UpdateHash(signature_info,key);
      key=DestroyStringInfo(key);
      FinalizeHash(signature_info);
      digest=GetHashDigest(signature_info);
      (void) CopyWizardMemory(random_info->seed,GetStringInfoDatum(digest),
        WizardMin(GetHashDigestsize(signature_info),
        sizeof(*random_info->seed)));
      signature_info=DestroyHashInfo(signature_info);
    }
  random_info->seed[1]=0x50a7f451UL;
  random_info->seed[2]=0x5365417eUL;
  random_info->seed[3]=0xc3a4171aUL;
  return(random_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   D e s t r o y R a n d o m I n f o                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DestroyRandomInfo() deallocates memory associated with the random
%  reservoir.
%
%  The format of the DestroyRandomInfo method is:
%
%      RandomInfo *DestroyRandomInfo(RandomInfo *random_info)
%
%  A description of each parameter follows:
%
%    o random_info: the random info.
%
*/
WizardExport RandomInfo *DestroyRandomInfo(RandomInfo *random_info)
{
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  WizardAssert(CipherDomain,random_info != (RandomInfo *) NULL);
  WizardAssert(CipherDomain,random_info->signature == WizardSignature);
  LockSemaphoreInfo(random_info->semaphore);
  if (random_info->reservoir != (StringInfo *) NULL)
    random_info->reservoir=DestroyStringInfo(random_info->reservoir);
  if (random_info->nonce != (StringInfo *) NULL)
    random_info->nonce=DestroyStringInfo(random_info->nonce);
  if (random_info->hmac_info != (HMACInfo *) NULL)
    random_info->hmac_info=DestroyHMACInfo(random_info->hmac_info);
  (void) ResetWizardMemory(random_info->seed,0,sizeof(*random_info->seed));
  random_info->signature=(~WizardSignature);
  UnlockSemaphoreInfo(random_info->semaphore);
  DestroySemaphoreInfo(&random_info->semaphore);
  random_info=(RandomInfo *) RelinquishWizardMemory(random_info);
  return(random_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   G e n e r a t e E n t r o p i c C h a o s                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GenerateEntropicChaos() generate entropic chaos used to initialize the
%  random reservoir.
%
%  The format of the GenerateEntropicChaos method is:
%
%      StringInfo *GenerateEntropicChaos(RandomInfo *random_info,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o random_info: the random info.
%
%    o exception: Return any errors or warnings in this structure.
%
*/

static ssize_t ReadRandom(int file,unsigned char *source,size_t length)
{
  register unsigned char
    *q;

  ssize_t
    offset,
    count;

  offset=0;
  for (q=source; length != 0; length-=count)
  {
    count=(ssize_t) read(file,q,length);
    if (count <= 0)
      {
        count=0;
        if (errno == EINTR)
          continue;
        return(-1);
      }
    q+=count;
    offset+=count;
  }
  return(offset);
}

static StringInfo *GenerateEntropicChaos(RandomInfo *random_info,
  ExceptionInfo *exception)
{
#define MaxEntropyExtent  64

  ssize_t
    pid;

  StringInfo
    *chaos,
    *entropy;

  size_t
    nanoseconds,
    seconds;

  WizardThreadType
    tid;

  /*
    Initialize random reservoir.
  */
  entropy=GetEntropyFromReservoir(random_info,exception);
  if (entropy == (StringInfo *) NULL)
    entropy=AcquireStringInfo(0);
  LockSemaphoreInfo(random_info->semaphore);
  chaos=AcquireStringInfo(sizeof(unsigned char *));
  SetStringInfoDatum(chaos,(unsigned char *) &entropy);
  ConcatenateStringInfo(entropy,chaos);
  SetStringInfoDatum(chaos,(unsigned char *) entropy);
  ConcatenateStringInfo(entropy,chaos);
  pid=(ssize_t) getpid();
  SetStringInfoLength(chaos,sizeof(pid));
  SetStringInfoDatum(chaos,(unsigned char *) &pid);
  ConcatenateStringInfo(entropy,chaos);
  tid=GetWizardThreadId();
  SetStringInfoLength(chaos,sizeof(tid));
  SetStringInfoDatum(chaos,(unsigned char *) &tid);
  ConcatenateStringInfo(entropy,chaos);
#if defined(WIZARDSTOOLKIT_HAVE_GETRUSAGE) && defined(RUSAGE_SELF)
  {
    struct rusage
      usage;

    if (getrusage(RUSAGE_SELF,&usage) == 0)
      {
        SetStringInfoLength(chaos,sizeof(usage));
        SetStringInfoDatum(chaos,(unsigned char *) &usage);
      }
  }
#endif
  seconds=time((time_t *) 0);
  nanoseconds=0;
#if defined(WIZARDSTOOLKIT_HAVE_GETTIMEOFDAY)
  {
    struct timeval
      timer;

    if (gettimeofday(&timer,(struct timezone *) NULL) == 0)
      {
        seconds=timer.tv_sec;
        nanoseconds=1000UL*timer.tv_usec;
      }
  }
#endif
#if defined(WIZARDSTOOLKIT_HAVE_CLOCK_GETTIME) && defined(CLOCK_REALTIME_HR)
  {
    struct timespec
      timer;

    if (clock_gettime(CLOCK_REALTIME_HR,&timer) == 0)
      {
        seconds=timer.tv_sec;
        nanoseconds=timer.tv_nsec;
      }
  }
#endif
  SetStringInfoLength(chaos,sizeof(seconds));
  SetStringInfoDatum(chaos,(unsigned char *) &seconds);
  ConcatenateStringInfo(entropy,chaos);
  SetStringInfoLength(chaos,sizeof(nanoseconds));
  SetStringInfoDatum(chaos,(unsigned char *) &nanoseconds);
  ConcatenateStringInfo(entropy,chaos);
  nanoseconds=0;
#if defined(WIZARDSTOOLKIT_HAVE_CLOCK)
  nanoseconds=clock();
#endif
#if defined(WIZARDSTOOLKIT_HAVE_TIMES)
  {
    struct tms
      timer;

    (void) times(&timer);
    nanoseconds=timer.tms_utime+timer.tms_stime;
  }
#endif
  SetStringInfoLength(chaos,sizeof(nanoseconds));
  SetStringInfoDatum(chaos,(unsigned char *) &nanoseconds);
  ConcatenateStringInfo(entropy,chaos);
#if defined(WIZARDSTOOLKIT_HAVE_MKSTEMP)
  {
    char
      *filename;

    int
      file;

    filename=AcquireString("wizardXXXXXX");
    file=mkstemp(filename);
    if (file != -1)
      if (close(file) == -1)
        (void) ThrowWizardException(exception,GetWizardModule(),RandomError,
          "unable to close file `%s': %s",filename,strerror(errno));
    if (remove_utf8(filename) == -1)
      (void) ThrowWizardException(exception,GetWizardModule(),RandomError,
        "unable to remove file `%s': %s",filename,strerror(errno));
    SetStringInfoLength(chaos,strlen(filename));
    SetStringInfoDatum(chaos,(unsigned char *) filename);
    ConcatenateStringInfo(entropy,chaos);
    filename=DestroyString(filename);
  }
#endif
#if defined(WIZARDSTOOLKIT_WINDOWS_SUPPORT)
  {
    double
      seconds;

    LARGE_INTEGER
      nanoseconds;

    WizardBooleanType
      status;

    /*
      Not crytographically strong but better than nothing.
    */
    seconds=NTElapsedTime()+NTUserTime();
    SetStringInfoLength(chaos,sizeof(seconds));
    SetStringInfoDatum(chaos,(unsigned char *) &seconds);
    ConcatenateStringInfo(entropy,chaos);
    if (QueryPerformanceCounter(&nanoseconds) != 0)
      {
        SetStringInfoLength(chaos,sizeof(nanoseconds));
        SetStringInfoDatum(chaos,(unsigned char *) &nanoseconds);
        ConcatenateStringInfo(entropy,chaos);
      }
    /*
      Our best hope for true entropy.
    */
    SetStringInfoLength(chaos,MaxEntropyExtent);
    status=NTGatherRandomData(MaxEntropyExtent,GetStringInfoDatum(chaos));
    if (status == WizardFalse)
      (void) ThrowWizardException(exception,GetWizardModule(),RandomError,
        "unable to gather random data: %s",strerror(errno));
    ConcatenateStringInfo(entropy,chaos);
  }
#else
  {
    char
      *filename;

    int
      file;

    ssize_t
      count;

    StringInfo
      *device;

    /*
      Not crytographically strong but better than nothing.
    */
    if (environ != (char **) NULL)
      {
        register ssize_t
          i;

        /*
          Squeeze some entropy from the sometimes unpredicatble environment.
        */
        for (i=0; environ[i] != (char *) NULL; i++)
        {
          SetStringInfoLength(chaos,strlen(environ[i]));
          SetStringInfoDatum(chaos,(unsigned char *) environ[i]);
          ConcatenateStringInfo(entropy,chaos);
        }
      }
    filename=AcquireString("/dev/urandom");
    device=StringToStringInfo(filename);
    if (GetStringInfoCRC(device) != (WizardSizeType) 0xf9f0014228f2264fll)
      ThrowFatalException(RandomFatalError,"random device error `%s'");
    device=DestroyStringInfo(device);
    file=open_utf8(filename,O_RDONLY | O_BINARY,0);
    filename=DestroyString(filename);
    if (file != -1)
      {
        SetStringInfoLength(chaos,MaxEntropyExtent);
        count=ReadRandom(file,GetStringInfoDatum(chaos),MaxEntropyExtent);
        if (close(file) == -1)
          (void) ThrowWizardException(exception,GetWizardModule(),RandomError,
            "unable to close file `%s': %s",filename,strerror(errno));
        SetStringInfoLength(chaos,(size_t) count);
        ConcatenateStringInfo(entropy,chaos);
      }
    if (gather_true_random != WizardFalse)
      {
        /*
          Our best hope for true entropy.
        */
        filename=AcquireString("/dev/random");
        device=StringToStringInfo(filename);
        if (GetStringInfoCRC(device) != (WizardSizeType) 0xc36e1cc17bf25fd4ll)
          ThrowFatalException(RandomFatalError,"random device error `%s'");
        device=DestroyStringInfo(device);
        file=open_utf8(filename,O_RDONLY | O_BINARY,0);
        filename=DestroyString(filename);
        if (file == -1)
          {
            filename=AcquireString("/dev/srandom");
            device=StringToStringInfo(filename);
            if (GetStringInfoCRC(device) != (WizardSizeType)
                0xf9f0014228f223efll)
              ThrowFatalException(RandomFatalError,"random device error `%s'");
            device=DestroyStringInfo(device);
            file=open_utf8(filename,O_RDONLY | O_BINARY,0);
          }
        if (file != -1)
          {
            SetStringInfoLength(chaos,MaxEntropyExtent);
            count=ReadRandom(file,GetStringInfoDatum(chaos),MaxEntropyExtent);
            if (close(file) == -1)
              (void) ThrowWizardException(exception,GetWizardModule(),
                RandomError,"unable to close file `%s': %s",filename,
                strerror(errno));
            SetStringInfoLength(chaos,(size_t) count);
            ConcatenateStringInfo(entropy,chaos);
          }
      }
  }
#endif
  chaos=DestroyStringInfo(chaos);
  {
    const StringInfo
      *chaos;

    EntropyInfo
      *entropy_info;

    /*
      Estimate entropy.
    */
    entropy_info=AcquireEntropyInfo(ZIPEntropy,RandomEntropyLevel);
    (void) IncreaseEntropy(entropy_info,entropy,exception);
    chaos=GetEntropyChaos(entropy_info);
    if (GetStringInfoLength(entropy) < (GetStringInfoLength(chaos)/2))
      ThrowFatalException(RandomFatalError,"not enough entropy to continue "
        "`%s'");
    entropy_info=DestroyEntropyInfo(entropy_info);
  }
  UnlockSemaphoreInfo(random_info->semaphore);
  return(entropy);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t E n t r o p y F r o m R e s e r v i o r                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetEntropyFromReservoir() gets entropy from the random reservoir on disk.
%
%  The format of the GetEntropyFromReservoir method is:
%
%      StringInfo *GetEntropyFromReservoir(RandomInfo *random_info,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o random_info: the random reservoir info.
%
%    o exception: Return any errors or warnings in this structure.
%
*/
static StringInfo *GetEntropyFromReservoir(RandomInfo *random_info,
  ExceptionInfo *exception)
{
  FileInfo
    *file_info;

  WizardStatusType
    status;

  StringInfo
    *entropy,
    *filetype,
    *magick,
    *target;

  size_t
    length;

  WizardSizeType
    crc;

  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  WizardAssert(CipherDomain,random_info != (RandomInfo *) NULL);
  WizardAssert(CipherDomain,random_info->signature == WizardSignature);
  WizardAssert(CipherDomain,exception != (ExceptionInfo *) NULL);
  LockSemaphoreInfo(random_info->semaphore);
  file_info=AcquireFileInfo((const char *) NULL,RandomFilename,ReadFileMode,
    exception);
  if (file_info == (FileInfo *) NULL)
    {
      UnlockSemaphoreInfo(random_info->semaphore);
      return((StringInfo *) NULL);
    }
  magick=GetWizardMagick(WizardMagick,sizeof(WizardMagick));
  target=CloneStringInfo(magick);
  status=ReadFileChunk(file_info,GetStringInfoDatum(target),
    GetStringInfoLength(target));
  if ((status == WizardFalse) || (CompareStringInfo(target,magick) != 0))
    {
      file_info=DestroyFileInfo(file_info,exception);
      (void) ThrowWizardException(exception,GetWizardModule(),RandomError,
        "corrupt random reservoir file `%s'",GetFilePath(file_info));
      UnlockSemaphoreInfo(random_info->semaphore);
      return((StringInfo *) NULL);
    }
  magick=DestroyStringInfo(magick);
  target=DestroyStringInfo(target);
  filetype=GetWizardMagick((unsigned char *) RandomFiletype,strlen(
    RandomFiletype));
  target=CloneStringInfo(filetype);
  status|=ReadFileChunk(file_info,GetStringInfoDatum(target),
    GetStringInfoLength(target));
  if ((status == WizardFalse) || (CompareStringInfo(target,filetype) != 0))
    {
      filetype=DestroyStringInfo(filetype);
      target=DestroyStringInfo(target);
      file_info=DestroyFileInfo(file_info,exception);
      (void) ThrowWizardException(exception,GetWizardModule(),RandomError,
        "corrupt random reservoir file `%s'",GetFilePath(file_info));
      UnlockSemaphoreInfo(random_info->semaphore);
      return((StringInfo *) NULL);
    }
  filetype=DestroyStringInfo(filetype);
  target=DestroyStringInfo(target);
  status|=ReadFile16Bits(file_info,&random_info->protocol_major);
  status|=ReadFile16Bits(file_info,&random_info->protocol_minor);
  if ((random_info->protocol_major != 1) || (random_info->protocol_minor != 0))
    {
      WizardSizeType
        timestamp;

      status|=ReadFile64Bits(file_info,&timestamp);
      random_info->timestamp=(time_t) timestamp;
    }
  status|=ReadFile32Bits(file_info,&length);
  if (status == WizardFalse)
    {
      file_info=DestroyFileInfo(file_info,exception);
      (void) ThrowWizardException(exception,GetWizardModule(),RandomError,
        "corrupt random reservoir file `%s'",GetFilePath(file_info));
      UnlockSemaphoreInfo(random_info->semaphore);
      return((StringInfo *) NULL);
    }
  entropy=AcquireStringInfo(length);
  status|=ReadFileChunk(file_info,GetStringInfoDatum(entropy),
    GetStringInfoLength(entropy));
  crc=0;
  status|=ReadFile64Bits(file_info,&crc);
  if ((status == WizardFalse) || (crc != GetStringInfoCRC(entropy)))
    {
      entropy=DestroyStringInfo(entropy);
      (void) ThrowWizardException(exception,GetWizardModule(),RandomError,
        "corrupt random reservoir file `%s'",GetFilePath(file_info));
      file_info=DestroyFileInfo(file_info,exception);
      UnlockSemaphoreInfo(random_info->semaphore);
      return((StringInfo *) NULL);
    }
  file_info=DestroyFileInfo(file_info,exception);
  UnlockSemaphoreInfo(random_info->semaphore);
  return(entropy);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t P s e u d o R a n d o m V a l u e                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetPseudoRandomValue() return a non-negative size_t value uniformly
%  distributed over the interval [0.0, 1.0) with a 2 to the 128th-1 period.
%
%  The format of the GetPseudoRandomValue method is:
%
%      double GetPseudoRandomValue(RandomInfo *randon_info)
%
%  A description of each parameter follows:
%
%    o random_info: the random info.
%
*/
WizardExport double GetPseudoRandomValue(RandomInfo *random_info)
{
  register unsigned long
    *seed;

  unsigned long
    alpha;

  seed=random_info->seed;
  do
  {
    alpha=(unsigned long) (seed[1] ^ (seed[1] << 11));
    seed[1]=seed[2];
    seed[2]=seed[3];
    seed[3]=seed[0];
    seed[0]=(seed[0] ^ (seed[0] >> 19)) ^ (alpha ^ (alpha >> 8));
  } while (seed[0] == ~0UL);
  return(random_info->normalize*seed[0]);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t R a n d o m K e y                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetRandomKey() gets a random key from the reservoir.
%
%  The format of the GetRandomKey method is:
%
%      StringInfo *GetRandomKey(RandomInfo *random_info,const size_t length)
%
%  A description of each parameter follows:
%
%    o random_info: the random info.
%
%    o length: the key length.
%
*/
WizardExport StringInfo *GetRandomKey(RandomInfo *random_info,
  const size_t length)
{
  StringInfo
    *key;

  WizardAssert(CipherDomain,random_info != (RandomInfo *) NULL);
  key=AcquireStringInfo(length);
  SetRandomKey(random_info,length,GetStringInfoDatum(key));
  return(key);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t R a n d o m V a l u e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetRandomValue() return a non-negative double-precision floating-point
%  value uniformly distributed over the interval [0.0, 1.0) with a 2 to the
%  128th-1 period (not cryptographically strong).
%
%  The format of the GetRandomValue method is:
%
%      double GetRandomValue(void)
%
*/
WizardExport double GetRandomValue(RandomInfo *random_info)
{
  unsigned long
    key,
    range;

  range=(~0UL);
  do
  {
    SetRandomKey(random_info,sizeof(key),(unsigned char *) &key);
  } while (key == range);
  return((double) key/range);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   R a n d o m C o m p o n e n t G e n e s i s                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RandomComponentGenesis() instantiates the random component.
%
%  The format of the RandomComponentGenesis method is:
%
%      RandomComponentGenesis(void)
%
*/
WizardExport WizardBooleanType RandomComponentGenesis(void)
{
  AcquireSemaphoreInfo(&random_semaphore);
  return(WizardTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   R a n d o m C o m p o n e n t T e r m i n u s                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RandomComponentTerminus() destroys the random component.
%
%  The format of the RandomComponentTerminus method is:
%
%      RandomComponentTerminus(void)
%
*/
WizardExport void RandomComponentTerminus(void)
{
  if (random_semaphore == (SemaphoreInfo *) NULL)
    AcquireSemaphoreInfo(&random_semaphore);
  DestroySemaphoreInfo(&random_semaphore);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S a v e E n t r o p y T o R e s e r v o i r                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SaveEntropyToReservoir() saves entropy to the reservior on disk.
%
%  The format of the SaveEntropyToReservoir method is:
%
%      WizardBooleanType SaveEntropyToReservoir(RandomInfo *random_info,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o random_info: the random info.
%
%    o exception: Return any errors or warnings in this structure.
%
*/
static WizardBooleanType SaveEntropyToReservoir(RandomInfo *random_info,
  ExceptionInfo *exception)
{
  FileInfo
    *file_info;

  WizardStatusType
    status;

  size_t
    length;

  StringInfo
    *filetype,
    *magick;

  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  WizardAssert(CipherDomain,random_info != (RandomInfo *) NULL);
  WizardAssert(CipherDomain,random_info->signature == WizardSignature);
  WizardAssert(CipherDomain,exception != (ExceptionInfo *) NULL);
  LockSemaphoreInfo(random_info->semaphore);
  file_info=AcquireFileInfo((const char *) NULL,RandomFilename,WriteFileMode,
    exception);
  if (file_info == (FileInfo *) NULL)
    {
      UnlockSemaphoreInfo(random_info->semaphore);
      return(WizardFalse);
    }
  magick=GetWizardMagick(WizardMagick,sizeof(WizardMagick));
  status=WriteFileChunk(file_info,GetStringInfoDatum(magick),
    GetStringInfoLength(magick));
  magick=DestroyStringInfo(magick);
  filetype=GetWizardMagick((unsigned char *) RandomFiletype,strlen(
    RandomFiletype));
  status|=WriteFileChunk(file_info,GetStringInfoDatum(filetype),
    GetStringInfoLength(filetype));
  filetype=DestroyStringInfo(filetype);
  status|=WriteFile16Bits(file_info,random_info->protocol_major);
  status|=WriteFile16Bits(file_info,random_info->protocol_minor);
  status|=WriteFile64Bits(file_info,(WizardSizeType) random_info->timestamp);
  length=GetStringInfoLength(GetHMACDigest(random_info->hmac_info));
  status|=WriteFile32Bits(file_info,length);
  status|=WriteFileChunk(file_info,GetStringInfoDatum(GetHMACDigest(
    random_info->hmac_info)),length);
  status|=WriteFile64Bits(file_info,GetStringInfoCRC(GetHMACDigest(
    random_info->hmac_info)));
  if (status == WizardFalse)
    (void) ThrowWizardException(exception,GetWizardModule(),RandomError,
      "unable to write random reservior `%s': %s",GetFilePath(file_info),
      strerror(errno));
  file_info=DestroyFileInfo(file_info,exception);
  UnlockSemaphoreInfo(random_info->semaphore);
  return(WizardTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e e d P s e u d o R a n d o m G e n e r a t o r                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SeedPseudoRandomGenerator() initializes the pseudo-random number generator
%  with a random seed.
%
%  The format of the SeedPseudoRandomGenerator method is:
%
%      void SeedPseudoRandomGenerator(const unsigned long seed)
%
%  A description of each parameter follows:
%
%    o seed: the seed.
%
*/
WizardExport void SeedPseudoRandomGenerator(const unsigned long seed)
{
  random_seed=seed;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t R a n d o m K e y                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetRandomKey() sets a random key from the reservoir.
%
%  The format of the SetRandomKey method is:
%
%      void SetRandomKey(RandomInfo *random_info,const size_t length,
%        unsigned char *key)
%
%  A description of each parameter follows:
%
%    o random_info: the random info.
%
%    o length: the key length.
%
%    o key: the key.
%
*/

static inline void IncrementRandomNonce(StringInfo *nonce)
{
  register ssize_t
    i;

  unsigned char
    *datum;

  datum=GetStringInfoDatum(nonce);
  for (i=(ssize_t) (GetStringInfoLength(nonce)-1); i != 0; i--)
  {
    datum[i]++;
    if (datum[i] != 0)
      return;
  }
  ThrowFatalException(RandomFatalError,"Sequence wrap error `%s'");
}

WizardExport void SetRandomKey(RandomInfo *random_info,const size_t length,
  unsigned char *key)
{
  HMACInfo
    *hmac_info;

  register size_t
    i;

  register unsigned char
    *p;

  unsigned char
    *datum;

  WizardAssert(CipherDomain,random_info != (RandomInfo *) NULL);
  if (length == 0)
    return;
  LockSemaphoreInfo(random_info->semaphore);
  i=length;
  hmac_info=random_info->hmac_info;
  datum=GetStringInfoDatum(random_info->reservoir);
  for (p=key; (i != 0) && (random_info->i != 0); i--)
  {
    *p++=datum[random_info->i];
    random_info->i++;
    if (random_info->i == GetHMACDigestsize(hmac_info))
      random_info->i=0;
  }
  while (i >= GetHMACDigestsize(hmac_info))
  {
    ResetHMAC(hmac_info);
    UpdateHMAC(hmac_info,random_info->nonce);
    FinalizeHMAC(hmac_info);
    IncrementRandomNonce(random_info->nonce);
    (void) CopyWizardMemory(p,GetStringInfoDatum(GetHMACDigest(hmac_info)),
      GetHMACDigestsize(hmac_info));
    p+=GetHMACDigestsize(hmac_info);
    i-=GetHMACDigestsize(hmac_info);
  }
  if (i != 0)
    {
      ResetHMAC(hmac_info);
      UpdateHMAC(hmac_info,random_info->nonce);
      FinalizeHMAC(hmac_info);
      IncrementRandomNonce(random_info->nonce);
      SetStringInfo(random_info->reservoir,GetHMACDigest(hmac_info));
      random_info->i=i;
      datum=GetStringInfoDatum(random_info->reservoir);
      while (i-- != 0)
        p[i]=datum[i];
    }
  UnlockSemaphoreInfo(random_info->semaphore);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t R a n d o m T r u e R a n d o m                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetRandomTrueRandom() declares your intentions to use true random numbers.
%  True random numbers are encouraged but may not always be practical because
%  your application may block while entropy is gathered from your environment.
%
%  The format of the SetRandomTrueRandom method is:
%
%      void SetRandomTrueRandom(const WizardBooleanType true_random)
%
%  A description of each parameter follows:
%
%    o true_random: declare your intentions to use true-random number.
%
*/
WizardExport void SetRandomTrueRandom(const WizardBooleanType true_random)
{
  gather_true_random=true_random;
}
