/*
  Copyright 1999-2010 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.

  You may not use this file except in compliance with the License.
  obtain a copy of the License at

    http://www.wizards-toolkit.org/script/license.php

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  Wizard's toolkit xml-tree methods.
*/
#ifndef _WIZARDSTOOLKIT_XML_TREE_H
#define _WIZARDSTOOLKIT_XML_TREE_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#include <wizard/exception.h>

typedef struct _XMLTreeInfo
  XMLTreeInfo;

extern WizardExport char
  *CanonicalXMLContent(const char *,const WizardBooleanType),
  *XMLTreeInfoToXML(XMLTreeInfo *);

extern WizardExport const char
  *GetXMLTreeAttribute(XMLTreeInfo *,const char *),
  *GetXMLTreeContent(XMLTreeInfo *),
  **GetXMLTreeProcessingInstructions(XMLTreeInfo *,const char *),
  *GetXMLTreeTag(XMLTreeInfo *);

extern WizardExport XMLTreeInfo
  *AddChildToXMLTree(XMLTreeInfo *,const char *,const size_t),
  *AddPathToXMLTree(XMLTreeInfo *,const char *,const size_t),
  *DestroyXMLTree(XMLTreeInfo *),
  *GetNextXMLTreeTag(XMLTreeInfo *),
  *GetXMLTreeChild(XMLTreeInfo *,const char *),
  *GetXMLTreeOrdered(XMLTreeInfo *),
  *GetXMLTreePath(XMLTreeInfo *,const char *),
  *GetXMLTreeSibling(XMLTreeInfo *),
  *InsertTagIntoXMLTree(XMLTreeInfo *,XMLTreeInfo *,const size_t),
  *NewXMLTree(const char *,ExceptionInfo *),
  *NewXMLTreeTag(const char *),
  *PruneTagFromXMLTree(XMLTreeInfo *),
  *SetXMLTreeAttribute(XMLTreeInfo *,const char *,const char *),
  *SetXMLTreeContent(XMLTreeInfo *,const char *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
