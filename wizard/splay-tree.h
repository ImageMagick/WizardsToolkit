/*
  Copyright 1999-2011 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.

  You may not use this file except in compliance with the License.
  obtain a copy of the License at

    http://www.wizards-toolkit.org/script/license.php

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  Wizard's Toolkit splay-tree methods.
*/
#ifndef _WIZARDSTOOLKIT_SPLAY_H
#define _WIZARDSTOOLKIT_SPLAY_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

typedef struct _SplayTreeInfo
  SplayTreeInfo;

extern WizardExport const void
  *GetNextKeyInSplayTree(SplayTreeInfo *),
  *GetNextValueInSplayTree(SplayTreeInfo *),
  *GetValueFromSplayTree(SplayTreeInfo *,const void *);

extern WizardExport int
  CompareSplayTreeString(const void *,const void *),
  CompareSplayTreeStringInfo(const void *,const void *);

extern WizardExport SplayTreeInfo
  *CloneSplayTree(SplayTreeInfo *,void *(*)(void *),void *(*)(void *)),
  *DestroySplayTree(SplayTreeInfo *),
  *NewSplayTree(int (*)(const void *,const void *),void *(*)(void *),
    void *(*)(void *));

extern WizardExport size_t
  GetNumberOfNodesInSplayTree(const SplayTreeInfo *);

extern WizardExport void
  *RemoveNodeByValueFromSplayTree(SplayTreeInfo *,const void *),
  *RemoveNodeFromSplayTree(SplayTreeInfo *,const void *),
  ResetSplayTree(SplayTreeInfo *),
  ResetSplayTreeIterator(SplayTreeInfo *);

extern WizardExport WizardBooleanType
  AddValueToSplayTree(SplayTreeInfo *,const void *,const void *),
  DeleteNodeByValueFromSplayTree(SplayTreeInfo *,const void *),
  DeleteNodeFromSplayTree(SplayTreeInfo *,const void *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
