/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                    M   M  EEEEE  M   M   OOO   RRRR   Y   Y                 %
%                    MM MM  E      MM MM  O   O  R   R   Y Y                  %
%                    M M M  EEE    M M M  O   O  RRRR     Y                   %
%                    M   M  E      M   M  O   O  R R      Y                   %
%                    M   M  EEEEE  M   M   OOO   R  R     Y                   %
%                                                                             %
%                                                                             %
%                 The Wizard's Toolkit Memory Allocation Methods              %
%                                                                             %
%                              Software Design                                %
%                                  Cristy                                     %
%                                 July 1998                                   %
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
%  We provide these memory allocators:
%
%    AcquireCriticalMemory(): allocate a small memory request with
%      AcquireMagickMemory(), however, on fail throw a fatal exception and exit.
%      Free the memory reserve with RelinquishMagickMemory().
%    AcquireAlignedMemory(): allocate a small memory request that is aligned
%      on a cache line.  On fail, return NULL for possible recovery.
%      Free the memory reserve with RelinquishMagickMemory().
%    AcquireMagickMemory()/ResizeMagickMemory(): allocate a small to medium
%      memory request, typically with malloc()/realloc(). On fail, return NULL
%      for possible recovery.  Free the memory reserve with
%      RelinquishMagickMemory().
%    AcquireQuantumMemory()/ResizeQuantumMemory(): allocate a small to medium
%      memory request.  This is a secure memory allocator as it accepts two
%      parameters, count and quantum, to ensure the request does not overflow.
%      It also check to ensure the request does not exceed the maximum memory
%      per the security policy.  Free the memory reserve with
%      RelinquishMagickMemory().
%    AcquireVirtualMemory(): allocate a large memory request either in heap,
%      memory-mapped, or memory-mapped on disk depending on whether heap
%      allocation fails or if the request exceeds the maximum memory policy.
%      Free the memory reserve with RelinquishVirtualMemory().
%    ResetMagickMemory(): fills the bytes of the memory area with a constant
%      byte.
%    
%  In addition, we provide hooks for your own memory constructor/destructors.
%  You can also utilize our internal custom allocator as follows: Segregate
%  our memory requirements from any program that calls our API.  This should
%  help reduce the risk of others changing our program state or causing memory
%  corruption.
%
%  Our custom memory allocation manager implements a best-fit allocation policy
%  using segregated free lists.  It uses a linear distribution of size classes
%  for lower sizes and a power of two distribution of size classes at higher
%  sizes.  It is based on the paper, "Fast Memory Allocation using Lazy Fits."
%  written by Yoo C. Chung.
%
%  By default, ANSI memory methods are called (e.g. malloc).  Use the
%  custom memory allocator by defining WIZARDSTOOLKIT_EMBEDDABLE_SUPPORT
%  to allocate memory with private anonymous mapping rather than from the
%  heap.
%
*/

/*
  Include declarations.
*/
#include "wizard/studio.h"
#include "wizard/blob.h"
#include "wizard/blob-private.h"
#include "wizard/exception.h"
#include "wizard/exception-private.h"
#include "wizard/memory_.h"
#include "wizard/memory-private.h"
#include "wizard/resource_.h"
#include "wizard/semaphore.h"
#include "wizard/string_.h"
#include "wizard/utility-private.h"

/*
  Define declarations.
*/
#define BlockFooter(block,size) \
  ((size_t *) ((char *) (block)+(size)-2*sizeof(size_t)))
#define BlockHeader(block)  ((size_t *) (block)-1)
#define BlockSize  4096
#define BlockThreshold  1024
#define MaxBlockExponent  16
#define MaxBlocks ((BlockThreshold/(4*sizeof(size_t)))+MaxBlockExponent+1)
#define MaxSegments  1024
#define MemoryGuard  ((0xdeadbeef << 31)+0xdeafdeed)
#define NextBlock(block)  ((char *) (block)+SizeOfBlock(block))
#define NextBlockInList(block)  (*(void **) (block))
#define PreviousBlock(block)  ((char *) (block)-(*((size_t *) (block)-2)))
#define PreviousBlockBit  0x01
#define PreviousBlockInList(block)  (*((void **) (block)+1))
#define SegmentSize  (2*1024*1024)
#define SizeMask  (~0x01)
#define SizeOfBlock(block)  (*BlockHeader(block) & SizeMask)

/*
  Typedef declarations.
*/
typedef enum
{
  UndefinedVirtualMemory,
  AlignedVirtualMemory,
  MapVirtualMemory,
  UnalignedVirtualMemory
} VirtualMemoryType;

typedef struct _DataSegmentInfo
{
  void
    *allocation,
    *bound;

  WizardBooleanType
    mapped;

  size_t
    length;

  struct _DataSegmentInfo
    *previous,
    *next;
} DataSegmentInfo;

typedef struct _WizardMemoryMethods
{
  AcquireMemoryHandler
    acquire_memory_handler;

  ResizeMemoryHandler
    resize_memory_handler;

  DestroyMemoryHandler
    destroy_memory_handler;
} WizardMemoryMethods;

struct _MemoryInfo
{
  char
    filename[WizardPathExtent];

  VirtualMemoryType
    type;

  size_t
    length;

  void
    *blob;

  size_t
    signature;
};

typedef struct _MemoryPool
{
  size_t
    allocation;

  void
    *blocks[MaxBlocks+1];

  size_t
    number_segments;

  DataSegmentInfo
    *segments[MaxSegments],
    segment_pool[MaxSegments];
} MemoryPool;

/*
  Global declarations.
*/
static WizardMemoryMethods
  memory_methods =
  {
    (AcquireMemoryHandler) malloc,
    (ResizeMemoryHandler) realloc,
    (DestroyMemoryHandler) free
  };

#if defined(WIZARDSTOOLKIT_EMBEDDABLE_SUPPORT)
static MemoryPool
  memory_pool;

static SemaphoreInfo
  *memory_semaphore = (SemaphoreInfo *) NULL;

static volatile DataSegmentInfo
  *free_segments = (DataSegmentInfo *) NULL;

/*
  Forward declarations.
*/
static WizardBooleanType
  ExpandHeap(size_t);
#endif

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   A c q u i r e A l i g n e d M e m o r y                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquireAlignedMemory() returns a pointer to a block of memory at least size
%  bytes whose address is aligned on a cache line or page boundary.
%
%  The format of the AcquireAlignedMemory method is:
%
%      void *AcquireAlignedMemory(const size_t count,const size_t quantum)
%
%  A description of each parameter follows:
%
%    o count: the number of quantum elements to allocate.
%
%    o quantum: the number of bytes in each quantum.
%
*/
WizardExport void *AcquireAlignedMemory(const size_t count,const size_t quantum)
{
#define AlignedExtent(size,alignment) \
  (((size)+((alignment)-1)) & ~((alignment)-1))

  size_t
    alignment,
    extent,
    size;

  void
    *memory;

  size=count*quantum;
  if ((count == 0) || (quantum != (size/count)))
    {
      errno=ENOMEM;
      return((void *) NULL);
    }
  memory=NULL;
  alignment=CACHE_LINE_SIZE;
  if (size > (size_t) (GetWizardPageSize() >> 1))
    alignment=(size_t) GetWizardPageSize();
  extent=AlignedExtent(size,CACHE_LINE_SIZE);
  if ((size == 0) || (extent < size))
    return((void *) NULL);
#if defined(WIZARDSTOOLKIT_HAVE_POSIX_MEMALIGN)
  if (posix_memalign(&memory,alignment,extent) != 0)
    memory=NULL;
#elif defined(WIZARDSTOOLKIT_HAVE__ALIGNED_MALLOC)
  memory=_aligned_malloc(extent,alignment);
#else
  {
    void
      *p;

    extent=(size+alignment-1)+sizeof(void *);
    if (extent > size)
      {
        p=AcquireWizardMemory(extent);
        if (p != NULL)
          {
            memory=(void *) AlignedExtent((size_t) p+sizeof(void *),alignment);
            *((void **) memory-1)=p;
          }
      }
  }
#endif
  return(memory);
}

#if defined(WIZARDSTOOLKIT_EMBEDDABLE_SUPPORT)
/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   A c q u i r e B l o c k                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquireBlock() returns a pointer to a block of memory at least size bytes
%  suitably aligned for any use.
%
%  The format of the AcquireBlock method is:
%
%      void *AcquireBlock(const size_t size)
%
%  A description of each parameter follows:
%
%    o size: the size of the memory in bytes to allocate.
%
*/

static inline size_t AllocationPolicy(size_t size)
{
   size_t
    blocksize;

  /*
    The linear distribution.
  */
  assert(size != 0);
  assert(size % (4*sizeof(size_t)) == 0);
  if (size <= BlockThreshold)
    return(size/(4*sizeof(size_t)));
  /*
    Check for the largest block size.
  */
  if (size > (size_t) (BlockThreshold*(1L << (MaxBlockExponent-1L))))
    return(MaxBlocks-1L);
  /*
    Otherwise use a power of two distribution.
  */
  blocksize=BlockThreshold/(4*sizeof(size_t));
  for ( ; size > BlockThreshold; size/=2)
    blocksize++;
  assert(blocksize > (BlockThreshold/(4*sizeof(size_t))));
  assert(blocksize < (MaxBlocks-1L));
  return(blocksize);
}

static inline void InsertFreeBlock(void *block,const size_t i)
{
   void
    *next,
    *previous;

  size_t
    size;

  size=SizeOfBlock(block);
  previous=(void *) NULL;
  next=memory_pool.blocks[i];
  while ((next != (void *) NULL) && (SizeOfBlock(next) < size))
  {
    previous=next;
    next=NextBlockInList(next);
  }
  PreviousBlockInList(block)=previous;
  NextBlockInList(block)=next;
  if (previous != (void *) NULL)
    NextBlockInList(previous)=block;
  else
    memory_pool.blocks[i]=block;
  if (next != (void *) NULL)
    PreviousBlockInList(next)=block;
}

static inline void RemoveFreeBlock(void *block,const size_t i)
{
   void
    *next,
    *previous;

  next=NextBlockInList(block);
  previous=PreviousBlockInList(block);
  if (previous == (void *) NULL)
    memory_pool.blocks[i]=next;
  else
    NextBlockInList(previous)=next;
  if (next != (void *) NULL)
    PreviousBlockInList(next)=previous;
}

static void *AcquireBlock(size_t size)
{
   size_t
    i;

   void
    *block;

  /*
    Find free block.
  */
  size=(size_t) (size+sizeof(size_t)+6*sizeof(size_t)-1) & -(4U*sizeof(size_t));
  i=AllocationPolicy(size);
  block=memory_pool.blocks[i];
  while ((block != (void *) NULL) && (SizeOfBlock(block) < size))
    block=NextBlockInList(block);
  if (block == (void *) NULL)
    {
      i++;
      while (memory_pool.blocks[i] == (void *) NULL)
        i++;
      block=memory_pool.blocks[i];
      if (i >= MaxBlocks)
        return((void *) NULL);
    }
  assert((*BlockHeader(NextBlock(block)) & PreviousBlockBit) == 0);
  assert(SizeOfBlock(block) >= size);
  RemoveFreeBlock(block,AllocationPolicy(SizeOfBlock(block)));
  if (SizeOfBlock(block) > size)
    {
      size_t
        blocksize;

      void
        *next;

      /*
        Split block.
      */
      next=(char *) block+size;
      blocksize=SizeOfBlock(block)-size;
      *BlockHeader(next)=blocksize;
      *BlockFooter(next,blocksize)=blocksize;
      InsertFreeBlock(next,AllocationPolicy(blocksize));
      *BlockHeader(block)=size | (*BlockHeader(block) & ~SizeMask);
    }
  assert(size == SizeOfBlock(block));
  *BlockHeader(NextBlock(block))|=PreviousBlockBit;
  memory_pool.allocation+=size;
  return(block);
}
#endif

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   A c q u i r e V i r t u a l M e m o r y                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquireVirtualMemory() allocates a pointer to a block of memory at least size
%  bytes suitably aligned for any use.
%
%  The format of the AcquireVirtualMemory method is:
%
%      MemoryInfo *AcquireVirtualMemory(const size_t count,const size_t quantum,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o count: the number of quantum elements to allocate.
%
%    o quantum: the number of bytes in each quantum.
%
*/
WizardExport MemoryInfo *AcquireVirtualMemory(const size_t count,
  const size_t quantum,ExceptionInfo *exception)
{
  MemoryInfo
    *memory_info;

  size_t
    length;

  length=count*quantum;
  if ((count == 0) || (quantum != (length/count)))
    {
      errno=ENOMEM;
      return((MemoryInfo *) NULL);
    }
  memory_info=(MemoryInfo *) WizardAssumeAligned(AcquireAlignedMemory(1,
    sizeof(*memory_info)));
  if (memory_info == (MemoryInfo *) NULL)
    ThrowFatalException(ResourceFatalError,"memory allocation failed `%s'");
  (void) ResetWizardMemory(memory_info,0,sizeof(*memory_info));
  memory_info->length=length;
  memory_info->signature=WizardSignature;
  if (AcquireWizardResource(MemoryResource,length) != WizardFalse)
    {
      memory_info->blob=AcquireAlignedMemory(1,length);
      if (memory_info->blob != NULL)
        {
          memory_info->type=AlignedVirtualMemory;
          return(memory_info);
        }
    }
  RelinquishWizardResource(MemoryResource,length);
  if (AcquireWizardResource(MapResource,length) != WizardFalse)
    {
      /*
        Heap memory failed, try anonymous memory mapping.
      */
      memory_info->blob=MapBlob(-1,IOMode,0,length);
      if (memory_info->blob != NULL)
        {
          memory_info->type=MapVirtualMemory;
          return(memory_info);
        }
      if (AcquireWizardResource(DiskResource,length) != WizardFalse)
        {
          int
            file;

          /*
            Anonymous memory mapping failed, try file-backed memory mapping.
            If the MapResource request failed, there is no point in trying
            file-backed memory mapping.
          */
          file=AcquireUniqueFileResource("",memory_info->filename,exception);
          if (file != -1)
            {
              WizardOffsetType
                offset;

              offset=(WizardOffsetType) lseek(file,length-1,SEEK_SET);
              if ((offset == (WizardOffsetType) (length-1)) &&
                  (write(file,"",1) == 1))
                {
                  memory_info->blob=MapBlob(file,IOMode,0,length);
                  if (memory_info->blob != NULL)
                    {
                      (void) close(file);
                      memory_info->type=MapVirtualMemory;
                      return(memory_info);
                    }
                }
              /*
                File-backed memory mapping failed, delete the temporary file.
              */
              (void) close(file);
              (void) RelinquishUniqueFileResource(memory_info->filename,
                WizardTrue);
              *memory_info->filename = '\0';
            }
        }
      RelinquishWizardResource(DiskResource,length);
    }
  RelinquishWizardResource(MapResource,length);
  if (memory_info->blob == NULL)
    {
      memory_info->blob=AcquireWizardMemory(length);
      if (memory_info->blob != NULL)
        memory_info->type=UnalignedVirtualMemory;
    }
  if (memory_info->blob == NULL)
    memory_info=RelinquishVirtualMemory(memory_info);
  return(memory_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   A c q u i r e W i z a r d M e m o r y                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquireWizardMemory() returns a pointer to a block of memory at least size
%  bytes suitably aligned for any use.
%
%  The format of the AcquireWizardMemory method is:
%
%      void *AcquireWizardMemory(const size_t size)
%
%  A description of each parameter follows:
%
%    o size: the size of the memory in bytes to allocate.
%
*/
WizardExport void *AcquireWizardMemory(const size_t size)
{
   void
    *memory;

#if !defined(WIZARDSTOOLKIT_EMBEDDABLE_SUPPORT)
  memory=memory_methods.acquire_memory_handler(size == 0 ? 1UL : size);
#else
  if (memory_semaphore == (SemaphoreInfo *) NULL)
    ActivateSemaphoreInfo(&memory_semaphore);
  if (free_segments == (DataSegmentInfo *) NULL)
    {
      LockSemaphoreInfo(memory_semaphore);
      if (free_segments == (DataSegmentInfo *) NULL)
        {
           ssize_t
            i;

          assert(2*sizeof(size_t) > (size_t) (~SizeMask));
          (void) ResetWizardMemory(&memory_pool,0,sizeof(memory_pool));
          memory_pool.allocation=SegmentSize;
          memory_pool.blocks[MaxBlocks]=(void *) (-1);
          for (i=0; i < MaxSegments; i++)
          {
            if (i != 0)
              memory_pool.segment_pool[i].previous=
                (&memory_pool.segment_pool[i-1]);
            if (i != (MaxSegments-1))
              memory_pool.segment_pool[i].next=(&memory_pool.segment_pool[i+1]);
          }
          free_segments=(&memory_pool.segment_pool[0]);
        }
      UnlockSemaphoreInfo(memory_semaphore);
    }
  LockSemaphoreInfo(memory_semaphore);
  memory=AcquireBlock(size == 0 ? 1UL : size);
  if (memory == (void *) NULL)
    {
      if (ExpandHeap(size == 0 ? 1UL : size) != WizardFalse)
        memory=AcquireBlock(size == 0 ? 1UL : size);
    }
  UnlockSemaphoreInfo(memory_semaphore);
#endif
  return(memory);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   A c q u i r e Q u a n t u m M e m o r y                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquireQuantumMemory() returns a pointer to a block of memory at least
%  count * quantum bytes suitably aligned for any use.
%
%  The format of the AcquireQuantumMemory method is:
%
%      void *AcquireQuantumMemory(const size_t count,const size_t quantum)
%
%  A description of each parameter follows:
%
%    o count: the number of quantum elements to allocate.
%
%    o quantum: the number of bytes in each quantum.
%
*/
WizardExport void *AcquireQuantumMemory(const size_t count,const size_t quantum)
{
  size_t
    size;

  size=count*quantum;
  if ((count == 0) || (quantum != (size/count)))
    {
      errno=ENOMEM;
      return((void *) NULL);
    }
  return(AcquireWizardMemory(size));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   C o p y W i z a r d M e m o r y                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  CopyWizardMemory() copies size bytes from memory area source to the
%  destination.  Copying between objects that overlap will take place
%  correctly.  It returns destination.
%
%  The format of the CopyWizardMemory method is:
%
%      void *CopyWizardMemory(void *destination,const void *source,
%        const size_t size)
%
%  A description of each parameter follows:
%
%    o destination: the destination.
%
%    o source: the source.
%
%    o size: the size of the memory in bytes to allocate.
%
*/
WizardExport void *CopyWizardMemory(void *destination,const void *source,
  const size_t size)
{
   const unsigned char
    *p;

   unsigned char
    *q;

  assert(destination != (void *) NULL);
  assert(source != (const void *) NULL);
  p=(const unsigned char *) source;
  q=(unsigned char *) destination;
  if (((q+size) < p) || (q > (p+size)))
    switch (size)
    {
      default: return(memcpy(destination,source,size));
      case 8: *q++=(*p++);
      case 7: *q++=(*p++);
      case 6: *q++=(*p++);
      case 5: *q++=(*p++);
      case 4: *q++=(*p++);
      case 3: *q++=(*p++);
      case 2: *q++=(*p++);
      case 1: *q++=(*p++);
      case 0: return(destination);
    }
  return(memmove(destination,source,size));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   D e s t r o y W i z a r d M e m o r y                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DestroyWizardMemory() deallocates memory associated with the memory manager.
%
%  The format of the DestroyWizardMemory method is:
%
%      DestroyWizardMemory(void)
%
*/
WizardExport void DestroyWizardMemory(void)
{
#if defined(WIZARDSTOOLKIT_EMBEDDABLE_SUPPORT)
   ssize_t
    i;

  if (memory_semaphore == (SemaphoreInfo *) NULL)
    ActivateSemaphoreInfo(&memory_semaphore);
  LockSemaphoreInfo(memory_semaphore);
  for (i=0; i < (ssize_t) memory_pool.number_segments; i++)
    if (memory_pool.segments[i]->mapped == WizardFalse)
      memory_methods.destroy_memory_handler(
        memory_pool.segments[i]->allocation);
    else
      (void) UnmapBlob(memory_pool.segments[i]->allocation,
        memory_pool.segments[i]->length);
  free_segments=(DataSegmentInfo *) NULL;
  (void) ResetWizardMemory(&memory_pool,0,sizeof(memory_pool));
  UnlockSemaphoreInfo(memory_semaphore);
  RelinquishSemaphoreInfo(&memory_semaphore);
#endif
}

#if defined(WIZARDSTOOLKIT_EMBEDDABLE_SUPPORT)
/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   E x p a n d H e a p                                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ExpandHeap() get more memory from the system.  It returns WizardTrue on
%  success otherwise WizardFalse.
%
%  The format of the ExpandHeap method is:
%
%      WizardBooleanType ExpandHeap(size_t size)
%
%  A description of each parameter follows:
%
%    o size: the size of the memory in bytes we require.
%
*/
static WizardBooleanType ExpandHeap(size_t size)
{
  DataSegmentInfo
    *segment_info;

  WizardBooleanType
    mapped;

   ssize_t
    i;

   void
    *block;

  size_t
    blocksize;

  void
    *segment;

  blocksize=((size+12*sizeof(size_t))+SegmentSize-1) & -SegmentSize;
  assert(memory_pool.number_segments < MaxSegments);
  segment=MapBlob(-1,IOMode,0,blocksize);
  mapped=segment != (void *) NULL ? WizardTrue : WizardFalse;
  if (segment == (void *) NULL)
    segment=(void *) memory_methods.acquire_memory_handler(blocksize);
  if (segment == (void *) NULL)
    return(WizardFalse);
  segment_info=(DataSegmentInfo *) free_segments;
  free_segments=segment_info->next;
  segment_info->mapped=mapped;
  segment_info->length=blocksize;
  segment_info->allocation=segment;
  segment_info->bound=(char *) segment+blocksize;
  i=(ssize_t) memory_pool.number_segments-1;
  for ( ; (i >= 0) && (memory_pool.segments[i]->allocation > segment); i--)
    memory_pool.segments[i+1]=memory_pool.segments[i];
  memory_pool.segments[i+1]=segment_info;
  memory_pool.number_segments++;
  size=blocksize-12*sizeof(size_t);
  block=(char *) segment_info->allocation+4*sizeof(size_t);
  *BlockHeader(block)=size | PreviousBlockBit;
  *BlockFooter(block,size)=size;
  InsertFreeBlock(block,AllocationPolicy(size));
  block=NextBlock(block);
  assert(block < segment_info->bound);
  *BlockHeader(block)=2*sizeof(size_t);
  *BlockHeader(NextBlock(block))=PreviousBlockBit;
  return(WizardTrue);
}
#endif

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t W i z a r d M e m o r y M e t h o d s                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetWizardMemoryMethods() gets the methods to acquire, resize, and destroy
%  memory.
%
%  The format of the GetWizardMemoryMethods() method is:
%
%      void GetWizardMemoryMethods(AcquireMemoryHandler *acquire_memory_handler,
%        ResizeMemoryHandler *resize_memory_handler,
%        DestroyMemoryHandler *destroy_memory_handler)
%
%  A description of each parameter follows:
%
%    o acquire_memory_handler: method to acquire memory (e.g. malloc).
%
%    o resize_memory_handler: method to resize memory (e.g. realloc).
%
%    o destroy_memory_handler: method to destroy memory (e.g. free).
%
*/
WizardExport void GetWizardMemoryMethods(
  AcquireMemoryHandler *acquire_memory_handler,
  ResizeMemoryHandler *resize_memory_handler,
  DestroyMemoryHandler *destroy_memory_handler)
{
  assert(acquire_memory_handler != (AcquireMemoryHandler *) NULL);
  assert(resize_memory_handler != (ResizeMemoryHandler *) NULL);
  assert(destroy_memory_handler != (DestroyMemoryHandler *) NULL);
  *acquire_memory_handler=memory_methods.acquire_memory_handler;
  *resize_memory_handler=memory_methods.resize_memory_handler;
  *destroy_memory_handler=memory_methods.destroy_memory_handler;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t V i r t u a l M e m o r y B l o b                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetVirtualMemoryBlob() returns the virtual memory blob associated with the
%  specified MemoryInfo structure.
%
%  The format of the GetVirtualMemoryBlob method is:
%
%      void *GetVirtualMemoryBlob(const MemoryInfo *memory_info)
%
%  A description of each parameter follows:
%
%    o memory_info: The MemoryInfo structure.
*/
WizardExport void *GetVirtualMemoryBlob(const MemoryInfo *memory_info)
{
  assert(memory_info != (MemoryInfo *) NULL);
  assert(memory_info->signature == WizardSignature);
  return(memory_info->blob);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e l i n q u i s h A l i g n e d M e m o r y                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RelinquishAlignedMemory() frees memory acquired with AcquireAlignedMemory()
%  or reuse.
%
%  The format of the RelinquishAlignedMemory method is:
%
%      void *RelinquishAlignedMemory(void *memory)
%
%  A description of each parameter follows:
%
%    o memory: A pointer to a block of memory to free for reuse.
%
*/
WizardExport void *RelinquishAlignedMemory(void *memory)
{
  if (memory == (void *) NULL)
    return((void *) NULL);
#if defined(WIZARDSTOOLKIT_HAVE_POSIX_MEMALIGN)
  free(memory);
#elif defined(WIZARDSTOOLKIT_HAVE__ALIGNED_MALLOC)
  _aligned_free(memory);
#else
  RelinquishWizardMemory(*((void **) memory-1));
#endif
  return(NULL);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e l i n q u i s h V i r t u a l M e m o r y                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RelinquishVirtualMemory() destroys the MemoryInfo structure returned by a
%  previous call to AcquireVirtualMemory().
%
%  The format of the RelinquishVirtualMemory method is:
%
%      void *RelinquishVirtualMemory(MemoryInfo *memory_info)
%
%  A description of each parameter follows:
%
%    o memory_info: A pointer to a block of memory to free for reuse.
%
*/
WizardExport MemoryInfo *RelinquishVirtualMemory(MemoryInfo *memory_info)
{
  assert(memory_info != (MemoryInfo *) NULL);
  assert(memory_info->signature == WizardSignature);
  if (memory_info->blob != (void *) NULL)
    switch (memory_info->type)
    {
      case AlignedVirtualMemory:
      {
        memory_info->blob=RelinquishAlignedMemory(memory_info->blob);
        RelinquishWizardResource(MemoryResource,memory_info->length);
        break;
      }
      case MapVirtualMemory:
      {
        (void) UnmapBlob(memory_info->blob,memory_info->length);
        memory_info->blob=NULL;
        RelinquishWizardResource(MapResource,memory_info->length);
        if (*memory_info->filename != '\0')
          {
            (void) RelinquishUniqueFileResource(memory_info->filename,
              WizardTrue);
            RelinquishWizardResource(DiskResource,memory_info->length);
          }
        break;
      }
      case UnalignedVirtualMemory:
      default:
      {
        memory_info->blob=RelinquishWizardMemory(memory_info->blob);
        break;
      }
    }
  memory_info->signature=(~WizardSignature);
  memory_info=(MemoryInfo *) RelinquishAlignedMemory(memory_info);
  return(memory_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e l i n q u i s h W i z a r d M e m o r y                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RelinquishWizardMemory() zeros memory that has been allocated, frees it for
%  reuse.
%
%  The format of the RelinquishWizardMemory method is:
%
%      void *RelinquishWizardMemory(void *memory)
%
%  A description of each parameter follows:
%
%    o memory: A pointer to a block of memory to free for reuse.
%
*/
WizardExport void *RelinquishWizardMemory(void *memory)
{
  if (memory == (void *) NULL)
    return((void *) NULL);
#if !defined(WIZARDSTOOLKIT_EMBEDDABLE_SUPPORT)
  memory_methods.destroy_memory_handler(memory);
#else
  assert((SizeOfBlock(memory) % (4*sizeof(size_t))) == 0);
  assert((*BlockHeader(NextBlock(memory)) & PreviousBlockBit) != 0);
  LockSemaphoreInfo(memory_semaphore);
  if ((*BlockHeader(memory) & PreviousBlockBit) == 0)
    {
      void
        *previous;

      /*
        Coalesce with previous adjacent block.
      */
      previous=PreviousBlock(memory);
      RemoveFreeBlock(previous,AllocationPolicy(SizeOfBlock(previous)));
      *BlockHeader(previous)=(SizeOfBlock(previous)+SizeOfBlock(memory)) |
        (*BlockHeader(previous) & ~SizeMask);
      memory=previous;
    }
  if ((*BlockHeader(NextBlock(NextBlock(memory))) & PreviousBlockBit) == 0)
    {
      void
        *next;

      /*
        Coalesce with next adjacent block.
      */
      next=NextBlock(memory);
      RemoveFreeBlock(next,AllocationPolicy(SizeOfBlock(next)));
      *BlockHeader(memory)=(SizeOfBlock(memory)+SizeOfBlock(next)) |
        (*BlockHeader(memory) & ~SizeMask);
    }
  *BlockFooter(memory,SizeOfBlock(memory))=SizeOfBlock(memory);
  *BlockHeader(NextBlock(memory))&=(~PreviousBlockBit);
  InsertFreeBlock(memory,AllocationPolicy(SizeOfBlock(memory)));
  UnlockSemaphoreInfo(memory_semaphore);
#endif
  return((void *) NULL);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e s e t W i z a r d M e m o r y                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ResetWizardMemory() fills the first size bytes of the memory area pointed to
%  by memory with the constant byte c.  We use a volatile pointer when
%  updating the byte string.  Most compilers will avoid optimizing away access
%  to a volatile pointer, even if the pointer appears to be unused after the
%  call.
%
%  The format of the ResetWizardMemory method is:
%
%      void *ResetWizardMemory(void *memory,int c,const size_t size)
%
%  A description of each parameter follows:
%
%    o memory: A pointer to a memory allocation.
%
%    o c: Set the memory to this value.
%
%    o size: Size of the memory to reset.
%
*/
WizardExport void *ResetWizardMemory(void *memory,int c,const size_t size)
{
  volatile unsigned char
    *p = (volatile unsigned char *) memory;

  size_t
    n = size;

  assert(memory != (void *) NULL);
  while (n-- != 0)
    *p++=(unsigned char) c;
  return(memory);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e s i z e W i z a r d M e m o r y                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ResizeWizardMemory() changes the size of the memory and returns a pointer to
%  the (possibly moved) block.  The contents will be unchanged up to the
%  lesser of the new and old sizes.
%
%  The format of the ResizeWizardMemory method is:
%
%      void *ResizeWizardMemory(void *memory,const size_t size)
%
%  A description of each parameter follows:
%
%    o memory: A pointer to a memory allocation.
%
%    o size: the new size of the allocated memory.
%
*/

#if defined(WIZARDSTOOLKIT_EMBEDDABLE_SUPPORT)
static inline void *ResizeBlock(void *block,size_t size)
{
   void
    *memory;

  if (block == (void *) NULL)
    return(AcquireBlock(size));
  memory=AcquireBlock(size);
  if (memory == (void *) NULL)
    return((void *) NULL);
  if (size <= (SizeOfBlock(block)-sizeof(size_t)))
    (void) memcpy(memory,block,size);
  else
    (void) memcpy(memory,block,SizeOfBlock(block)-sizeof(size_t));
  memory_pool.allocation+=size;
  return(memory);
}
#endif

WizardExport void *ResizeWizardMemory(void *memory,const size_t size)
{
   void
    *block;

  if (memory == (void *) NULL)
    return(AcquireWizardMemory(size));
#if !defined(WIZARDSTOOLKIT_EMBEDDABLE_SUPPORT)
  block=memory_methods.resize_memory_handler(memory,size == 0 ? 1UL : size);
  if (block == (void *) NULL)
    memory=RelinquishWizardMemory(memory);
#else
  LockSemaphoreInfo(memory_semaphore);
  block=ResizeBlock(memory,size == 0 ? 1UL : size);
  if (block == (void *) NULL)
    {
      if (ExpandHeap(size == 0 ? 1UL : size) == WizardFalse)
        {
          UnlockSemaphoreInfo(memory_semaphore);
          memory=RelinquishWizardMemory(memory);
          ThrowFatalException(ResourceFatalError,
            "memory allocation failed `%s'");
        }
      block=ResizeBlock(memory,size == 0 ? 1UL : size);
      assert(block != (void *) NULL);
    }
  UnlockSemaphoreInfo(memory_semaphore);
  memory=RelinquishWizardMemory(memory);
#endif
  return(block);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e s i z e Q u a n t u m M e m o r y                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ResizeQuantumMemory() changes the size of the memory and returns a pointer
%  to the (possibly moved) block.  The contents will be unchanged up to the
%  lesser of the new and old sizes.
%
%  The format of the ResizeQuantumMemory method is:
%
%      void *ResizeQuantumMemory(void *memory,const size_t count,
%        const size_t quantum)
%
%  A description of each parameter follows:
%
%    o memory: A pointer to a memory allocation.
%
%    o count: the number of quantum elements to allocate.
%
%    o quantum: the number of bytes in each quantum.
%
*/
WizardExport void *ResizeQuantumMemory(void *memory,const size_t count,
  const size_t quantum)
{
  size_t
    size;

  size=count*quantum;
  if ((count == 0) || (quantum != (size/count)))
    {
      memory=RelinquishWizardMemory(memory);
      errno=ENOMEM;
      return((void *) NULL);
    }
  return(ResizeWizardMemory(memory,size));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t W i z a r d M e m o r y M e t h o d s                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetWizardMemoryMethods() sets the methods to acquire, resize, and destroy
%  memory.
%
%  The format of the SetWizardMemoryMethods() method is:
%
%      SetWizardMemoryMethods(AcquireMemoryHandler acquire_memory_handler,
%        ResizeMemoryHandler resize_memory_handler,
%        DestroyMemoryHandler destroy_memory_handler)
%
%  A description of each parameter follows:
%
%    o acquire_memory_handler: method to acquire memory (e.g. malloc).
%
%    o resize_memory_handler: method to resize memory (e.g. realloc).
%
%    o destroy_memory_handler: method to destroy memory (e.g. free).
%
*/
WizardExport void SetWizardMemoryMethods(
  AcquireMemoryHandler acquire_memory_handler,
  ResizeMemoryHandler resize_memory_handler,
  DestroyMemoryHandler destroy_memory_handler)
{
  /*
    Set memory methods.
  */
  if (acquire_memory_handler != (AcquireMemoryHandler) NULL)
    memory_methods.acquire_memory_handler=acquire_memory_handler;
  if (resize_memory_handler != (ResizeMemoryHandler) NULL)
    memory_methods.resize_memory_handler=resize_memory_handler;
  if (destroy_memory_handler != (DestroyMemoryHandler) NULL)
    memory_methods.destroy_memory_handler=destroy_memory_handler;
}
