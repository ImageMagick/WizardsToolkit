#include <stdio.h>
#include <wizard/WizardsToolkit.h>

int main(int argc,char **argv)
{
  HashInfo
    *hash_info;

  StringInfo
    *content;

  WizardsToolkitGenesis(*argv);
  hash_info=AcquireHashInfo(SHA2512Hash);
  InitializeHash(hash_info);
  content=StringToStringInfo("The Wizard's Toolkit");
  UpdateHash(hash_info,content);
  FinalizeHash(hash_info);
  PrintStringInfo("Content",content);
  PrintStringInfo("Digest",GetHashDigest(hash_info));
  DestroyHashInfo(hash_info);
  DestroyStringInfo(content);
  WizardsToolkitTerminus();
  return(0);
}
