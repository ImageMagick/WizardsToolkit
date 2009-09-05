#include <stdio.h>
#include <string.h>
#include <wizard/WizardsToolkit.h>

int main(int argc, char **argv)
{
  char
    *xml;

  const char
    *acronym,
    *data_type,
    *description,
    *endian,
    *mask,
    *mime_type,
    *offset,
    *pattern,
    *priority,
    *match_type,
    *value;

  ExceptionInfo
    *exception;

  size_t
    length;
 
  static char
    *doc_type =
    {
      "<!DOCTYPE mimemap [\n"
      "  <!ELEMENT mimemap (mime+)>\n"
      "  <!ELEMENT mime (#PCDATA)>\n"
      "  <!ATTLIST mime type CDATA #REQUIRED>\n"
      "  <!ATTLIST mime acronym CDATA #IMPLIED>\n"
      "  <!ATTLIST mime description CDATA #IMPLIED>\n"
      "  <!ATTLIST mime pattern CDATA #IMPLIED>\n"
      "  <!ATTLIST mime offset CDATA #IMPLIED>\n"
      "  <!ATTLIST mime data-type (string|byte|short|long) #IMPLIED>\n"
      "  <!ATTLIST mime endian (lsb|msb) #IMPLIED>\n"
      "  <!ATTLIST mime magic CDATA #IMPLIED>\n"
      "  <!ATTLIST mime mask CDATA #IMPLIED>\n"
      "  <!ATTLIST mime priority CDATA #IMPLIED>\n"
      "]>"
    };

  XMLTreeInfo
    *comment,
    *expanded_acronym,
    *glob,
    *magic,
    *match,
    *type,
    *xml_info;

  if (argc != 2)
    return(fprintf(stderr,"usage: %s xml-file\n", argv[0]));
  exception=AcquireExceptionInfo();
  xml=(char *) FileToBlob(argv[1],~0UL,&length,exception);
  if (xml == (char *) NULL)
    return(fprintf(stderr,"%s: unable to read file\n",argv[1]));
  xml_info=NewXMLTree(xml,exception);
  xml=(char *) RelinquishWizardMemory(xml);
  if (xml_info == (XMLTreeInfo *) NULL)
    return(fprintf(stderr,"%s: unable to parse xml-file\n",argv[1]));
  (void) printf("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
  (void) printf("%s\n",doc_type);
  (void) printf("<mimemap>\n");
  type=GetXMLTreeChild(xml_info,"mime-type");
	while (type != (XMLTreeInfo *) NULL)
  {
    mime_type=GetXMLTreeAttribute(type,"type");
    acronym=(const char *) NULL;
    expanded_acronym=GetXMLTreeChild(type,"acronym");
    if (expanded_acronym != (XMLTreeInfo *) NULL)
      acronym=GetXMLTreeContent(expanded_acronym);
    expanded_acronym=GetXMLTreeChild(type,"expanded-acronym");
    description=(const char *) NULL;
    comment=GetXMLTreeChild(type,"comment");
    if (comment != (XMLTreeInfo *) NULL)
      description=GetXMLTreeContent(comment);
    if (expanded_acronym != (XMLTreeInfo *) NULL)
      description=GetXMLTreeContent(expanded_acronym);
    magic=GetXMLTreeChild(type,"magic");
    priority=(char *) NULL;
    match=(XMLTreeInfo *) NULL;
    if (magic != (XMLTreeInfo *) NULL)
      {
        priority=GetXMLTreeAttribute(magic,"priority");
        match=GetXMLTreeChild(magic,"match");
      }
  	while (match != (XMLTreeInfo *) NULL)
    {
      value=(char *) NULL;
      match_type=(char *) NULL;
      mask=(char *) NULL;
      offset=(char *) NULL;
      if (match != (XMLTreeInfo *) NULL)
        {
          value=GetXMLTreeAttribute(match,"value");
          match_type=GetXMLTreeAttribute(match,"type");
          offset=GetXMLTreeAttribute(match,"offset");
          mask=GetXMLTreeAttribute(match,"mask");
        }
      (void) printf("  <mime");
      if (mime_type != (const char *) NULL)
        (void) printf(" type=\"%s\"",mime_type);
      if (description != (const char *) NULL)
        (void) printf(" description=\"%s\"",description);
      if (match_type != (const char *) NULL)
        {
          data_type="string";
          endian="undefined";
          if (strncmp(match_type,"little",6) == 0)
            endian="LSB";
          if (strncmp(match_type,"big",3) == 0)
            endian="MSB";
          if (strcmp(match_type,"byte") == 0)
            data_type="byte";
          if (strcmp(match_type+strlen(match_type)-2,"16") == 0)
            data_type="short";
          if (strcmp(match_type+strlen(match_type)-2,"32") == 0)
            data_type="long";
          (void) printf(" data-type=\"%s\"",data_type);
          if (strcmp(endian,"undefined") != 0)
            (void) printf(" endian=\"%s\"",endian);
        }
      if (offset != (const char *) NULL)
        (void) printf(" offset=\"%s\"",offset);
      if (mask != (const char *) NULL)
        (void) printf(" mask=\"%s\"",mask);
      if (value != (const char *) NULL)
        {
          char
            *magic;

          magic=AcquireString(value);
          SubstituteString(&magic,"<","&lt;");
          SubstituteString(&magic,">","&gt;");
          SubstituteString(&magic,"\"","&quot;");
          (void) printf(" magic=\"%s\"",magic);
          magic=(char *) RelinquishWizardMemory(magic);
        }
      if (priority != (const char *) NULL)
        (void) printf(" priority=\"%s\"",priority);
      (void) printf(" />\n");
		  match=GetNextXMLTreeTag(match);
    }
    glob=GetXMLTreeChild(type,"glob");
    while (glob != (XMLTreeInfo *) NULL)
    {
      pattern=GetXMLTreeAttribute(glob,"pattern");
      value=(char *) NULL;
      if (match)
        value=GetXMLTreeAttribute(match,"value");
      (void) printf("  <mime");
      if (mime_type != (const char *) NULL)
        (void) printf(" type=\"%s\"",mime_type);
      if (acronym != (const char *) NULL)
        (void) printf(" acronym=\"%s\"",acronym);
      if (description != (const char *) NULL)
        (void) printf(" description=\"%s\"",description);
      (void) printf(" priority=\"100\"");
      if (pattern != (const char *) NULL)
        (void) printf(" pattern=\"%s\"",pattern);
      (void) printf(" />\n");
      glob=GetNextXMLTreeTag(glob);
    }
    type=GetNextXMLTreeTag(type);
  }
  (void) printf("</mimemap>\n");
  xml=XMLTreeInfoToXML(xml_info);
  (void) fprintf(stderr,"%s\n",xml);
  xml=(char *) RelinquishWizardMemory(xml);
  DestroyXMLTree(xml_info);
  exception=DestroyExceptionInfo(exception);
  return(0);
}
