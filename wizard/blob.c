/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                         BBBB   L       OOO   BBBB                           %
%                         B   B  L      O   O  B   B                          %
%                         BBBB   L      O   O  BBBB                           %
%                         B   B  L      O   O  B   B                          %
%                         BBBB   LLLLL   OOO   BBBB                           %
%                                                                             %
%                                                                             %
%                  Wizard's Toolkit Binary Large OBjectS Methods              %
%                                                                             %
%                                                                             %
%                              Software Design                                %
%                                  Cristy                                     %
%                                March 2003                                   %
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
%
%
*/

/*
  Include declarations.
*/
#include "wizard/studio.h"
#include "wizard/blob.h"
#include "wizard/blob-private.h"
#include "wizard/cipher.h"
#include "wizard/exception.h"
#include "wizard/exception-private.h"
#include "wizard/memory_.h"
#include "wizard/semaphore.h"
#include "wizard/nt-base.h"
#include "wizard/string-private.h"
#include "wizard/utility.h"
#include "wizard/utility-private.h"
#include "bzlib.h"
#include "zlib.h"

/*
  Define declarations.
*/
#define WizardMaxBlobExtent  65541
# if !defined(MAP_ANONYMOUS) && defined(MAP_ANON)
#  define MAP_ANONYMOUS MAP_ANON
# endif
#if !defined(MAP_FAILED)
#define MAP_FAILED      ((void *) -1)
#endif

/*
  Typedef declarations.
*/
typedef enum
{
  UndefinedStream,
  StandardStream,
  FileStream,
  PipeStream,
  ZipStream,
  BZipStream,
  BlobStream
} StreamType;

typedef union BlobFileInfo
{
  FILE
    *file;

#if defined(WIZARDSTOOLKIT_ZLIB_DELEGATE)
  gzFile
    gzfile;
#endif

#if defined(WIZARDSTOOLKIT_BZLIB_DELEGATE)
  BZFILE
    *bzfile;
#endif
} BlobFileInfo;

struct _BlobInfo
{
  char
    filename[MaxTextExtent];

  size_t
    length,
    extent,
    quantum;

  WizardBooleanType
    mapped,
    eof;

  WizardOffsetType
    offset;

  WizardSizeType
    size;

  WizardBooleanType
    exempt,
    status,
    temporary;

  StreamType
    type;

  BlobFileInfo
    file_info;

  struct stat
    properties;

  unsigned char
    *data;

  WizardBooleanType
    debug;

  SemaphoreInfo
    *semaphore;

  ssize_t
    reference_count;

  size_t
    signature;
};

/*
  Forward declarations.
*/
static unsigned char
  *DetachBlob(BlobInfo *);

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   A t t a c h B l o b                                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AttachBlob() attaches a blob to the BlobInfo structure.
%
%  The format of the AttachBlob method is:
%
%      void AttachBlob(BlobInfo *blob_info,const void *blob,const size_t length)
%
%  A description of each parameter follows:
%
%    o blob_info: the blob info.
%
%    o blob: the character stream to attach.
%
%    o length: the length in bytes of the blob.
%
*/
static void AttachBlob(BlobInfo *blob_info,const void *blob,const size_t length)
{
  assert(blob_info != (BlobInfo *) NULL);
  if (blob_info->debug != WizardFalse)
    (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  blob_info->length=length;
  blob_info->extent=length;
  blob_info->quantum=(size_t) WizardMaxBlobExtent;
  blob_info->offset=0;
  blob_info->type=BlobStream;
  blob_info->file_info.file=(FILE *) NULL;
  blob_info->data=(unsigned char *) blob;
  blob_info->mapped=WizardFalse;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   C l o s e B l o b                                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  CloseBlob() closes a stream associated with the blob_info.
%
%  The format of the CloseBlob method is:
%
%      WizardBooleanType CloseBlob(BlobInfo *blob_info)
%
%  A description of each parameter follows:
%
%    o blob_info: the blob info.
%
*/
WizardExport WizardBooleanType CloseBlob(BlobInfo *blob_info)
{
  int
    status;

  /*
    Close blob_info file.
  */
  assert(blob_info != (BlobInfo *) NULL);
  assert(blob_info->signature == WizardSignature);
  if (blob_info->debug != WizardFalse)
    (void) LogWizardEvent(TraceEvent,GetWizardModule(),"%s",
      blob_info->filename);
  if (blob_info->type == UndefinedStream)
    return(WizardTrue);
  status=SyncBlob(blob_info);
  switch (blob_info->type)
  {
    case UndefinedStream:
    case StandardStream:
      break;
    case FileStream:
    case PipeStream:
    {
      status=ferror(blob_info->file_info.file);
      break;
    }
    case ZipStream:
    {
#if defined(WIZARDSTOOLKIT_ZLIB_DELEGATE)
      (void) gzerror(blob_info->file_info.gzfile,&status);
#endif
      break;
    }
    case BZipStream:
    {
#if defined(WIZARDSTOOLKIT_BZLIB_DELEGATE)
      (void) BZ2_bzerror(blob_info->file_info.bzfile,&status);
#endif
      break;
    }
    case BlobStream:
      break;
  }
  blob_info->status=status < 0 ? WizardTrue : WizardFalse;
  blob_info->size=GetBlobSize(blob_info);
  blob_info->eof=WizardFalse;
  if (blob_info->exempt != WizardFalse)
    {
      blob_info->type=UndefinedStream;
      return(blob_info->status);
    }
  switch (blob_info->type)
  {
    case UndefinedStream:
    case StandardStream:
      break;
    case FileStream:
    {
      status=fclose(blob_info->file_info.file);
      break;
    }
    case PipeStream:
    {
#if defined(WIZARDSTOOLKIT_HAVE_POPEN)
      status=pclose(blob_info->file_info.file);
#endif
      break;
    }
    case ZipStream:
    {
#if defined(WIZARDSTOOLKIT_ZLIB_DELEGATE)
      status=gzclose(blob_info->file_info.gzfile);
#endif
      break;
    }
    case BZipStream:
    {
#if defined(WIZARDSTOOLKIT_BZLIB_DELEGATE)
      BZ2_bzclose(blob_info->file_info.bzfile);
#endif
      break;
    }
    case BlobStream:
      break;
  }
  (void) DetachBlob(blob_info);
  blob_info->status=status < 0 ? WizardTrue : WizardFalse;
  return(blob_info->status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   D e s t r o y B l o b                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DestroyBlob() deallocates memory associated with a blob.
%
%  The format of the DestroyBlob method is:
%
%      BlobInfo *DestroyBlob(BlobInfo *blob_info)
%
%  A description of each parameter follows:
%
%    o blob_info: the blob info.
%
*/
WizardExport BlobInfo *DestroyBlob(BlobInfo *blob_info)
{
  WizardBooleanType
    destroy;

  assert(blob_info != (BlobInfo *) NULL);
  assert(blob_info->signature == WizardSignature);
  if (blob_info->debug != WizardFalse)
    (void) LogWizardEvent(TraceEvent,GetWizardModule(),"%s",
      blob_info->filename);
  destroy=WizardFalse;
  LockSemaphoreInfo(blob_info->semaphore);
  blob_info->reference_count--;
  if (blob_info->reference_count == 0)
    destroy=WizardTrue;
  UnlockSemaphoreInfo(blob_info->semaphore);
  if (destroy == WizardFalse)
    return(blob_info);
  (void) CloseBlob(blob_info);
  if (blob_info->mapped != WizardFalse)
    {
      (void) UnmapBlob(blob_info->data,blob_info->length);
      RelinquishWizardResource(MapResource,blob_info->length);
    }
  if (blob_info->semaphore != (SemaphoreInfo *) NULL)
    RelinquishSemaphoreInfo(&blob_info->semaphore);
  blob_info->signature=(~WizardSignature);
  blob_info=(BlobInfo *) RelinquishWizardMemory(blob_info);
  return(blob_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   D e t a c h B l o b                                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DetachBlob() detaches a blob from the BlobInfo structure.
%
%  The format of the DetachBlob method is:
%
%      unsigned char *DetachBlob(BlobInfo *blob_info)
%
%  A description of each parameter follows:
%
%    o blob_info: the blob info.
%
*/
static unsigned char *DetachBlob(BlobInfo *blob_info)
{
  unsigned char
    *data;

  assert(blob_info != (BlobInfo *) NULL);
  if (blob_info->debug != WizardFalse)
    (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  if (blob_info->mapped != WizardFalse)
    {
      (void) UnmapBlob(blob_info->data,blob_info->length);
      blob_info->data=(unsigned char *) NULL;
      RelinquishWizardResource(MapResource,blob_info->length);
    }
  blob_info->mapped=WizardFalse;
  blob_info->length=0;
  blob_info->offset=0;
  blob_info->eof=WizardFalse;
  blob_info->exempt=WizardFalse;
  blob_info->type=UndefinedStream;
  blob_info->file_info.file=(FILE *) NULL;
  data=blob_info->data;
  blob_info->data=(unsigned char *) NULL;
  return(data);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+  E O F B l o b                                                              %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  EOFBlob() returns a non-zero value when EOF has been detected reading from
%  a blob or file.
%
%  The format of the EOFBlob method is:
%
%      int EOFBlob(BlobInfo *blob_info)
%
%  A description of each parameter follows:
%
%    o blob_info: the blob info.
%
*/
WizardExport int EOFBlob(BlobInfo *blob_info)
{
  assert(blob_info != (BlobInfo *) NULL);
  assert(blob_info->signature == WizardSignature);
  if (blob_info->debug != WizardFalse)
    (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  assert(blob_info->type != UndefinedStream);
  switch (blob_info->type)
  {
    case UndefinedStream:
    case StandardStream:
      break;
    case FileStream:
    case PipeStream:
    {
      blob_info->eof=feof(blob_info->file_info.file) != 0 ? WizardTrue :
        WizardFalse;
      break;
    }
    case ZipStream:
    {
      blob_info->eof=WizardFalse;
      break;
    }
    case BZipStream:
    {
#if defined(WIZARDSTOOLKIT_BZLIB_DELEGATE)
      int
        status;

      status=0;
      (void) BZ2_bzerror(blob_info->file_info.bzfile,&status);
      blob_info->eof=status == BZ_UNEXPECTED_EOF ? WizardTrue : WizardFalse;
#endif
      break;
    }
    case BlobStream:
      break;
  }
  return((int) blob_info->eof);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   F i l e T o B l o b                                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  FileToBlob() returns the contents of a file as a blob.  It returns the
%  file as a blob and its length.  If an error occurs, NULL is returned.
%
%  The format of the FileToBlob method is:
%
%      void *FileToBlob(const char *filename,const size_t extent,size_t *length,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o blob:  FileToBlob() returns the contents of a file as a blob.  If
%      an error occurs NULL is returned.
%
%    o filename: The filename.
%
%    o extent:  The maximum length of the blob.
%
%    o length: On return, it reflects the actual length of the blob.
%
%    o exception: Return any errors or warnings in this structure.
%
*/
WizardExport unsigned char *FileToBlob(const char *filename,const size_t extent,
  size_t *length,ExceptionInfo *exception)
{
  int
    file;

  register size_t
    i;

  ssize_t
    count;

  unsigned char
    *blob;

  WizardOffsetType
    offset;

  void
    *map;

  assert(filename != (const char *) NULL);
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"%s",filename);
  assert(exception != (ExceptionInfo *) NULL);
  *length=0;
  file=fileno(stdin);
  if (strcmp(filename,"-") != 0)
    file=open_utf8(filename,O_RDONLY | O_BINARY,0);
  if (file == -1)
    {
      (void) ThrowWizardException(exception,GetWizardModule(),BlobError,
        "unable to open file `%s': %s",filename,strerror(errno));
      return((unsigned char *) NULL);
    }
  offset=(WizardOffsetType) lseek(file,0,SEEK_END);
  count=0;
  if ((file == fileno(stdin)) || (offset < 0) ||
      (offset != (WizardOffsetType) ((ssize_t) offset)))
    {
      size_t
        quantum;

      struct stat
        file_info;

      /*
        Stream is not seekable.
      */
      offset=(WizardOffsetType) lseek(file,0,SEEK_SET);
      quantum=(size_t) WizardMaxBufferExtent;
      if ((fstat(file,&file_info) == 0) && (file_info.st_size != 0))
        quantum=Min((size_t) file_info.st_size,WizardMaxBufferExtent);
      blob=(unsigned char *) AcquireQuantumMemory(quantum,sizeof(*blob));
      for (i=0; blob != (unsigned char *) NULL; i+=count)
      {
        count=read(file,blob+i,quantum);
        if (count <= 0)
          {
            count=0;
            if (errno != EINTR)
              break;
          }
        if (~((size_t) i) < (quantum+1))
          {
            blob=(unsigned char *) RelinquishWizardMemory(blob);
            break;
          }
        blob=(unsigned char *) ResizeQuantumMemory(blob,i+quantum+1,
          sizeof(*blob));
        if ((size_t) (i+count) >= extent)
          break;
      }
      if (close(file) == -1)
        (void) ThrowWizardException(exception,GetWizardModule(),BlobError,
          "unable to close file `%s': %s",filename,strerror(errno));
      if (blob == (unsigned char *) NULL)
        {
          (void) ThrowWizardException(exception,GetWizardModule(),ResourceError,
            "memory allocation failed: `%s'",filename);
          return((unsigned char *) NULL);
        }
      *length=Min(i+count,extent);
      blob[*length]='\0';
      return(blob);
    }
  *length=Min((size_t) offset,extent);
  blob=(unsigned char *) NULL;
  if (~(*length) >= (MaxCipherBlocksize-1))
    blob=(unsigned char *) AcquireQuantumMemory(*length+MaxCipherBlocksize,
      sizeof(*blob));
  if (blob == (unsigned char *) NULL)
    {
      if (close(file) == -1)
        {
          (void) ThrowWizardException(exception,GetWizardModule(),BlobError,
            "unable to close file `%s': %s",filename,strerror(errno));
          return((unsigned char *) NULL);
        }
      (void) ThrowWizardException(exception,GetWizardModule(),BlobError,
        "memory allocation failed `%s'",strerror(errno));
      return((unsigned char *) NULL);
    }
  map=MapBlob(file,ReadMode,0,*length);
  if (map != (void *) NULL)
    {
      (void) memcpy(blob,map,*length);
      if (UnmapBlob(map,*length) == WizardFalse)
        (void) ThrowWizardException(exception,GetWizardModule(),BlobError,
          "unable to unmap blob `%s': %s",filename,strerror(errno));
    }
  else
    {
      register size_t
        i;

      ssize_t
        count;

      if (lseek(file,0,SEEK_SET) < 0)
        (void) ThrowWizardException(exception,GetWizardModule(),BlobError,
          "unable to seek blob `%s': %s",filename,strerror(errno));
      for (i=0; i < *length; i+=count)
      {
        count=read(file,blob+i,Min(*length-i,(size_t) SSIZE_MAX));
        if (count <= 0)
          {
            count=0;
            if (errno != EINTR)
              break;
          }
      }
      if (i < *length)
        {
          if (close(file) == -1)
            (void) ThrowWizardException(exception,GetWizardModule(),BlobError,
              "unable to close file `%s': %s",filename,strerror(errno));
          blob=(unsigned char *) RelinquishWizardMemory(blob);
          (void) ThrowWizardException(exception,GetWizardModule(),BlobError,
            "unable to read file `%s'",filename);
          return((unsigned char *) NULL);
        }
    }
  if (close(file) == -1)
    (void) ThrowWizardException(exception,GetWizardModule(),BlobError,
      "unable to close file `%s': %s",filename,strerror(errno));
  blob[*length]=(unsigned char) '\0';
  return(blob);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+  G e t B l o b F i l e n a m e                                              %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetBlobFilename() returns the blob filename.
%
%  The format of the GetBlobFilename method is:
%
%      const char *GetBlobFilename(const BlobInfo *blob_info)
%
%  A description of each parameter follows:
%
%    o blob_info: the blob info info.
%
*/
WizardExport const char *GetBlobFilename(const BlobInfo *blob_info)
{
  assert(blob_info != (BlobInfo *) NULL);
  if (blob_info->debug != WizardFalse)
    (void) LogWizardEvent(TraceEvent,GetWizardModule(),"%s",
      blob_info->filename);
  return(blob_info->filename);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   G e t B l o b I n f o                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetBlobInfo() initializes the BlobInfo structure.
%
%  The format of the GetBlobInfo method is:
%
%      void GetBlobInfo(BlobInfo *blob_info)
%
%  A description of each parameter follows:
%
%    o blob_info: Specifies a pointer to a BlobInfo structure.
%
*/
WizardExport void GetBlobInfo(BlobInfo *blob_info)
{
  assert(blob_info != (BlobInfo *) NULL);
  (void) ResetWizardMemory(blob_info,0,sizeof(*blob_info));
  blob_info->type=UndefinedStream;
  blob_info->quantum=(size_t) WizardMaxBlobExtent;
  blob_info->debug=IsEventLogging();
  blob_info->reference_count=1;
  blob_info->semaphore=AcquireSemaphoreInfo();
  blob_info->signature=WizardSignature;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+  G e t B l o b S i z e                                                      %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetBlobSize() returns the current length of the blob; zero is returned if
%  the size cannot be determined.
%
%  The format of the GetBlobSize method is:
%
%      WizardSizeType GetBlobSize(BlobInfo *blob_info)
%
%  A description of each parameter follows:
%
%    o blob_info: the blob info.
%
*/
WizardExport WizardSizeType GetBlobSize(BlobInfo *blob_info)
{
  WizardSizeType
    length;

  assert(blob_info != (BlobInfo *) NULL);
  if (blob_info->debug != WizardFalse)
    (void) LogWizardEvent(TraceEvent,GetWizardModule(),"%s",
      blob_info->filename);
  length=0;
  switch (blob_info->type)
  {
    case UndefinedStream:
    {
      length=blob_info->size;
      break;
    }
    case StandardStream:
    {
      length=blob_info->size;
      break;
    }
    case FileStream:
    {
      if (fstat(fileno(blob_info->file_info.file),&blob_info->properties) == 0)
        length=(WizardSizeType) blob_info->properties.st_size;
      break;
    }
    case PipeStream:
    {
      length=blob_info->size;
      break;
    }
    case ZipStream:
    {
#if defined(WIZARDSTOOLKIT_ZLIB_DELEGATE)
      if (fstat(fileno(blob_info->file_info.file),&blob_info->properties) == 0)
        length=(WizardSizeType) blob_info->properties.st_size;
#endif
      break;
    }
    case BZipStream:
    {
#if defined(WIZARDSTOOLKIT_BZLIB_DELEGATE)
      if (fstat(fileno(blob_info->file_info.file),&blob_info->properties) == 0)
        length=(WizardSizeType) blob_info->properties.st_size;
#endif
      break;
    }
    case BlobStream:
    {
      length=(WizardSizeType) blob_info->length;
      break;
    }
  }
  return(length);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  G e t B l o b P r o p e r t i e s                                          %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetBlobProperties() returns information about a blob.
%
%  The format of the GetBlobProperties method is:
%
%      const struct stat *GetBlobProperties(const BlobInfo *blob_info)
%
%  A description of each parameter follows:
%
%    o blob_info: the blob info.
%
*/
WizardExport const struct stat *GetBlobProperties(const BlobInfo *blob_info)
{
  assert(blob_info != (BlobInfo *) NULL);
  if (blob_info->debug != WizardFalse)
    (void) LogWizardEvent(TraceEvent,GetWizardModule(),"%s",
      blob_info->filename);
  return(&blob_info->properties);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+  M a p B l o b                                                              %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MapBlob() creates a mapping from a file to a binary large object.
%
%  The format of the MapBlob method is:
%
%      void *MapBlob(int file,const MapMode mode,const WizardOffsetType offset,
%        const size_t length)
%
%  A description of each parameter follows:
%
%    o file: map this file descriptor.
%
%    o mode: ReadMode, WriteMode, or IOMode.
%
%    o offset: starting at this offset within the file.
%
%    o length: the length of the mapping is returned in this pointer.
%
*/
WizardExport void *MapBlob(int file,const MapMode mode,
  const WizardOffsetType offset,const size_t length)
{
#if defined(WIZARDSTOOLKIT_HAVE_MMAP_FILEIO)
  int
    flags,
    protection;

  unsigned char
    *map;

  /*
    Map file.
  */
  flags=0;
  if (file == -1)
#if defined(MAP_ANONYMOUS)
    flags|=MAP_ANONYMOUS;
#else
    return((unsigned char *) NULL);
#endif
  flags|=MAP_PRIVATE;
  switch (mode)
  {
    case ReadMode:
    default:
    {
      protection=PROT_READ;
      flags|=MAP_PRIVATE;
      break;
    }
    case WriteMode:
    {
      protection=PROT_WRITE;
      flags|=MAP_SHARED;
      break;
    }
    case IOMode:
    {
      protection=PROT_READ | PROT_WRITE;
      flags|=MAP_SHARED;
      break;
    }
  }
#if !defined(WIZARDSTOOLKIT_HAVE_HUGEPAGES) || !defined(MAP_HUGETLB)
  map=(unsigned char *) mmap((char *) NULL,length,protection,flags,file,
    (off_t) offset);
#else
  map=(unsigned char *) mmap((char *) NULL,length,protection,flags |
    MAP_HUGETLB,file,(off_t) offset);
  if (map == (unsigned char *) MAP_FAILED)
    map=(unsigned char *) mmap((char *) NULL,length,protection,flags,file,
      (off_t) offset);
#endif
  if (map == (unsigned char *) MAP_FAILED)
    return((unsigned char *) NULL);
  return(map);
#else
  return((unsigned char *) NULL);
#endif
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   O p e n B l o b                                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  OpenBlob() opens a file associated with the blob.  A file name of '-' sets
%  the file to stdin for type 'r' and stdout for type 'w'.  If the filename
%  suffix is '.gz' or '.Z', the blob_info is decompressed for type 'r' and
%  compressed for type 'w'.  If the filename prefix is '|', it is piped to or
%  from a system command.
%
%  The format of the OpenBlob method is:
%
%       BlobInfo *OpenBlob(const char *filename,const BlobMode mode,
%         const WizardBooleanType compress,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o filename: the filename.
%
%    o mode: the mode for opening the file.
%
%    o compress: a value other than 0 (de)compresses BZIP or ZIP files.
%
%    o exception: return any errors or warnings in this structure.
*/
WizardExport BlobInfo *OpenBlob(const char *filename,const BlobMode mode,
  const WizardBooleanType compress,ExceptionInfo *exception)
{
  BlobInfo
    *blob_info;

  const char
    *type;

  WizardBooleanType
    status;

  assert(filename != (const char *) NULL);
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"%s",filename);
  assert(exception != (ExceptionInfo *) NULL);
  blob_info=(BlobInfo *) AcquireWizardMemory(sizeof(*blob_info));
  if (blob_info == (BlobInfo *) NULL)
    {
      (void) ThrowWizardException(exception,GetWizardModule(),ResourceError,
        "memory allocation failed: `%s'",filename);
      return((BlobInfo *) NULL);
    }
  GetBlobInfo(blob_info);
  switch (mode)
  {
    default: type="r"; break;
    case ReadBlobMode: type="r"; break;
    case ReadBinaryBlobMode: type="rb"; break;
    case WriteBlobMode: type="w"; break;
    case WriteBinaryBlobMode: type="w+b"; break;
  }
  /*
    Open file.
  */
  (void) CopyWizardString(blob_info->filename,filename,MaxTextExtent);
  if (LocaleCompare(filename,"-") == 0)
    {
      blob_info->file_info.file=(*type == 'r') ? stdin : stdout;
#if defined(WIZARDSTOOLKIT_WINDOWS_SUPPORT)
      if (strchr(type,'b') != (char *) NULL)
        setmode(_fileno(blob_info->file_info.file),_O_BINARY);
#endif
      blob_info->type=StandardStream;
      blob_info->exempt=WizardTrue;
      return(blob_info);
    }
  if (LocaleNCompare(filename,"fd:",3) == 0)
    {
      char
        mode[MaxTextExtent];

      *mode=(*type);
      mode[1]='\0';
      blob_info->file_info.file=fdopen((long) StringToLong(filename+3),mode);
#if defined(WIZARDSTOOLKIT_WINDOWS_SUPPORT)
      if (strchr(type,'b') != (char *) NULL)
        setmode(_fileno(blob_info->file_info.file),_O_BINARY);
#endif
      blob_info->type=StandardStream;
      blob_info->exempt=WizardTrue;
      return(blob_info);
    }
#if defined(WIZARDSTOOLKIT_HAVE_POPEN)
  if (*filename == '|')
    {
      char
        mode[MaxTextExtent];

      /*
        Pipe blob_info to or from a system command.
      */
#if defined(SIGPIPE)
      if (*type == 'w')
        (void) signal(SIGPIPE,SIG_IGN);
#endif
      *mode=(*type);
      mode[1]='\0';
      blob_info->file_info.file=(FILE *) popen_utf8(filename+1,mode);
      if (blob_info->file_info.file == (FILE *) NULL)
        {
          (void) ThrowWizardException(exception,GetWizardModule(),BlobError,
            "unable to open file `%s': %s",filename,strerror(errno));
          blob_info=(BlobInfo *) RelinquishWizardMemory(blob_info);
          return((BlobInfo *) NULL);
        }
      blob_info->type=PipeStream;
      blob_info->exempt=WizardTrue;
      return(blob_info);
    }
#endif
  status=stat_utf8(filename,&blob_info->properties) == 0 ? WizardTrue :
    WizardFalse;
#if defined(S_ISFIFO)
  if ((status == WizardTrue) && S_ISFIFO(blob_info->properties.st_mode))
    {
      blob_info->file_info.file=fopen_utf8(filename,type);
      if (blob_info->file_info.file == (FILE *) NULL)
        {
          (void) ThrowWizardException(exception,GetWizardModule(),BlobError,
            "unable to open file `%s': %s",filename,strerror(errno));
          blob_info=(BlobInfo *) RelinquishWizardMemory(blob_info);
          return((BlobInfo *) NULL);
        }
      blob_info->type=FileStream;
      blob_info->exempt=WizardTrue;
      return(blob_info);
    }
#endif
  if (*type == 'r')
    {
      blob_info->file_info.file=fopen_utf8(filename,type);
      if (blob_info->file_info.file != (FILE *) NULL)
        {
          size_t
            length;

          ssize_t
            count;

          unsigned char
            magick[3];

          blob_info->type=FileStream;
#if defined(WIZARDSTOOLKIT_HAVE_SETVBUF)
          (void) setvbuf(blob_info->file_info.file,(char *) NULL,(int) _IOFBF,
            16384);
#endif
          (void) ResetWizardMemory(magick,0,sizeof(magick));
          count=(ssize_t) fread(magick,1,sizeof(magick),
            blob_info->file_info.file);
          (void) fseek(blob_info->file_info.file,-((off_t) count),SEEK_CUR);
          (void) fflush(blob_info->file_info.file);
          (void) LogWizardEvent(BlobEvent,GetWizardModule(),
            "  read %.20g magic header bytes",(double) count);
#if defined(WIZARDSTOOLKIT_ZLIB_DELEGATE)
          if ((compress != WizardFalse) && ((int) magick[0] == 0x1F) &&
              ((int) magick[1] == 0x8B) && ((int) magick[2] == 0x08))
            {
              (void) fclose(blob_info->file_info.file);
              blob_info->type=UndefinedStream;
              blob_info->file_info.gzfile=gzopen(filename,type);
              if (blob_info->file_info.gzfile != (gzFile) NULL)
                blob_info->type=ZipStream;
            }
#endif
#if defined(WIZARDSTOOLKIT_BZLIB_DELEGATE)
          if ((compress != WizardFalse) &&
              (strncmp((char *) magick,"BZh",3) == 0))
            {
              (void) fclose(blob_info->file_info.file);
              blob_info->type=UndefinedStream;
              blob_info->file_info.bzfile=BZ2_bzopen(filename,type);
              if (blob_info->file_info.bzfile != (BZFILE *) NULL)
                blob_info->type=BZipStream;
            }
#endif
          length=(size_t) blob_info->properties.st_size;
          if ((blob_info->type == FileStream) &&
              (length > WizardMaxBufferExtent) &&
              (AcquireWizardResource(MapResource,length) != WizardFalse))
            {
              void
                *blob;

              blob=MapBlob(fileno(blob_info->file_info.file),ReadMode,0,length);
              if (blob == (void *) NULL)
                RelinquishWizardResource(MapResource,length);
              else
                {
                  /*
                    Use memory-mapped I/O.
                  */
                  (void) fclose(blob_info->file_info.file);
                  blob_info->file_info.file=(FILE *) NULL;
                  AttachBlob(blob_info,blob,length);
                  blob_info->mapped=WizardTrue;
                }
            }
        }
    }
  else
    {
      char
        extension[MaxTextExtent];

      GetPathComponent(filename,ExtensionPath,extension);
#if defined(WIZARDSTOOLKIT_ZLIB_DELEGATE)
      if ((compress != WizardFalse) &&
          ((LocaleCompare(extension,"Z") == 0) ||
           (LocaleCompare(extension,"gz") == 0)))
        {
          blob_info->file_info.gzfile=gzopen(filename,type);
          if (blob_info->file_info.gzfile != (gzFile) NULL)
            blob_info->type=ZipStream;
        }
      else
#endif
#if defined(WIZARDSTOOLKIT_BZLIB_DELEGATE)
        if ((compress != WizardFalse) && (LocaleCompare(extension,"bz2") == 0))
          {
            blob_info->file_info.bzfile=BZ2_bzopen(filename,type);
            if (blob_info->file_info.gzfile != (BZFILE *) NULL)
              blob_info->type=BZipStream;
          }
        else
#endif
          {
            blob_info->file_info.file=fopen_utf8(filename,type);
            if (blob_info->file_info.file != (FILE *) NULL)
              {
                blob_info->type=FileStream;
#if defined(WIZARDSTOOLKIT_HAVE_SETVBUF)
                (void) setvbuf(blob_info->file_info.file,(char *) NULL,(int) _IOFBF,
                  16384);
#endif
              }
          }
    }
  blob_info->status=WizardFalse;
  if (blob_info->type != UndefinedStream)
    blob_info->size=GetBlobSize(blob_info);
  else
    {
      (void) ThrowWizardException(exception,GetWizardModule(),BlobError,
        "unable to open file `%s': %s",filename,strerror(errno));
      blob_info=(BlobInfo *) RelinquishWizardMemory(blob_info);
      return((BlobInfo *) NULL);
    }
  return(blob_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+  R e a d B l o b                                                            %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadBlob() reads data from the blob or file and returns it.  It
%  returns the number of bytes read. If length is zero, ReadBlob() returns
%  zero and has no other results. If length is greater than SSIZE_MAX, the
%  result is unspecified.
%
%  The format of the ReadBlob method is:
%
%      ssize_t ReadBlob(BlobInfo *blob_info,const size_t length,
%        unsigned char *data)
%
%  A description of each parameter follows:
%
%    o blob_info: the blob info.
%
%    o length: number of bytes to read from the blob.
%
%    o data: area to place the information requested from the blob.
%
*/

static inline WizardSizeType WizardMin(const WizardSizeType x,
  const WizardSizeType y)
{
  if (x < y)
    return(x);
  return(y);
}

WizardExport ssize_t ReadBlob(BlobInfo *blob_info,const size_t length,
  unsigned char *data)
{
  int
    c;

  register unsigned char
    *q;

  ssize_t
    count;

  assert(blob_info != (BlobInfo *) NULL);
  assert(blob_info->signature == WizardSignature);
  assert(blob_info != (BlobInfo *) NULL);
  assert(blob_info->type != UndefinedStream);
  if (length == 0)
    return(0);
  assert(data != (void *) NULL);
  count=0;
  q=data;
  switch (blob_info->type)
  {
    case UndefinedStream:
      break;
    case StandardStream:
    {
      register ssize_t
        i;

      for (i=0; i < (ssize_t) length; i+=count)
      {
        count=read(fileno(blob_info->file_info.file),q+i,(size_t)
          WizardMin(length-i,(WizardSizeType) SSIZE_MAX));
        if (count <= 0)
          {
            count=0;
            if (errno != EINTR)
              break;
          }
      }
      count=i;
      break;
    }
    case FileStream:
    case PipeStream:
    {
      switch (length)
      {
        default:
        {
          count=(ssize_t) fread(q,1,length,blob_info->file_info.file);
          break;
        }
        case 2:
        {
          c=getc(blob_info->file_info.file);
          if (c == EOF)
            break;
          *q++=(unsigned char) c;
          count++;
        }
        case 1:
        {
          c=getc(blob_info->file_info.file);
          if (c == EOF)
            break;
          *q++=(unsigned char) c;
          count++;
        }
        case 0:
          break;
      }
      break;
    }
    case ZipStream:
    {
#if defined(WIZARDSTOOLKIT_ZLIB_DELEGATE)
      switch (length)
      {
        default:
        {
          count=(ssize_t) gzread(blob_info->file_info.gzfile,q,(unsigned int)
            length);
          break;
        }
        case 2:
        {
          c=gzgetc(blob_info->file_info.gzfile);
          if (c == EOF)
            break;
          *q++=(unsigned char) c;
          count++;
        }
        case 1:
        {
          c=gzgetc(blob_info->file_info.gzfile);
          if (c == EOF)
            break;
          *q++=(unsigned char) c;
          count++;
        }
        case 0:
          break;
      }
#endif
      break;
    }
    case BZipStream:
    {
#if defined(WIZARDSTOOLKIT_BZLIB_DELEGATE)
      count=(ssize_t) BZ2_bzread(blob_info->file_info.bzfile,q,(int) length);
#endif
      break;
    }
    case BlobStream:
    {
      register const unsigned char
        *p;

      if (blob_info->offset >= (WizardOffsetType) blob_info->length)
        {
          blob_info->eof=WizardTrue;
          break;
        }
      p=blob_info->data+blob_info->offset;
      count=(ssize_t) WizardMin(length,(size_t) (blob_info->length-
        blob_info->offset));
      blob_info->offset+=count;
      if (count != (ssize_t) length)
        blob_info->eof=WizardTrue;
      (void) memcpy(q,p,(size_t) count);
      break;
    }
  }
  return(count);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+  R e a d B l o b B y t e                                                    %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadBlobByte() reads a single byte from the blob_info file and returns it.
%
%  The format of the ReadBlobByte method is:
%
%      int ReadBlobByte(BlobInfo *blob_info)
%
%  A description of each parameter follows.
%
%    o blob_info: the blob info.
%
*/

static inline const unsigned char *ReadBlobStream(BlobInfo *blob_info,
  const size_t length,unsigned char *data,ssize_t *count)
{
  assert(count != (ssize_t *) NULL);
  assert(blob_info != (BlobInfo *) NULL);
  if (blob_info->type != BlobStream)
    {
      *count=ReadBlob(blob_info,length,data);
      return(data);
    }
  if (blob_info->offset >= (WizardOffsetType) blob_info->length)
    {
      *count=0;
      blob_info->eof=WizardTrue;
      return(data);
    }
  data=blob_info->data+blob_info->offset;
  *count=(ssize_t) WizardMin(length,(size_t) (blob_info->length-
    blob_info->offset));
  blob_info->offset+=(*count);
  if (*count != (ssize_t) length)
    blob_info->eof=WizardTrue;
  return(data);
}

WizardExport int ReadBlobByte(BlobInfo *blob_info)
{
  register const unsigned char
    *p;

  ssize_t
    count;

  unsigned char
    buffer[1];

  assert(blob_info != (BlobInfo *) NULL);
  assert(blob_info->signature == WizardSignature);
  p=ReadBlobStream(blob_info,1,buffer,&count);
  if (count != 1)
    return(EOF);
  return((int) (*p));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+  R e a d B l o b C h u n k                                                  %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadBlobChunk() reads data from the blob and returns it.  It returns the
%  number of bytes read.  ReadBlobChunk() differs from ReadBlob() by making an
%  effort to return the number of bytes that was requested except in the event
%  an EOF is encountered.
%
%  The format of the ReadBlobChunk method is:
%
%      ssize_t ReadBlobChunk(BlobInfo *blob_info,const size_t length,
%        unsigned char *data)
%
%  A description of each parameter follows:
%
%    o blob_info: the blob info.
%
%    o length: the number of bytes to read from the blob.
%
%    o data: the area to place the information requested from the blob.
%
*/
WizardExport ssize_t ReadBlobChunk(BlobInfo *blob_info,const size_t length,
  unsigned char *data)
{
  register ssize_t
    i;

  ssize_t
    count;

  assert(blob_info != (BlobInfo *) NULL);
  assert(blob_info->signature == WizardSignature);
  assert(blob_info->type != UndefinedStream);
  assert(data != (void *) NULL);
  if (blob_info->type == BlobStream)
    return(ReadBlob(blob_info,length,data));
  count=0;
  for (i=0; i < (ssize_t) length; i+=count)
  {
    count=ReadBlob(blob_info,(size_t) WizardMin(length-i,(WizardSizeType)
      SSIZE_MAX),data+i);
    if (count <= 0)
      {
        count=0;
        if (errno != EINTR)
          break;
      }
  }
  return(i);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+  S e t B l o b E x t e n t                                                  %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetBlobExtent() ensures enough space is allocated for the blob.  If the
%  method is successful, subsequent writes to bytes in the specified range are
%  guaranteed not to fail.
%
%  The format of the SetBlobExtent method is:
%
%      WizardBooleanType SetBlobExtent(BlobInfo *blob_info,
%        const WizardSizeType extent)
%
%  A description of each parameter follows:
%
%    o blob_info: the blob info.
%
%    o extent:  the blob maximum extent.
%
*/
WizardExport WizardBooleanType SetBlobExtent(BlobInfo *blob_info,
  const WizardSizeType extent)
{
  assert(blob_info != (BlobInfo *) NULL);
  assert(blob_info->signature == WizardSignature);
  switch (blob_info->type)
  {
    case UndefinedStream:
      break;
    case StandardStream:
    {
      return(WizardFalse);
      break;
    }
    case FileStream:
    {
      ssize_t
        count;

      WizardOffsetType
        offset;

      if (extent != (WizardSizeType) ((off_t) extent))
        return(WizardFalse);
      offset=fseek(blob_info->file_info.file,0,SEEK_END);
      if (offset < 0)
        return(WizardFalse);
      if ((WizardSizeType) offset >= extent)
        break;
      offset=fseek(blob_info->file_info.file,(WizardOffsetType) extent-1,
        SEEK_SET);
      count=(ssize_t) fwrite((const unsigned char *) "",1,1,
        blob_info->file_info.file);
      offset=fseek(blob_info->file_info.file,offset,SEEK_SET);
      if (count != 1)
        return(WizardFalse);
      break;
    }
    case PipeStream:
    case ZipStream:
    {
      return(WizardFalse);
      break;
    }
    case BZipStream:
      return(WizardFalse);
    case BlobStream:
    {
      if (extent != (WizardSizeType) ((size_t) extent))
        return(WizardFalse);
      if (blob_info->mapped != WizardFalse)
        {
          ssize_t
            count;

          WizardOffsetType
            offset;

          (void) UnmapBlob(blob_info->data,blob_info->length);
          RelinquishWizardResource(MapResource,blob_info->length);
          offset=fseek(blob_info->file_info.file,0,SEEK_END);
          if (offset < 0)
            return(WizardFalse);
          if ((WizardSizeType) offset >= extent)
            break;
          offset=fseek(blob_info->file_info.file,(WizardOffsetType) extent-1,
            SEEK_SET);
          count=(ssize_t) fwrite((const unsigned char *) "",1,1,
            blob_info->file_info.file);
          offset=fseek(blob_info->file_info.file,offset,SEEK_SET);
          if (count != 1)
            return(WizardFalse);
          (void) AcquireWizardResource(MapResource,extent);
          blob_info->data=(unsigned char *) MapBlob(fileno(
            blob_info->file_info.file),WriteMode,0,(size_t) extent);
          blob_info->extent=(size_t) extent;
          blob_info->length=(size_t) extent;
          (void) SyncBlob(blob_info);
          break;
        }
      blob_info->extent=(size_t) extent;
      blob_info->data=(unsigned char *) ResizeQuantumMemory(
        blob_info->data,blob_info->extent,sizeof(*blob_info->data));
      (void) SyncBlob(blob_info);
      if (blob_info->data == (unsigned char *) NULL)
        {
          (void) DetachBlob(blob_info);
          return(WizardFalse);
        }
      break;
    }
  }
  return(WizardTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  S y n c B l o b                                                            %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SyncBlob() flushes the datastream if it is a file or synchonizes the data
%  attributes if it is an blob.
%
%  The format of the SyncBlob method is:
%
%      int SyncBlob(BlobInfo *blob_info)
%
%  A description of each parameter follows:
%
%    o blob_info: the blob info.
%
*/
WizardExport int SyncBlob(BlobInfo *blob_info)
{
  int
    status;

  assert(blob_info != (BlobInfo *) NULL);
  assert(blob_info->signature == WizardSignature);
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"%s",blob_info->filename);
  status=0;
  switch (blob_info->type)
  {
    case UndefinedStream:
    case StandardStream:
      break;
    case FileStream:
    case PipeStream:
    {
      status=fflush(blob_info->file_info.file);
      break;
    }
    case ZipStream:
    {
#if defined(WIZARDSTOOLKIT_ZLIB_DELEGATE)
      status=gzflush(blob_info->file_info.gzfile,Z_SYNC_FLUSH);
#endif
      break;
    }
    case BZipStream:
    {
#if defined(WIZARDSTOOLKIT_BZLIB_DELEGATE)
      status=BZ2_bzflush(blob_info->file_info.bzfile);
#endif
      break;
    }
    case BlobStream:
      break;
  }
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+  T e l l B l o b                                                            %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  TellBlob() obtains the current value of the blob or file position.
%
%  The format of the TellBlob method is:
%
%      WizardOffsetType TellBlob(const BlobInfo *blob_info)
%
%  A description of each parameter follows:
%
%    o image: The image.
%
*/
WizardExport WizardOffsetType TellBlob(const BlobInfo *blob_info)
{
  WizardOffsetType
    offset;

  assert(blob_info != (BlobInfo *) NULL);
  assert(blob_info->signature == WizardSignature);
  assert(blob_info->type != UndefinedStream);
  (void) LogWizardEvent(TraceEvent,GetWizardModule(),"%s",blob_info->filename);
  offset=(-1);
  switch (blob_info->type)
  {
    case UndefinedStream:
    case StandardStream:
      break;
    case FileStream:
    {
      offset=ftell(blob_info->file_info.file);
      break;
    }
    case PipeStream:
      break;
    case ZipStream:
    {
#if defined(WIZARDSTOOLKIT_ZLIB_DELEGATE)
      offset=(WizardOffsetType) gztell(blob_info->file_info.gzfile);
#endif
      break;
    }
    case BZipStream:
      break;
    case BlobStream:
    {
      offset=blob_info->offset;
      break;
    }
  }
  return(offset);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+  U n m a p B l o b                                                          %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnmapBlob() deallocates the binary large object previously allocated with
%  the MapBlob method.
%
%  The format of the UnmapBlob method is:
%
%      WizardBooleanType UnmapBlob(void *map,const size_t length)
%
%  A description of each parameter follows:
%
%    o map: The address  of the binary large object.
%
%    o length: The length of the binary large object.
%
*/
WizardExport WizardBooleanType UnmapBlob(void *map,const size_t length)
{
#if defined(WIZARDSTOOLKIT_HAVE_MMAP_FILEIO)
  int
    status;

  status=munmap(map,length);
  return(status == -1 ? WizardFalse : WizardTrue);
#else
  return(WizardFalse);
#endif
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+  W r i t e B l o b                                                          %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WriteBlob() writes data to a blob or blob_info file.  It returns the number
%  of bytes written.
%
%  The format of the WriteBlob method is:
%
%      ssize_t WriteBlob(BlobInfo *blob_info,const size_t length,
%        const unsigned char *data)
%
%  A description of each parameter follows:
%
%    o blob_info: the blob.
%
%    o length: the number of bytes to write to the blob.
%
%    o data: the area to place the information requested from the blob.
%
*/
WizardExport ssize_t WriteBlob(BlobInfo *blob_info,const size_t length,
  const unsigned char *data)
{
  int
    c;

  register const unsigned char
    *p;

  ssize_t
    count;

  assert(blob_info != (BlobInfo *) NULL);
  assert(blob_info->signature == WizardSignature);
  assert(data != (const unsigned char *) NULL);
  if (length == 0)
    return(0);
  count=0;
  p=data;
  switch (blob_info->type)
  {
    case UndefinedStream:
      break;
    case StandardStream:
    {
      count=write(fileno(blob_info->file_info.file),data,length);
      break;
    }
    case FileStream:
    case PipeStream:
    {
      switch (length)
      {
        default:
        {
          count=(ssize_t) fwrite((const char *) data,1,length,
            blob_info->file_info.file);
          break;
        }
        case 2:
        {
          c=putc((int) *p++,blob_info->file_info.file);
          if (c == EOF)
            break;
          count++;
        }
        case 1:
        {
          c=putc((int) *p++,blob_info->file_info.file);
          if (c == EOF)
            break;
          count++;
        }
        case 0:
          break;
      }
      break;
    }
    case ZipStream:
    {
#if defined(WIZARDSTOOLKIT_ZLIB_DELEGATE)
      switch (length)
      {
        default:
        {
          count=(ssize_t) gzwrite(blob_info->file_info.gzfile,(void *) data,
            (unsigned int) length);
          break;
        }
        case 2:
        {
          c=gzputc(blob_info->file_info.gzfile,(int) *p++);
          if (c == EOF)
            break;
          count++;
        }
        case 1:
        {
          c=gzputc(blob_info->file_info.gzfile,(int) *p++);
          if (c == EOF)
            break;
          count++;
        }
        case 0:
          break;
      }
#endif
      break;
    }
    case BZipStream:
    {
#if defined(WIZARDSTOOLKIT_BZLIB_DELEGATE)
      count=(ssize_t) BZ2_bzwrite(blob_info->file_info.bzfile,(void *) data,
        (int) length);
#endif
      break;
    }
    case BlobStream:
    {
      register unsigned char
        *q;

      if ((blob_info->offset+(WizardOffsetType) length) >=
          (WizardOffsetType) blob_info->extent)
        {
          if (blob_info->mapped != WizardFalse)
            return(0);
          blob_info->quantum<<=1;
          blob_info->extent+=length+blob_info->quantum;
          blob_info->data=(unsigned char *) ResizeQuantumMemory(
            blob_info->data,blob_info->extent+1,sizeof(*blob_info->data));
          (void) SyncBlob(blob_info);
          if (blob_info->data == (unsigned char *) NULL)
            {
              (void) DetachBlob(blob_info);
              return(0);
            }
        }
      q=blob_info->data+blob_info->offset;
      (void) memcpy(q,p,length);
      blob_info->offset+=length;
      if (blob_info->offset >= (WizardOffsetType) blob_info->length)
        blob_info->length=(size_t) blob_info->offset;
      count=(ssize_t) length;
    }
  }
  return(count);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+  W r i t e B l o b B y t e                                                  %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WriteBlobByte() write an integer to a blob.  It returns the number of bytes
%  written (either 0 or 1);
%
%  The format of the WriteBlobByte method is:
%
%      ssize_t WriteBlobByte(BlobInfo *blob_info,const unsigned char value)
%
%  A description of each parameter follows.
%
%    o blob_info: the blob info.
%
%    o value: the value to write.
%
*/

static inline ssize_t WriteBlobStream(BlobInfo *blob_info,const size_t length,
  const unsigned char *data)
{
  register unsigned char
    *q;

  WizardSizeType
    extent;

  assert(blob_info != (BlobInfo *) NULL);
  if (blob_info->type != BlobStream)
    return(WriteBlob(blob_info,length,data));
  assert(blob_info->type != UndefinedStream);
  assert(data != (void *) NULL);
  extent=(WizardSizeType) (blob_info->offset+(WizardOffsetType) length);
  if (extent >= blob_info->extent)
    {
      blob_info->quantum<<=1;
      extent=blob_info->extent+blob_info->quantum+length;
      if (SetBlobExtent(blob_info,extent) == WizardFalse)
        return(0);
    }
  q=blob_info->data+blob_info->offset;
  (void) memcpy(q,data,length);
  blob_info->offset+=length;
  if (blob_info->offset >= (WizardOffsetType) blob_info->length)
    blob_info->length=(size_t) blob_info->offset;
  return((ssize_t) length);
}

WizardExport ssize_t WriteBlobByte(BlobInfo *blob_info,
  const unsigned char value)
{
  assert(blob_info != (BlobInfo *) NULL);
  assert(blob_info->signature == WizardSignature);
  return(WriteBlobStream(blob_info,1,&value));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+  W r i t e B l o b C h u n k                                                %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WriteBlobChunk() writes data to a blob or blob_info file.  It returns the
%  number of bytes written.  WriteBlobChunk() differs from WriteBlob() by
%  making an effort to write the number of bytes that was requested except in
%  the event an EOF is encountered.
%
%  The format of the WriteBlob method is:
%
%      ssize_t WriteBlobChunk(BlobInfo *blob_info,const size_t length,
%        const unsigned char *data)
%
%  A description of each parameter follows:
%
%    o blob_info: the blob.
%
%    o length: the number of bytes to write to the blob.
%
%    o data: the area to place the information requested from the blob.
%
*/
WizardExport ssize_t WriteBlobChunk(BlobInfo *blob_info,const size_t length,
  const unsigned char *data)
{
  register ssize_t
    i;

  ssize_t
    count;

  assert(blob_info != (BlobInfo *) NULL);
  assert(blob_info->signature == WizardSignature);
  assert(data != (const unsigned char *) NULL);
  if (blob_info->type == BlobStream)
    return(WriteBlob(blob_info,length,data));
  count=0;
  for (i=0; i < (ssize_t) length; i+=count)
  {
    count=WriteBlob(blob_info,(size_t) WizardMin(length-i,(WizardSizeType)
      SSIZE_MAX),data+i);
    if (count <= 0)
      {
        count=0;
        if (errno != EINTR)
          break;
      }
  }
  return(i);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+  W r i t e B l o b S t r i n g                                              %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WriteBlobString() write a string to a blob.  It returns the number of
%  characters written.
%
%  The format of the WriteBlobString method is:
%
%      ssize_t WriteBlobString(BlobInfo *blob_info,const char *string)
%
%  A description of each parameter follows.
%
%    o image: The image.
%
%    o string: Specifies the string to write.
%
*/
WizardExport ssize_t WriteBlobString(BlobInfo *blob_info,const char *string)
{
  ssize_t
    count;

  assert(blob_info != (BlobInfo *) NULL);
  assert(blob_info->signature == WizardSignature);
  assert(string != (const char *) NULL);
  count=WriteBlobStream(blob_info,strlen(string),(const unsigned char *)
    string);
  return(count);
}
