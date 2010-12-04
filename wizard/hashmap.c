/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                H   H   AAA   SSSSS  H   H  M   M   AAA   PPPP               %
%                H   H  A   A  SS     H   H  MM MM  A   A  P   P              %
%                HHHHH  AAAAA   SSS   HHHHH  M M M  AAAAA  PPPP               %
%                H   H  A   A     SS  H   H  M   M  A   A  P                  %
%                H   H  A   A  SSSSS  H   H  M   M  A   A  P                  %
%                                                                             %
%                                                                             %
%                Wizard's Toolkit Hash-map and Linked-list Methods            %
%                                                                             %
%                              Software Design                                %
%                                John Cristy                                  %
%                               December 2002                                 %
%                                                                             %
%                                                                             %
%  Copyright 1999-2011 ImageMagick Studio LLC, a non-profit organization      %
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
%  This module implements the standard handy hash and linked-list methods for
%  storing and retrieving large numbers of data elements.  It is loosely based
%  on the Java implementation of these algorithms.
%
*/

/*
  Increde declarations.
*/
#include "wizard/studio.h"
#include "wizard/exception.h"
#include "wizard/exception-private.h"
#include "wizard/hash.h"
#include "wizard/hashmap.h"
#include "wizard/memory_.h"
#include "wizard/semaphore.h"
#include "wizard/string_.h"

/*
  Typedef declarations.
*/
typedef struct _ElementInfo
{
  void
    *value;

  struct _ElementInfo
    *next;
} ElementInfo;

typedef struct _EntryInfo
{
  size_t
    hash;

  void
    *key,
    *value;
} EntryInfo;

struct _LinkedListInfo
{
  size_t
    capacity,
    elements;

  ElementInfo
    *head,
    *tail,
    *next;

  WizardBooleanType
    debug;

  SemaphoreInfo
    *semaphore;

  size_t
    signature;
};

struct _HashmapInfo
{
  size_t
    (*hash)(const void *);

  WizardBooleanType
    (*compare)(const void *,const void *);

  void
    *(*relinquish_key)(void *),
    *(*relinquish_value)(void *);

  size_t
    capacity,
    entries,
    next;

  WizardBooleanType
    head_of_list;

  LinkedListInfo
    **map;

  WizardBooleanType
    debug;

  SemaphoreInfo
    *semaphore;

  size_t
    signature;
};

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   A p p e n d V a l u e T o L i n k e d L i s t                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AppendValueToLinkedList() appends a value to the end of the linked-list.
%
%  The format of the AppendValueToLinkedList method is:
%
%      WizardBooleanType AppendValueToLinkedList(LinkedListInfo *list_info,
%        const void *value)
%
%  A description of each parameter follows:
%
%    o list_info: The linked-list info.
%
%    o value: The value.
%
*/
WizardExport WizardBooleanType AppendValueToLinkedList(
  LinkedListInfo *list_info,const void *value)
{
  register ElementInfo
    *next;

  assert(list_info != (LinkedListInfo *) NULL);
  assert(list_info->signature == WizardSignature);
  if (list_info->debug != WizardFalse)
    (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  if (list_info->elements == list_info->capacity)
    return(WizardFalse);
  next=(ElementInfo *) AcquireWizardMemory(sizeof(*next));
  if (next == (ElementInfo *) NULL)
    return(WizardFalse);
  next->value=(void *) value;
  next->next=(ElementInfo *) NULL;
  LockSemaphoreInfo(list_info->semaphore);
  if (list_info->next == (ElementInfo *) NULL)
    list_info->next=next;
  if (list_info->elements == 0)
    list_info->head=next;
  else
    list_info->tail->next=next;
  list_info->tail=next;
  list_info->elements++;
  UnlockSemaphoreInfo(list_info->semaphore);
  return(WizardTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   C l e a r L i n k e d L i s t                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ClearLinkedList() clears all the elements from the linked-list.
%
%  The format of the ClearLinkedList method is:
%
%      void ClearLinkedList(LinkedListInfo *list_info,
%        void *(*relinquish_value)(void *))
%
%  A description of each parameter follows:
%
%    o list_info: The linked-list info.
%
%    o relinquish_value: The value deallocation method; typically
%      RelinquishWizardMemory().
%
*/
WizardExport void ClearLinkedList(LinkedListInfo *list_info,
  void *(*relinquish_value)(void *))
{
  ElementInfo
    *element;

  register ElementInfo
    *next;

  assert(list_info != (LinkedListInfo *) NULL);
  assert(list_info->signature == WizardSignature);
  if (list_info->debug != WizardFalse)
    (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  LockSemaphoreInfo(list_info->semaphore);
  next=list_info->head;
  while (next != (ElementInfo *) NULL)
  {
    if (relinquish_value != (void *(*)(void *)) NULL)
      next->value=relinquish_value(next->value);
    element=next;
    next=next->next;
    element=(ElementInfo *) RelinquishWizardMemory(element);
  }
  list_info->head=(ElementInfo *) NULL;
  list_info->tail=(ElementInfo *) NULL;
  list_info->next=(ElementInfo *) NULL;
  list_info->elements=0;
  UnlockSemaphoreInfo(list_info->semaphore);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   C o m p a r e H a s h m a p S t r i n g                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Specify the CompareHashmapString() method in NewHashmap() to find an entry
%  in a hash-map based on the contents of a string.
%
%  The format of the CompareHashmapString method is:
%
%      WizardBooleanType CompareHashmapString(const void *target,
%        const void *source)
%
%  A description of each parameter follows:
%
%    o target: The target string.
%
%    o source: The source string.
%
*/
WizardExport WizardBooleanType CompareHashmapString(const void *target,
  const void *source)
{
  char
    *p,
    *q;

  p=(char *) target;
  q=(char *) source;
  return(strcmp(p,q) == 0 ? WizardTrue : WizardFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   C o m p a r e H a s h m a p S t r i n g I n f o                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Specify the CompareHashmapStringInfo() method in NewHashmap() to find an
%  entry in a hash-map based on the contents of a string.
%
%  The format of the CompareHashmapStringInfo method is:
%
%      WizardBooleanType CompareHashmapStringInfo(const void *target,
%        const void *source)
%
%  A description of each parameter follows:
%
%    o target: The target string.
%
%    o source: The source string.
%
*/
WizardExport WizardBooleanType CompareHashmapStringInfo(const void *target,
  const void *source)
{
  StringInfo
    *p,
    *q;

  p=(StringInfo *) target;
  q=(StringInfo *) source;
  return(CompareStringInfo(p,q) == 0 ? WizardTrue : WizardFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D e s t r o y H a s h m a p                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DestroyHashmap() frees the hash-map and all associated resources.
%
%  The format of the DestroyHashmap method is:
%
%      HashmapInfo *DestroyHashmap(HashmapInfo *hashmap_info)
%
%  A description of each parameter follows:
%
%    o hashmap_info: The hashmap info.
%
*/
WizardExport HashmapInfo *DestroyHashmap(HashmapInfo *hashmap_info)
{
  LinkedListInfo
    *list_info;

  register EntryInfo
    *entry;

  register ssize_t
    i;

  assert(hashmap_info != (HashmapInfo *) NULL);
  assert(hashmap_info->signature == WizardSignature);
  if (hashmap_info->debug != WizardFalse)
    (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  LockSemaphoreInfo(hashmap_info->semaphore);
  for (i=0; i < (ssize_t) hashmap_info->capacity; i++)
  {
    list_info=hashmap_info->map[i];
    if (list_info != (LinkedListInfo *) NULL)
      {
        list_info->next=list_info->head;
        entry=(EntryInfo *) GetNextValueInLinkedList(list_info);
        while (entry != (EntryInfo *) NULL)
        {
          if (hashmap_info->relinquish_key != (void *(*)(void *)) NULL)
            entry->key=hashmap_info->relinquish_key(entry->key);
          if (hashmap_info->relinquish_value != (void *(*)(void *)) NULL)
            entry->value=hashmap_info->relinquish_value(entry->value);
          entry=(EntryInfo *) GetNextValueInLinkedList(list_info);
        }
      }
    if (list_info != (LinkedListInfo *) NULL)
      list_info=DestroyLinkedList(list_info,RelinquishWizardMemory);
  }
  hashmap_info->map=(LinkedListInfo **) RelinquishWizardMemory(
    hashmap_info->map);
  hashmap_info->signature=(~WizardSignature);
  UnlockSemaphoreInfo(hashmap_info->semaphore);
  DestroySemaphoreInfo(&hashmap_info->semaphore);
  hashmap_info=(HashmapInfo *) RelinquishWizardMemory(hashmap_info);
  return(hashmap_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D e s t r o y L i n k e d L i s t                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DestroyLinkedList() frees the linked-list and all associated resources.
%
%  The format of the DestroyLinkedList method is:
%
%      LinkedListInfo *DestroyLinkedList(LinkedListInfo *list_info,
%        void *(*relinquish_value)(void *))
%
%  A description of each parameter follows:
%
%    o list_info: The linked-list info.
%
%    o relinquish_value: The value deallocation method;  typically
%      RelinquishWizardMemory().
%
*/
WizardExport LinkedListInfo *DestroyLinkedList(LinkedListInfo *list_info,
  void *(*relinquish_value)(void *))
{
  ElementInfo
    *entry;

  register ElementInfo
    *next;

  assert(list_info != (LinkedListInfo *) NULL);
  assert(list_info->signature == WizardSignature);
  if (list_info->debug != WizardFalse)
    (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  LockSemaphoreInfo(list_info->semaphore);
  for (next=list_info->head; next != (ElementInfo *) NULL; )
  {
    if (relinquish_value != (void *(*)(void *)) NULL)
      next->value=relinquish_value(next->value);
    entry=next;
    next=next->next;
    entry=(ElementInfo *) RelinquishWizardMemory(entry);
  }
  list_info->signature=(~WizardSignature);
  UnlockSemaphoreInfo(list_info->semaphore);
  DestroySemaphoreInfo(&list_info->semaphore);
  list_info=(LinkedListInfo *) RelinquishWizardMemory(list_info);
  return(list_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t L a s t V a l u e I n L i n k e d L i s t                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetLastValueInLinkedList() gets the last value in the linked-list.
%
%  The format of the GetLastValueInLinkedList method is:
%
%      void *GetLastValueInLinkedList(LinkedListInfo *list_info)
%
%  A description of each parameter follows:
%
%    o list_info: The linked_list info.
%
*/
WizardExport void *GetLastValueInLinkedList(LinkedListInfo *list_info)
{
  void
    *value;

  assert(list_info != (LinkedListInfo *) NULL);
  assert(list_info->signature == WizardSignature);
  if (list_info->debug != WizardFalse)
    (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  if (list_info->elements == 0)
    return((void *) NULL);
  LockSemaphoreInfo(list_info->semaphore);
  value=list_info->tail->value;
  UnlockSemaphoreInfo(list_info->semaphore);
  return(value);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t N e x t K e y I n H a s h m a p                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetNextKeyInHashmap() gets the next key in the hash-map.
%
%  The format of the GetNextKeyInHashmap method is:
%
%      void *GetNextKeyInHashmap(HashmapInfo *hashmap_info)
%
%  A description of each parameter follows:
%
%    o hashmap_info: The hashmap info.
%
*/
WizardExport void *GetNextKeyInHashmap(HashmapInfo *hashmap_info)
{
  LinkedListInfo
    *list_info;

  register EntryInfo
    *entry;

  void
    *key;

  assert(hashmap_info != (HashmapInfo *) NULL);
  assert(hashmap_info->signature == WizardSignature);
  if (hashmap_info->debug != WizardFalse)
    (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  LockSemaphoreInfo(hashmap_info->semaphore);
  while (hashmap_info->next < hashmap_info->capacity)
  {
    list_info=hashmap_info->map[hashmap_info->next];
    if (list_info != (LinkedListInfo *) NULL)
      {
        if (hashmap_info->head_of_list == WizardFalse)
          {
            list_info->next=list_info->head;
            hashmap_info->head_of_list=WizardTrue;
          }
        entry=(EntryInfo *) GetNextValueInLinkedList(list_info);
        if (entry != (EntryInfo *) NULL)
          {
            key=entry->key;
            UnlockSemaphoreInfo(hashmap_info->semaphore);
            return(key);
          }
        hashmap_info->head_of_list=WizardFalse;
      }
    hashmap_info->next++;
  }
  UnlockSemaphoreInfo(hashmap_info->semaphore);
  return((void *) NULL);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t N e x t V a l u e I n H a s h m a p                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetNextValueInHashmap() gets the next value in the hash-map.
%
%  The format of the GetNextValueInHashmap method is:
%
%      void *GetNextValueInHashmap(HashmapInfo *hashmap_info)
%
%  A description of each parameter follows:
%
%    o hashmap_info: The hashmap info.
%
*/
WizardExport void *GetNextValueInHashmap(HashmapInfo *hashmap_info)
{
  LinkedListInfo
    *list_info;

  register EntryInfo
    *entry;

  void
    *value;

  assert(hashmap_info != (HashmapInfo *) NULL);
  assert(hashmap_info->signature == WizardSignature);
  if (hashmap_info->debug != WizardFalse)
    (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  LockSemaphoreInfo(hashmap_info->semaphore);
  while (hashmap_info->next < hashmap_info->capacity)
  {
    list_info=hashmap_info->map[hashmap_info->next];
    if (list_info != (LinkedListInfo *) NULL)
      {
        if (hashmap_info->head_of_list == WizardFalse)
          {
            list_info->next=list_info->head;
            hashmap_info->head_of_list=WizardTrue;
          }
        entry=(EntryInfo *) GetNextValueInLinkedList(list_info);
        if (entry != (EntryInfo *) NULL)
          {
            value=entry->value;
            UnlockSemaphoreInfo(hashmap_info->semaphore);
            return(value);
          }
        hashmap_info->head_of_list=WizardFalse;
      }
    hashmap_info->next++;
  }
  UnlockSemaphoreInfo(hashmap_info->semaphore);
  return((void *) NULL);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t N e x t V a l u e I n L i n k e d L i s t                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetNextValueInLinkedList() gets the next value in the linked-list.
%
%  The format of the GetNextValueInLinkedList method is:
%
%      void *GetNextValueInLinkedList(LinkedListInfo *list_info)
%
%  A description of each parameter follows:
%
%    o list_info: The linked-list info.
%
*/
WizardExport void *GetNextValueInLinkedList(LinkedListInfo *list_info)
{
  void
    *value;

  assert(list_info != (LinkedListInfo *) NULL);
  assert(list_info->signature == WizardSignature);
  if (list_info->debug != WizardFalse)
    (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  LockSemaphoreInfo(list_info->semaphore);
  if (list_info->next == (ElementInfo *) NULL)
    {
      UnlockSemaphoreInfo(list_info->semaphore);
      return((void *) NULL);
    }
  value=list_info->next->value;
  list_info->next=list_info->next->next;
  UnlockSemaphoreInfo(list_info->semaphore);
  return(value);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t N u m b e r O f E n t r i e s I n H a s h m a p                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetNumberOfEntriesInHashmap() returns the number of entries in the hash-map.
%
%  The format of the GetNumberOfEntriesInHashmap method is:
%
%      size_t GetNumberOfEntriesInHashmap(const HashmapInfo *hashmap_info)
%
%  A description of each parameter follows:
%
%    o hashmap_info: The hashmap info.
%
*/
WizardExport size_t GetNumberOfEntriesInHashmap(
  const HashmapInfo *hashmap_info)
{
  assert(hashmap_info != (HashmapInfo *) NULL);
  assert(hashmap_info->signature == WizardSignature);
  if (hashmap_info->debug != WizardFalse)
    (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  return(hashmap_info->entries);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t N u m b e r O f E l e m e n t s I n L i n k e d L i s t             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetNumberOfElementsInLinkedList() returns the number of entries in the
%  linked-list.
%
%  The format of the GetNumberOfElementsInLinkedList method is:
%
%      size_t GetNumberOfElementsInLinkedList(
%        const LinkedListInfo *list_info)
%
%  A description of each parameter follows:
%
%    o list_info: The linked-list info.
%
*/
WizardExport size_t GetNumberOfElementsInLinkedList(
  const LinkedListInfo *list_info)
{
  assert(list_info != (LinkedListInfo *) NULL);
  assert(list_info->signature == WizardSignature);
  if (list_info->debug != WizardFalse)
    (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  return(list_info->elements);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t V a l u e F r o m H a s h m a p                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetValueFromHashmap() gets a value from the hash-map by its key.
%
%  The format of the GetValueFromHashmap method is:
%
%      void *GetValueFromHashmap(HashmapInfo *hashmap_info,const void *key)
%
%  A description of each parameter follows:
%
%    o hashmap_info: The hashmap info.
%
%    o key: The key.
%
*/
WizardExport void *GetValueFromHashmap(HashmapInfo *hashmap_info,
  const void *key)
{
  LinkedListInfo
    *list_info;

  register EntryInfo
    *entry;

  size_t
    hash;

  void
    *value;

  assert(hashmap_info != (HashmapInfo *) NULL);
  assert(hashmap_info->signature == WizardSignature);
  if (hashmap_info->debug != WizardFalse)
    (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  if (key == NULL)
    return((void *) NULL);
  LockSemaphoreInfo(hashmap_info->semaphore);
  hash=hashmap_info->hash(key);
  list_info=hashmap_info->map[hash % hashmap_info->capacity];
  if (list_info != (LinkedListInfo *) NULL)
    {
      list_info->next=list_info->head;
      entry=(EntryInfo *) GetNextValueInLinkedList(list_info);
      while (entry != (EntryInfo *) NULL)
      {
        if (entry->hash == hash)
          {
            WizardBooleanType
              compare;

            compare=WizardTrue;
            if (hashmap_info->compare !=
                (WizardBooleanType (*)(const void *,const void *)) NULL)
              compare=hashmap_info->compare(key,entry->key);
            if (compare == WizardTrue)
              {
                value=entry->value;
                UnlockSemaphoreInfo(hashmap_info->semaphore);
                return(value);
              }
          }
        entry=(EntryInfo *) GetNextValueInLinkedList(list_info);
      }
    }
  UnlockSemaphoreInfo(hashmap_info->semaphore);
  return((void *) NULL);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t V a l u e F r o m L i n k e d L i s t                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetValueFromLinkedList() gets a vlaue from the linked-list by the specified
%  location.
%
%  The format of the GetValueFromLinkedList method is:
%
%      void *GetValueFromLinkedList(LinkedListInfo *list_info,
%        const size_t index)
%
%  A description of each parameter follows:
%
%    o list_info: The linked_list info.
%
%    o index: The list index.
%
*/
WizardExport void *GetValueFromLinkedList(LinkedListInfo *list_info,
  const size_t index)
{
  register ElementInfo
    *next;

  register ssize_t
    i;

  void
    *value;

  assert(list_info != (LinkedListInfo *) NULL);
  assert(list_info->signature == WizardSignature);
  if (list_info->debug != WizardFalse)
    (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  if (index >= list_info->elements)
    return((void *) NULL);
  LockSemaphoreInfo(list_info->semaphore);
  if (index == 0)
    {
      value=list_info->head->value;
      UnlockSemaphoreInfo(list_info->semaphore);
      return(value);
    }
  if (index == (list_info->elements-1))
    {
      value=list_info->tail->value;
      UnlockSemaphoreInfo(list_info->semaphore);
      return(value);
    }
  next=list_info->head;
  for (i=0; i < (ssize_t) index; i++)
    next=next->next;
  value=next->value;
  UnlockSemaphoreInfo(list_info->semaphore);
  return(value);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   H a s h P o i n t e r T y p e                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Specify the HashPointerType() method in NewHashmap() to find an entry
%  in a hash-map based on the address of a pointer.
%
%  The format of the HashPointerType method is:
%
%      size_t HashPointerType(const void *pointer)
%
%  A description of each parameter follows:
%
%    o pointer: compute the hash entry location from this pointer address.
%
*/
WizardExport size_t HashPointerType(const void *pointer)
{
  size_t
    hash;

  hash=(size_t) pointer;
  hash+=(~(hash << 9));
  hash^=(hash >> 14);
  hash+=(hash << 4);
  hash^=(hash >> 10);
  return(hash);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   H a s h S t r i n g T y p e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Specify the HashStringType() method in NewHashmap() to find an entry
%  in a hash-map based on the contents of a string.
%
%  The format of the HashStringType method is:
%
%      size_t HashStringType(const void *string)
%
%  A description of each parameter follows:
%
%    o string: compute the hash entry location from this string.
%
*/
WizardExport size_t HashStringType(const void *string)
{
  const StringInfo
    *digest;

  HashInfo
    *hashmap_info;

  register size_t
    i;

  size_t
    hash;

  StringInfo
    *message;

  unsigned char
    *datum;

  hashmap_info=AcquireHashInfo(CRC64Hash);
  if (hashmap_info == (HashInfo *) NULL)
    return((size_t) string);
  InitializeHash(hashmap_info);
  message=StringToStringInfo((const char *) string);
  UpdateHash(hashmap_info,message);
  FinalizeHash(hashmap_info);
  hash=0;
  digest=GetHashDigest(hashmap_info);
  datum=GetStringInfoDatum(digest);
  for (i=0; i < GetStringInfoLength(digest); i++)
    hash^=datum[i];
  hashmap_info=DestroyHashInfo(hashmap_info);
  return(hash);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   H a s h S t r i n g I n f o T y p e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Specify the HashStringInfoType() method in NewHashmap() to find an entry
%  in a hash-map based on the contents of a string.
%
%  The format of the HashStringInfoType method is:
%
%      size_t HashStringInfoType(const void *string)
%
%  A description of each parameter follows:
%
%    o string: compute the hash entry location from this string.
%
*/
WizardExport size_t HashStringInfoType(const void *string)
{
  register size_t
    i;

  size_t
    hash;

  StringInfo
    *message;

  unsigned char
    *datum;

  message=(StringInfo *) string;
  hash=0;
  datum=GetStringInfoDatum(message);
  for (i=0; i < GetStringInfoLength(message); i++)
    hash^=datum[i];
  return(hash);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I n s e r t V a l u e I n L i n k e d L i s t                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  InsertValueInLinkedList() inserts an element in the linked-list at the
%  specified location.
%
%  The format of the InsertValueInLinkedList method is:
%
%      WizardBooleanType InsertValueInLinkedList(ListInfo *list_info,
%        const size_t index,const void *value)
%
%  A description of each parameter follows:
%
%    o list_info: The hashmap info.
%
%    o index: The index.
%
%    o value: The value.
%
*/
WizardExport WizardBooleanType InsertValueInLinkedList(
  LinkedListInfo *list_info,const size_t index,const void *value)
{
  register ElementInfo
    *next;

  register ssize_t
    i;

  assert(list_info != (LinkedListInfo *) NULL);
  assert(list_info->signature == WizardSignature);
  if (list_info->debug != WizardFalse)
    (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  if (value == NULL)
    return(WizardFalse);
  if ((index > list_info->elements) ||
      (list_info->elements == list_info->capacity))
    return(WizardFalse);
  next=(ElementInfo *) AcquireWizardMemory(sizeof(*next));
  if (next == (ElementInfo *) NULL)
    return(WizardFalse);
  next->value=(void *) value;
  next->next=(ElementInfo *) NULL;
  LockSemaphoreInfo(list_info->semaphore);
  if (list_info->elements == 0)
    {
      if (list_info->next == (ElementInfo *) NULL)
        list_info->next=next;
      list_info->head=next;
      list_info->tail=next;
    }
  else
    {
      if (index == 0)
        {
          if (list_info->next == list_info->head)
            list_info->next=next;
          next->next=list_info->head;
          list_info->head=next;
        }
      else
        if (index == list_info->elements)
          {
            if (list_info->next == (ElementInfo *) NULL)
              list_info->next=next;
            list_info->tail->next=next;
            list_info->tail=next;
          }
        else
          {
            ElementInfo
              *element;

            element=list_info->head;
            next->next=element->next;
            for (i=1; i < (ssize_t) index; i++)
            {
              element=element->next;
              next->next=element->next;
            }
            next=next->next;
            element->next=next;
            if (list_info->next == next->next)
              list_info->next=next;
          }
    }
  list_info->elements++;
  UnlockSemaphoreInfo(list_info->semaphore);
  return(WizardTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I n s e r t V a l u e I n S o r t e d L i n k e d L i s t                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  InsertValueInSortedLinkedList() inserts a value in the sorted linked-list.
%
%  The format of the InsertValueInSortedLinkedList method is:
%
%      WizardBooleanType InsertValueInSortedLinkedList(ListInfo *list_info,
%        int (*compare)(const void *,const void *),void **replace,
%        const void *value)
%
%  A description of each parameter follows:
%
%    o list_info: The hashmap info.
%
%    o index: The index.
%
%    o compare: The compare method.
%
%    o replace: return previous element here.
%
%    o value: The value.
%
*/
WizardExport WizardBooleanType InsertValueInSortedLinkedList(
  LinkedListInfo *list_info,int (*compare)(const void *,const void *),
  void **replace,const void *value)
{
  ElementInfo
    *element;

  register ElementInfo
    *next;

  register ssize_t
    i;

  assert(list_info != (LinkedListInfo *) NULL);
  assert(list_info->signature == WizardSignature);
  if (list_info->debug != WizardFalse)
    (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  if ((compare == (int (*)(const void *,const void *)) NULL) ||
      (value == (const void *) NULL))
    return(WizardFalse);
  if (list_info->elements == list_info->capacity)
    return(WizardFalse);
  next=(ElementInfo *) AcquireWizardMemory(sizeof(*next));
  if (next == (ElementInfo *) NULL)
    return(WizardFalse);
  next->value=(void *) value;
  element=(ElementInfo *) NULL;
  LockSemaphoreInfo(list_info->semaphore);
  next->next=list_info->head;
  while (next->next != (ElementInfo *) NULL)
  {
    i=compare(value,next->next->value);
    if ((i < 0) || ((replace != (void **) NULL) && (i == 0)))
      {
        if (i == 0)
          {
            *replace=next->next->value;
            next->next=next->next->next;
            if (element != (ElementInfo *) NULL)
              element->next=(ElementInfo *) RelinquishWizardMemory(
                element->next);
            list_info->elements--;
          }
        if (element != (ElementInfo *) NULL)
          element->next=next;
        else
          list_info->head=next;
        break;
      }
    element=next->next;
    next->next=next->next->next;
  }
  if (next->next == (ElementInfo *) NULL)
    {
      if (element != (ElementInfo *) NULL)
        element->next=next;
      else
        list_info->head=next;
      list_info->tail=next;
    }
  list_info->elements++;
  UnlockSemaphoreInfo(list_info->semaphore);
  return(WizardTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s H a s h m a p E m p t y                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsHashmapEmpty() returns WizardTrue if the hash-map is empty.
%
%  The format of the IsHashmapEmpty method is:
%
%      WizardBooleanType IsHashmapEmpty(const HashmapInfo *hashmap_info)
%
%  A description of each parameter follows:
%
%    o hashmap_info: The hashmap info.
%
*/
WizardExport WizardBooleanType IsHashmapEmpty(const HashmapInfo *hashmap_info)
{
  assert(hashmap_info != (HashmapInfo *) NULL);
  assert(hashmap_info->signature == WizardSignature);
  if (hashmap_info->debug != WizardFalse)
    (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  return(hashmap_info->entries == 0 ? WizardTrue : WizardFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s L i n k e d L i s t E m p t y                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsLinkedListEmpty() returns WizardTrue if the linked-list is empty.
%
%  The format of the IsLinkedListEmpty method is:
%
%      WizardBooleanType IsLinkedListEmpty(LinkedListInfo *list_info)
%
%  A description of each parameter follows:
%
%    o list_info: The linked-list info.
%
*/
WizardExport WizardBooleanType IsLinkedListEmpty(
  const LinkedListInfo *list_info)
{
  assert(list_info != (LinkedListInfo *) NULL);
  assert(list_info->signature == WizardSignature);
  if (list_info->debug != WizardFalse)
    (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  return(list_info->elements == 0 ? WizardTrue : WizardFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   L i n k e d L i s t T o A r r a y                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  LinkedListToArray() converts the linked-list to an array.
%
%  The format of the LinkedListToArray method is:
%
%      WizardBooleanType LinkedListToArray(LinkedListInfo *list_info,
%        void **array)
%
%  A description of each parameter follows:
%
%    o list_info: The linked-list info.
%
%    o array: The array.
%
*/
WizardExport WizardBooleanType LinkedListToArray(LinkedListInfo *list_info,
  void **array)
{
  register ElementInfo
    *next;

  register ssize_t
    i;

  assert(list_info != (LinkedListInfo *) NULL);
  assert(list_info->signature == WizardSignature);
  if (list_info->debug != WizardFalse)
    (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  if (array == (void **) NULL)
    return(WizardFalse);
  LockSemaphoreInfo(list_info->semaphore);
  next=list_info->head;
  for (i=0; next != (ElementInfo *) NULL; i++)
  {
    array[i]=next->value;
    next=next->next;
  }
  UnlockSemaphoreInfo(list_info->semaphore);
  return(WizardTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   N e w H a s h m a p                                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  NewHashmap() returns a pointer to a HashmapInfo structure initialized
%  to default values.  The capacity is an initial estimate.  The hashmap will
%  increase capacity dynamically as the demand requires.
%
%  The format of the NewHashmap method is:
%
%      HashmapInfo *NewHashmap(const size_t capacity,
%        size_t (*hash)(const void *),
%        WizardBooleanType (*compare)(const void *,const void *),
%        void *(*relinquish_key)(void *),void *(*relinquish_value)(void *))
%
%  A description of each parameter follows:
%
%    o capacity: The initial number entries in the hash-map: typically
%      SmallHashmapSize, MediumHashmapSize, or LargeHashmapSize.  The
%      hashmap will dynamically increase its capacity on demand.
%
%    o hash: The hash method, typically HashPointerType(), HashStringType(),
%      or HashStringInfoType().
%
%    o compare: The compare method, typically NULL, CompareHashmapString(),
%      or CompareHashmapStringInfo().
%
%    o relinquish_key: The key deallocation method, typically
%      RelinquishWizardMemory(), called whenever a key is removed from the
%      hash-map.
%
%    o relinquish_value: The value deallocation method;  typically
%      RelinquishWizardMemory(), called whenever a value object is removed from
%      the hash-map.
%
*/
WizardExport HashmapInfo *NewHashmap(const size_t capacity,
  size_t (*hash)(const void *),
  WizardBooleanType (*compare)(const void *,const void *),
  void *(*relinquish_key)(void *),void *(*relinquish_value)(void *))
{
  HashmapInfo
    *hashmap_info;

  hashmap_info=(HashmapInfo *) AcquireWizardMemory(sizeof(*hashmap_info));
  if (hashmap_info == (HashmapInfo *) NULL)
    ThrowWizardFatalError(CacheDomain,MemoryError);
  (void) ResetWizardMemory(hashmap_info,0,sizeof(*hashmap_info));
  hashmap_info->hash=HashPointerType;
  if (hash != (size_t (*)(const void *)) NULL)
    hashmap_info->hash=hash;
  hashmap_info->compare=(WizardBooleanType (*)(const void *,const void *)) NULL;
  if (compare != (WizardBooleanType (*)(const void *,const void *)) NULL)
    hashmap_info->compare=compare;
  hashmap_info->relinquish_key=relinquish_key;
  hashmap_info->relinquish_value=relinquish_value;
  hashmap_info->entries=0;
  hashmap_info->capacity=capacity;
  hashmap_info->map=(LinkedListInfo **) AcquireQuantumMemory((size_t)
    capacity+1UL,sizeof(*hashmap_info->map));
  if (hashmap_info->map == (LinkedListInfo **) NULL)
    ThrowWizardFatalError(CacheDomain,MemoryError);
  (void) ResetWizardMemory(hashmap_info->map,0,(size_t) capacity*
    sizeof(*hashmap_info->map));
  hashmap_info->debug=IsEventLogging();
  hashmap_info->semaphore=AllocateSemaphoreInfo();
  hashmap_info->signature=WizardSignature;
  return(hashmap_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   N e w L i n k e d L i s t I n f o                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  NewLinkedList() returns a pointer to a LinkedListInfo structure
%  initialized to default values.
%
%  The format of the Acquirestruct LinkedListInfomethod is:
%
%      LinkedListInfo *NewLinkedList(const size_t capacity)
%
%  A description of each parameter follows:
%
%    o capacity: The maximum number of elements in the list.
%
*/
WizardExport LinkedListInfo *NewLinkedList(const size_t capacity)
{
  LinkedListInfo
    *list_info;

  list_info=(LinkedListInfo *) AcquireWizardMemory(sizeof(*list_info));
  if (list_info == (LinkedListInfo *) NULL)
    ThrowWizardFatalError(CacheDomain,MemoryError);
  (void) ResetWizardMemory(list_info,0,sizeof(*list_info));
  list_info->capacity=capacity == 0 ? (size_t) (~0) : capacity;
  list_info->elements=0;
  list_info->head=(ElementInfo *) NULL;
  list_info->tail=(ElementInfo *) NULL;
  list_info->next=(ElementInfo *) NULL;
  list_info->debug=WizardFalse;
  list_info->semaphore=AllocateSemaphoreInfo();
  list_info->signature=WizardSignature;
  return(list_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   P u t E n t r y I n H a s h m a p                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  PutEntryInHashmap() puts an entry in the hash-map.  If the key already
%  exists in the map it is first removed.
%
%  The format of the PutEntryInHashmap method is:
%
%      WizardBooleanType PutEntryInHashmap(HashmapInfo *hashmap_info,
%        const void *key,const void *value)
%
%  A description of each parameter follows:
%
%    o hashmap_info: The hashmap info.
%
%    o key: The key.
%
%    o value: The value.
%
*/

static WizardBooleanType IncreaseHashmapCapacity(HashmapInfo *hashmap_info)
{
#define MaxCapacities  20

  const size_t
    capacities[MaxCapacities] =
    {
      17, 31, 61, 131, 257, 509, 1021, 2053, 4099, 8191, 16381, 32771,
      65537, 131071, 262147, 524287, 1048573, 2097143, 4194301, 8388617
    };

  ElementInfo
    *element;

  EntryInfo
    *entry;

  LinkedListInfo
    *list_info,
    *map_info,
    **map;

  register ElementInfo
    *next;

  register ssize_t
    i;

  size_t
    capacity;

  /*
    Increase to the next prime capacity.
  */
  for (i=0; i < MaxCapacities; i++)
    if (hashmap_info->capacity < capacities[i])
      break;
  if (i >= (MaxCapacities-1))
    return(WizardFalse);
  capacity=capacities[i+1];
  map=(LinkedListInfo **) AcquireQuantumMemory((size_t) capacity+1UL,
    sizeof(*map));
  if (map == (LinkedListInfo **) NULL)
    return(WizardFalse);
  (void) ResetWizardMemory(map,0,(size_t) capacity*sizeof(*map));
  /*
    Copy entries to new hashmap with increased capacity.
  */
  for (i=0; i < (ssize_t) hashmap_info->capacity; i++)
  {
    list_info=hashmap_info->map[i];
    if (list_info == (LinkedListInfo *) NULL)
      continue;
    LockSemaphoreInfo(list_info->semaphore);
    for (next=list_info->head; next != (ElementInfo *) NULL; )
    {
      element=next;
      next=next->next;
      entry=(EntryInfo *) element->value;
      map_info=map[entry->hash % capacity];
      if (map_info == (LinkedListInfo *) NULL)
        {
          map_info=NewLinkedList(0);
          map[entry->hash % capacity]=map_info;
        }
      map_info->next=element;
      element->next=map_info->head;
      map_info->head=element;
      map_info->elements++;
    }
    list_info->signature=(~WizardSignature);
    UnlockSemaphoreInfo(list_info->semaphore);
    DestroySemaphoreInfo(&list_info->semaphore);
    list_info=(LinkedListInfo *) RelinquishWizardMemory(list_info);
  }
  hashmap_info->map=(LinkedListInfo **) RelinquishWizardMemory(hashmap_info->map);
  hashmap_info->map=map;
  hashmap_info->capacity=capacity;
  return(WizardTrue);
}

WizardExport WizardBooleanType PutEntryInHashmap(HashmapInfo *hashmap_info,
  const void *key,const void *value)
{
  EntryInfo
    *entry,
    *next;

  LinkedListInfo
    *list_info;

  register size_t
    i;

  assert(hashmap_info != (HashmapInfo *) NULL);
  assert(hashmap_info->signature == WizardSignature);
  if (hashmap_info->debug != WizardFalse)
    (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  if ((key == (void *) NULL) || (value == (void *) NULL))
    return(WizardFalse);
  next=(EntryInfo *) AcquireWizardMemory(sizeof(*next));
  if (next == (EntryInfo *) NULL)
    return(WizardFalse);
  LockSemaphoreInfo(hashmap_info->semaphore);
  next->hash=hashmap_info->hash(key);
  next->key=(void *) key;
  next->value=(void *) value;
  list_info=hashmap_info->map[next->hash % hashmap_info->capacity];
  if (list_info == (LinkedListInfo *) NULL)
    {
      list_info=NewLinkedList(0);
      hashmap_info->map[next->hash % hashmap_info->capacity]=list_info;
    }
  else
    {
      list_info->next=list_info->head;
      entry=(EntryInfo *) GetNextValueInLinkedList(list_info);
      for (i=0; entry != (EntryInfo *) NULL; i++)
      {
        if (entry->hash == next->hash)
          {
            WizardBooleanType
              compare;

            compare=WizardTrue;
            if (hashmap_info->compare !=
                (WizardBooleanType (*)(const void *,const void *)) NULL)
              compare=hashmap_info->compare(key,entry->key);
            if (compare == WizardTrue)
              {
                (void) RemoveElementFromLinkedList(list_info,i);
                if (hashmap_info->relinquish_key != (void *(*)(void *)) NULL)
                  entry->key=hashmap_info->relinquish_key(entry->key);
                if (hashmap_info->relinquish_value != (void *(*)(void *)) NULL)
                  entry->value=hashmap_info->relinquish_value(entry->value);
                entry=(EntryInfo *) RelinquishWizardMemory(entry);
                break;
              }
          }
        entry=(EntryInfo *) GetNextValueInLinkedList(list_info);
      }
    }
  if (InsertValueInLinkedList(list_info,0,next) == WizardFalse)
    {
      next=(EntryInfo *) RelinquishWizardMemory(next);
      UnlockSemaphoreInfo(hashmap_info->semaphore);
      return(WizardFalse);
    }
  if (list_info->elements >= (hashmap_info->capacity-1))
    if (IncreaseHashmapCapacity(hashmap_info) == WizardFalse)
      {
        UnlockSemaphoreInfo(hashmap_info->semaphore);
        return(WizardFalse);
      }
  hashmap_info->entries++;
  UnlockSemaphoreInfo(hashmap_info->semaphore);
  return(WizardTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e m o v e E l e m e n t B y V a l u e F r o m L i n k e d L i s t       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RemoveElementByValueFromLinkedList() removes an element from the linked-list
%  by value.
%
%  The format of the RemoveElementByValueFromLinkedList method is:
%
%      void *RemoveElementByValueFromLinkedList(LinkedListInfo *list_info,
%        const void *value)
%
%  A description of each parameter follows:
%
%    o list_info: The list info.
%
%    o value: The value.
%
*/
WizardExport void *RemoveElementByValueFromLinkedList(LinkedListInfo *list_info,
  const void *value)
{
  ElementInfo
    *next;

  assert(list_info != (LinkedListInfo *) NULL);
  assert(list_info->signature == WizardSignature);
  if (list_info->debug != WizardFalse)
    (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  if ((list_info->elements == 0) || (value == NULL))
    return((void *) NULL);
  LockSemaphoreInfo(list_info->semaphore);
  if (value == list_info->head->value)
    {
      if (list_info->next == list_info->head)
        list_info->next=list_info->head->next;
      next=list_info->head;
      list_info->head=list_info->head->next;
      next=(ElementInfo *) RelinquishWizardMemory(next);
    }
  else
    {
      ElementInfo
        *element;

      next=list_info->head;
      while ((next->next != (ElementInfo *) NULL) &&
             (next->next->value != value))
        next=next->next;
      if (next->next == (ElementInfo *) NULL)
        {
          UnlockSemaphoreInfo(list_info->semaphore);
          return((void *) NULL);
        }
      element=next->next;
      next->next=element->next;
      if (element == list_info->tail)
        list_info->tail=next;
      if (list_info->next == element)
        list_info->next=element->next;
      element=(ElementInfo *) RelinquishWizardMemory(element);
    }
  list_info->elements--;
  UnlockSemaphoreInfo(list_info->semaphore);
  return((void *) value);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e m o v e E l e m e n t F r o m L i n k e d L i s t                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RemoveElementFromLinkedList() removes an element from the linked-list at the
%  specified location.
%
%  The format of the RemoveElementFromLinkedList method is:
%
%      void *RemoveElementFromLinkedList(LinkedListInfo *list_info,
%        const size_t index)
%
%  A description of each parameter follows:
%
%    o list_info: The linked-list info.
%
%    o index: The index.
%
*/
WizardExport void *RemoveElementFromLinkedList(LinkedListInfo *list_info,
  const size_t index)
{
  ElementInfo
    *next;

  register ssize_t
    i;

  void
    *value;

  assert(list_info != (LinkedListInfo *) NULL);
  assert(list_info->signature == WizardSignature);
  if (list_info->debug != WizardFalse)
    (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  if (index >= list_info->elements)
    return((void *) NULL);
  LockSemaphoreInfo(list_info->semaphore);
  if (index == 0)
    {
      if (list_info->next == list_info->head)
        list_info->next=list_info->head->next;
      value=list_info->head->value;
      next=list_info->head;
      list_info->head=list_info->head->next;
      next=(ElementInfo *) RelinquishWizardMemory(next);
    }
  else
    {
      ElementInfo
        *element;

      next=list_info->head;
      for (i=1; i < (ssize_t) index; i++)
        next=next->next;
      element=next->next;
      next->next=element->next;
      if (element == list_info->tail)
        list_info->tail=next;
      if (list_info->next == element)
        list_info->next=element->next;
      value=element->value;
      element=(ElementInfo *) RelinquishWizardMemory(element);
    }
  list_info->elements--;
  UnlockSemaphoreInfo(list_info->semaphore);
  return(value);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e m o v e E n t r y F r o m H a s h m a p                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RemoveEntryFromHashmap() removes an entry from the hash-map by its key.
%
%  The format of the RemoveEntryFromHashmap method is:
%
%      void *RemoveEntryFromHashmap(HashmapInfo *hashmap_info,void *key)
%
%  A description of each parameter follows:
%
%    o hashmap_info: The hashmap info.
%
%    o key: The key.
%
*/
WizardExport void *RemoveEntryFromHashmap(HashmapInfo *hashmap_info,
  const void *key)
{
  EntryInfo
    *entry;

  LinkedListInfo
    *list_info;

  register size_t
    i;

  size_t
    hash;

  void
    *value;

  assert(hashmap_info != (HashmapInfo *) NULL);
  assert(hashmap_info->signature == WizardSignature);
  if (hashmap_info->debug != WizardFalse)
    (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  if (key == NULL)
    return((void *) NULL);
  LockSemaphoreInfo(hashmap_info->semaphore);
  hash=hashmap_info->hash(key);
  list_info=hashmap_info->map[hash % hashmap_info->capacity];
  if (list_info != (LinkedListInfo *) NULL)
    {
      list_info->next=list_info->head;
      entry=(EntryInfo *) GetNextValueInLinkedList(list_info);
      for (i=0; entry != (EntryInfo *) NULL; i++)
      {
        if (entry->hash == hash)
          {
            WizardBooleanType
              compare;

            compare=WizardTrue;
            if (hashmap_info->compare !=
                (WizardBooleanType (*)(const void *,const void *)) NULL)
              compare=hashmap_info->compare(key,entry->key);
            if (compare == WizardTrue)
              {
                entry=(EntryInfo *) RemoveElementFromLinkedList(list_info,i);
                if (entry == (EntryInfo *) NULL)
                  {
                    UnlockSemaphoreInfo(hashmap_info->semaphore);
                    return((void *) NULL);
                  }
                if (hashmap_info->relinquish_key != (void *(*)(void *)) NULL)
                  entry->key=hashmap_info->relinquish_key(entry->key);
                value=entry->value;
                entry=(EntryInfo *) RelinquishWizardMemory(entry);
                hashmap_info->entries--;
                UnlockSemaphoreInfo(hashmap_info->semaphore);
                return(value);
              }
          }
        entry=(EntryInfo *) GetNextValueInLinkedList(list_info);
      }
    }
  UnlockSemaphoreInfo(hashmap_info->semaphore);
  return((void *) NULL);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e m o v e L a s t E l e m e n t F r o m L i n k e d L i s t             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RemoveLastElementFromLinkedList() removes the last element from the
%  linked-list.
%
%  The format of the RemoveLastElementFromLinkedList method is:
%
%      void *RemoveLastElementFromLinkedList(LinkedListInfo *list_info)
%
%  A description of each parameter follows:
%
%    o list_info: The linked-list info.
%
*/
WizardExport void *RemoveLastElementFromLinkedList(LinkedListInfo *list_info)
{
  void
    *value;

  assert(list_info != (LinkedListInfo *) NULL);
  assert(list_info->signature == WizardSignature);
  if (list_info->debug != WizardFalse)
    (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  if (list_info->elements == 0)
    return((void *) NULL);
  LockSemaphoreInfo(list_info->semaphore);
  if (list_info->next == list_info->tail)
    list_info->next=(ElementInfo *) NULL;
  if (list_info->elements == 1UL)
    {
      value=list_info->head->value;
      list_info->head=(ElementInfo *) NULL;
      list_info->tail=(ElementInfo *) RelinquishWizardMemory(list_info->tail);
    }
  else
    {
      ElementInfo
        *next;

      value=list_info->tail->value;
      next=list_info->head;
      while (next->next != list_info->tail)
        next=next->next;
      list_info->tail=(ElementInfo *) RelinquishWizardMemory(list_info->tail);
      list_info->tail=next;
      next->next=(ElementInfo *) NULL;
    }
  list_info->elements--;
  UnlockSemaphoreInfo(list_info->semaphore);
  return(value);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e s e t H a s h m a p I t e r a t o r                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ResetHashmapIterator() resets the hash-map iterator.  Use it in conjunction
%  with GetNextKeyInHashmap() to iterate over all the keys in the hash-map.
%
%  The format of the ResetHashmapIterator method is:
%
%      ResetHashmapIterator(HashmapInfo *hashmap_info)
%
%  A description of each parameter follows:
%
%    o hashmap_info: The hashmap info.
%
*/
WizardExport void ResetHashmapIterator(HashmapInfo *hashmap_info)
{
  assert(hashmap_info != (HashmapInfo *) NULL);
  assert(hashmap_info->signature == WizardSignature);
  if (hashmap_info->debug != WizardFalse)
    (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  LockSemaphoreInfo(hashmap_info->semaphore);
  hashmap_info->next=0;
  hashmap_info->head_of_list=WizardFalse;
  UnlockSemaphoreInfo(hashmap_info->semaphore);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e s e t L i n k e d L i s t I t e r a t o r                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ResetLinkedListIterator() resets the lined-list iterator.  Use it in
%  conjunction with GetNextValueInLinkedList() to iterate over all the values
%  in the linked-list.
%
%  The format of the ResetLinkedListIterator method is:
%
%      ResetLinkedListIterator(LinkedListInfo *list_info)
%
%  A description of each parameter follows:
%
%    o list_info: The linked-list info.
%
*/
WizardExport void ResetLinkedListIterator(LinkedListInfo *list_info)
{
  assert(list_info != (LinkedListInfo *) NULL);
  assert(list_info->signature == WizardSignature);
  if (list_info->debug != WizardFalse)
    (void) LogWizardEvent(TraceEvent,GetWizardModule(),"...");
  LockSemaphoreInfo(list_info->semaphore);
  list_info->next=list_info->head;
  UnlockSemaphoreInfo(list_info->semaphore);
}
