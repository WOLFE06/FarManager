/*
fnparce.cpp

������ �������� ����������

*/

/* Revision: 1.06 07.12.2001 $ */

/*
Modify:
  07.12.2001 SVS
    - BugZ#149 - ���������� !& ������ ������� ������ � ���������
      � ������ ����� ������ ������� ��� � ��������.
  05.10.2001 SVS
    ! ������� ������� Panel::MakeListFile() �� panel.cpp - ����� �� �����
  25.07.2001 IS
    ! ��� !` �� ������ ������� �����.
  29.06.2001 IS
    ! ���������� ������� ��������� ������� - ������� ���, ��� ����� ��������� �
      "quote"
  22.06.2001 SVS
    ! ������� ��������� �������
  19.06.2001 SVS
    ! �������� ������ ������ ������� SubstFileName() - ��������, � ���� ��
      ������� '!' � ������.
  18.06.2001 SVS
    + Created - ������� � ��������� ������... ��� �������� :-)
*/

#include "headers.hpp"
#pragma hdrstop

#include "fn.hpp"
#include "panel.hpp"
#include "ctrlobj.hpp"
#include "cmdline.hpp"
#include "filepanels.hpp"
#include "dialog.hpp"
#include "global.hpp"
#include "lang.hpp"

static void ReplaceVariables(char *Str);

// Str=if exist !#!\!^!.! far:edit < diff -c -p "!#!\!^!.!" !\!.!
/*
  SubstFileName()
  �������������� ������������ ���������� ������ � �������� ��������

  ������� ListName � ShortListName ������� ����� ������ NM*2 !!!
*/
int SubstFileName(char *Str,            // �������������� ������
                  char *Name,           // ������� ���
                  char *ShortName,      // �������� ���
                  char *ListName,       // ������� ��� �����-������
                  char *ShortListName,  // �������� ��� �����-������
                  int   IgnoreInput,    // TRUE - �� ��������� "!?<title>?<init>!"
                  char *CmdLineDir)     // ������� ����������
{
  /* $ 19.06.2001 SVS
    ��������! ��� �������������� ������������, �� ���������� �� "!",
    ����� ����� ���� ������ ��� �������� ���� �������� ������� (���������
    ����������������!)
  */
  if(!strchr(Str,'!'))
    return FALSE;
  /* SVS $ */
  char TmpStr[8192],*CurStr,*ChPtr;
  char AnotherName[NM],AnotherShortName[NM];
  char NameOnly[NM],ShortNameOnly[NM];
  char AnotherNameOnly[NM],AnotherShortNameOnly[NM];
  char TmpName[NM+2],TmpShortName[NM+2];
  char CmdDir[NM];
  int PreserveLFN=FALSE;

_SVS(SysLog("'%s'",Str));
_SVS(SysLog(1));

  // ���� ��� �������� �������� �� ������...
  if (CmdLineDir!=NULL)
    strcpy(CmdDir,CmdLineDir);
  else // ...������� � ���.������
    CtrlObject->CmdLine->GetCurDir(CmdDir);

  *TmpStr=0; // ���� �����.

  // ���������� ����� ������-������� (���� �����)
  if (ListName!=NULL)
  {
    *ListName=0;
    ListName[NM]=0;
  }
  if (ShortListName!=NULL)
  {
    *ShortListName=0;
    ShortListName[NM]=0;
  }

  // �������������� ������� ��������� "���������" :-)
  strcpy(NameOnly,Name);
  if ((ChPtr=strrchr(NameOnly,'.'))!=NULL)
    *ChPtr=0;
  strcpy(ShortNameOnly,ShortName);
  if ((ChPtr=strrchr(ShortNameOnly,'.'))!=NULL)
    *ChPtr=0;

  Panel *AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(CtrlObject->Cp()->ActivePanel);
  AnotherPanel->GetCurName(AnotherName,AnotherShortName);
  strcpy(AnotherNameOnly,AnotherName);
  if ((ChPtr=strrchr(AnotherNameOnly,'.'))!=NULL)
    *ChPtr=0;
  strcpy(AnotherShortNameOnly,AnotherShortName);
  if ((ChPtr=strrchr(AnotherShortNameOnly,'.'))!=NULL)
    *ChPtr=0;

  char *DirBegin=NULL; // ������ ����� �������� (!?)
  int PassivePanel=FALSE; // ������������� ���� ���� ��� �������� ������!
_SVS(int Pass=1);

  CurStr=Str;
  while (*CurStr)
  {
_SVS(SysLog("***** Pass=%d",Pass));

    // ���������� ������������� ����������/����������� ������.
    if (strncmp(CurStr,"!#",2)==0)
    {
      CurStr+=2;
      PassivePanel=TRUE;
_SVS(SysLog("PassivePanel=TRUE '%s'",CurStr));
    }
    if (strncmp(CurStr,"!^",2)==0)
    {
      CurStr+=2;
      PassivePanel=FALSE;
_SVS(SysLog("PassivePanel=FALSE '%s'",CurStr));
    }

    {
      // !! ������ '!'
      if (strncmp(CurStr,"!!",2)==0 && CurStr[2] != '?')
      {
        strcat(TmpStr,"!");
        CurStr+=2;
_SVS(SysLog("!! TmpStr=[%s]",TmpStr));
        continue;
      }

      // !.!      ������� ��� ����� � �����������
      if (strncmp(CurStr,"!.!",3)==0 && CurStr[3] != '?')
      {
        strcat(TmpStr,PassivePanel ? AnotherName:Name);
        CurStr+=3;
_SVS(SysLog("!.! TmpStr=[%s]",TmpStr));
        continue;
      }

      // !~       �������� ��� ����� ��� ����������
      if (strncmp(CurStr,"!~",2)==0)
      {
        strcat(TmpStr,PassivePanel ? AnotherShortNameOnly:ShortNameOnly);
        CurStr+=2;
_SVS(SysLog("!~ TmpStr=[%s]",TmpStr));
        continue;
      }

      // !`  ������� ���������� ����� ��� �����
      if (strncmp(CurStr,"!`",2)==0)
      {
        char *Ext;
        if(CurStr[2] == '~')
        {
          Ext=strrchr((PassivePanel ? AnotherShortName:ShortName),'.');
          CurStr+=3;
        }
        else
        {
          Ext=strrchr((PassivePanel ? AnotherName:Name),'.');
          CurStr+=2;
        }
        if(Ext && *Ext)
          strcat(TmpStr,++Ext);
_SVS(SysLog("!` TmpStr=[%s]",TmpStr));
        continue;
      }

      // !\!.!  ������ ��� �����
      if (strncmp(CurStr,"!\\!.!",5)==0 && CurStr[5] != '?')
      {
        char CurDir[NM];
        char *FileName=PassivePanel ? AnotherName:Name;
        if (strpbrk(FileName,"\\:")==NULL)
        {
          if (PassivePanel)
            AnotherPanel->GetCurDir(CurDir);
          else
            strcpy(CurDir,CmdDir);
          AddEndSlash(CurDir);
        }
        else
          *CurDir=0;
        strcat(CurDir,FileName);
        CurStr+=5;
        if(!DirBegin) DirBegin=TmpStr+strlen(TmpStr);
        strcat(TmpStr,CurDir);
_SVS(SysLog("!\\!.! TmpStr=[%s]",TmpStr));
        continue;
      }

      // !& !&~  ������ ������ ����������� ��������.
      if (!strncmp(CurStr,"!&~",3) && CurStr[3] != '?' ||
          !strncmp(CurStr,"!&",2) && CurStr[2] != '?')
      {
        char FileNameL[NM],ShortNameL[NM];
        Panel *WPanel=PassivePanel?AnotherPanel:CtrlObject->Cp()->ActivePanel;
        int FileAttrL;
        int ShortN0=FALSE;
        int CntSkip=2;
        if(CurStr[2] == '~')
        {
          ShortN0=TRUE;
          CntSkip++;
        }
        WPanel->GetSelName(NULL,FileAttrL);
        while (WPanel->GetSelName(FileNameL,FileAttrL,ShortNameL))
        {
          if (ShortN0)
            strcpy(FileNameL,ShortNameL);
          else // � ������ ��� �� ������ ������� ��� � ��������.
            QuoteSpaceOnly(FileNameL);
// ��� ����� ��� ��� ����� - �����/�������...
//   ���� ����� ����� - ��������������� :-)
//          if(FileAttrL & FA_DIREC)
//            AddEndSlash(FileNameL);
          strcat(TmpStr," ");
          strcat(TmpStr,FileNameL);
        }
        CurStr+=CntSkip;
_SVS(SysLog("!& TmpStr=[%s]",TmpStr));
        continue;
      }

      // !@  ��� �����, ����������� ����� ���������� ������
      if (strncmp(CurStr,"!@",2)==0 && ListName!=NULL)
      {
        char Modifers[32]="", *Ptr;

        if((Ptr=strchr(CurStr+2,'!')) != NULL)
        {
          if(Ptr[1] != '?')
          {
            *Ptr=0;
            strncpy(Modifers,CurStr+2,sizeof(Modifers)-1);
            /* $ 02.09.2000 tran
               !@!, !#!@! bug */
            if ( PassivePanel && ( ListName[NM] || AnotherPanel->MakeListFile(ListName+NM,FALSE,Modifers)))
            {
              strcat(TmpStr,ListName+NM);
            }
            if ( !PassivePanel && (*ListName || CtrlObject->Cp()->ActivePanel->MakeListFile(ListName,FALSE,Modifers)))
            {
              strcat(TmpStr,ListName);
            }
            /* tran $ */
            CurStr+=Ptr-CurStr+1;
            continue;
          }
        }
      }

      // !$!      ��� �����, ����������� �������� ����� ���������� ������
      if (strncmp(CurStr,"!$",2)==0 && ShortListName!=NULL)
      {
        char Modifers[32]="", *Ptr;

        if((Ptr=strchr(CurStr+2,'!')) != NULL)
        {
          if(Ptr[1] != '?')
          {
            *Ptr=0;
            strncpy(Modifers,CurStr+2,sizeof(Modifers)-1);
            /* $ 02.09.2000 tran
               !@!, !#!@! bug */
            if ( PassivePanel && (ShortListName[NM] || AnotherPanel->MakeListFile(ShortListName+NM,TRUE,Modifers)))
            {
              /* $ 01.11.2000 IS
                 ��� ����� � ������ ������ ������ ���� ��������
              */
              ConvertNameToShort(ShortListName+NM,ShortListName+NM);
              /* IS $ */
              strcat(TmpStr,ShortListName+NM);
            }
            if ( !PassivePanel && (*ShortListName || CtrlObject->Cp()->ActivePanel->MakeListFile(ShortListName,TRUE,Modifers)))
            {
              /* $ 01.11.2000 IS
                 ��� ����� � ������ ������ ������ ���� ��������
              */
              ConvertNameToShort(ShortListName,ShortListName);
              /* IS $ */
              strcat(TmpStr,ShortListName);
            }
            /* tran $ */
            CurStr+=Ptr-CurStr+1;
_SVS(SysLog("!$! TmpStr=[%s]",TmpStr));
            continue;
          }
        }
      }

      // !-!      �������� ��� ����� � �����������
      if (strncmp(CurStr,"!-!",3)==0 && CurStr[3] != '?')
      {
        strcat(TmpStr,PassivePanel ? AnotherShortName:ShortName);
        CurStr+=3;
_SVS(SysLog("!-! TmpStr=[%s]",TmpStr));
        continue;
      }

      // !+!      ���������� !-!, �� ���� ������� ��� ����� �������
      //          ����� ���������� �������, FAR ����������� ���
      if (strncmp(CurStr,"!+!",3)==0 && CurStr[3] != '?')
      {
        strcat(TmpStr,PassivePanel ? AnotherShortName:ShortName);
        CurStr+=3;
        PreserveLFN=TRUE;
_SVS(SysLog("!+! TmpStr=[%s]",TmpStr));
        continue;
      }

      // !:       ������� ����
      if (strncmp(CurStr,"!:",2)==0)
      {
        char CurDir[NM];
        if (*Name && Name[1]==':')
          strcpy(CurDir,Name);
        else
          if (PassivePanel)
            AnotherPanel->GetCurDir(CurDir);
          else
            strcpy(CurDir,CmdDir);
        CurDir[2]=0;
        if (*CurDir && CurDir[1]!=':')
          *CurDir=0;
        if(!DirBegin) DirBegin=TmpStr+strlen(TmpStr);
        strcat(TmpStr,CurDir);
        CurStr+=2;
_SVS(SysLog("!: TmpStr=[%s]",TmpStr));
        continue;
      }

      // !\       ������� ����
      if (strncmp(CurStr,"!\\",2)==0)
      {
        char CurDir[NM];
        if (PassivePanel)
          AnotherPanel->GetCurDir(CurDir);
        else
          strcpy(CurDir,CmdDir);
        AddEndSlash(CurDir);
        CurStr+=2;
        if (*CurStr=='!')
        {
          strcpy(TmpName,Name);
          strcpy(TmpShortName,ShortName);
          if (strpbrk(Name,"\\:")!=NULL)
            *CurDir=0;
        }
        if(!DirBegin) DirBegin=TmpStr+strlen(TmpStr);
        strcat(TmpStr,CurDir);
_SVS(SysLog("!\\ TmpStr=[%s] CurDir=[%s]",TmpStr, CurDir));
        continue;
      }

      // !/       �������� ��� �������� ����
      if (strncmp(CurStr,"!/",2)==0)
      {
        char CurDir[NM];
        if (PassivePanel)
          AnotherPanel->GetCurDir(CurDir);
        else
          strcpy(CurDir,CmdDir);
        ConvertNameToShort(CurDir,CurDir);
        AddEndSlash(CurDir);
        CurStr+=2;
        if (*CurStr=='!')
        {
          strcpy(TmpName,Name);
          strcpy(TmpShortName,ShortName);
          if (strpbrk(Name,"\\:")!=NULL)
          {
            if (PointToName(ShortName)==ShortName)
            {
              strcpy(TmpShortName,CurDir);
              AddEndSlash(TmpShortName);
              strcat(TmpShortName,ShortName);
            }
            *CurDir=0;
          }
        }
        if(!DirBegin) DirBegin=TmpStr+strlen(TmpStr);
        strcat(TmpStr,CurDir);
_SVS(SysLog("!/ TmpStr=[%s]",TmpStr));
        continue;
      }

      // !?<title>?<init>!
      if (strncmp(CurStr,"!?",2)==0 && strchr(CurStr+2,'!')!=NULL)
      {
        char *NewCurStr=strchr(CurStr+2,'!')+1;
        strncat(TmpStr,CurStr,NewCurStr-CurStr);
        CurStr=NewCurStr;
_SVS(SysLog("!? TmpStr=[%s]",TmpStr));
        continue;
      }

      // !        ������� ��� ����� ��� ����������
      if (*CurStr=='!')
      {
        if(!DirBegin) DirBegin=TmpStr+strlen(TmpStr);
        strcat(TmpStr,PointToName(PassivePanel ? AnotherNameOnly:NameOnly));
        CurStr++;
_SVS(SysLog("! TmpStr=[%s]",TmpStr));
        continue;
      }

    }

    strncat(TmpStr,CurStr,1);
    CurStr++;
_SVS(++Pass);
  }

  if (!IgnoreInput)
    ReplaceVariables(TmpStr);
  strcpy(Str,TmpStr);

_SVS(SysLog(-1));
_SVS(SysLog("[%s]\n",Str));
  return(PreserveLFN);
}


void ReplaceVariables(char *Str)
{
  const int MaxSize=20;
  char *StartStr=Str;

  if (*Str=='\"')
    while (*Str && *Str!='\"')
      Str++;

  struct DialogItem *DlgData=NULL;
  int DlgSize=0;
  int StrPos[64],StrPosSize=0;

  while (*Str && DlgSize<MaxSize)
  {
    if (*(Str++)!='!')
      continue;
    if (*(Str++)!='?')
      continue;
    if (strchr(Str,'!')==NULL)
      return;
    StrPos[StrPosSize++]=Str-StartStr-2;
    DlgData=(struct DialogItem *)realloc(DlgData,(DlgSize+2)*sizeof(*DlgData));
    memset(&DlgData[DlgSize],0,2*sizeof(*DlgData));
    DlgData[DlgSize].Type=DI_TEXT;
    DlgData[DlgSize].X1=5;
    DlgData[DlgSize].Y1=DlgSize+2;
    DlgData[DlgSize+1].Type=DI_EDIT;
    DlgData[DlgSize+1].X1=5;
    DlgData[DlgSize+1].X2=70;
    DlgData[DlgSize+1].Y1=DlgSize+3;
    DlgData[DlgSize+1].Flags|=DIF_HISTORY|DIF_USELASTHISTORY;

    char HistoryName[MaxSize][20];
    int HistoryNumber=DlgSize/2;
    sprintf(HistoryName[HistoryNumber],"UserVar%d",HistoryNumber);
    /* $ 01.08.2000 SVS
       + .History
    */
    DlgData[DlgSize+1].Selected=(int)HistoryName[HistoryNumber];
    /* SVS $*/

    if (DlgSize==0)
    {
      DlgData[DlgSize+1].Focus=1;
      DlgData[DlgSize+1].DefaultButton=1;
    }
    char Title[256];
    strcpy(Title,Str);
    *strchr(Title,'!')=0;
    Str+=strlen(Title)+1;
    char *SrcText=strchr(Title,'?');
    if (SrcText!=NULL)
    {
      *SrcText=0;
      strcpy(DlgData[DlgSize+1].Data,SrcText+1);
    }
    strcpy(DlgData[DlgSize].Data,Title);
    /* $ 01.08.2000 SVS
       "���������" ���������
    */
    ExpandEnvironmentStr(DlgData[DlgSize].Data,DlgData[DlgSize].Data,sizeof(DlgData[DlgSize].Data));
    /* SVS $*/
    DlgSize+=2;
  }
  if (DlgSize==0)
    return;
  DlgData=(struct DialogItem *)realloc(DlgData,(DlgSize+1)*sizeof(*DlgData));
  memset(&DlgData[DlgSize],0,sizeof(*DlgData));
  DlgData[DlgSize].Type=DI_DOUBLEBOX;
  DlgData[DlgSize].X1=3;
  DlgData[DlgSize].Y1=1;
  DlgData[DlgSize].X2=72;
  DlgData[DlgSize].Y2=DlgSize+2;
  DlgSize++;
  Dialog Dlg(DlgData,DlgSize);
  Dlg.SetPosition(-1,-1,76,DlgSize+3);
  Dlg.Process();
  if (Dlg.GetExitCode()==-1)
  {
    /* $ 13.07.2000 SVS
       ������ ��� �� realloc
    */
    free(DlgData);
    /* SVS $ */
    *StartStr=0;
    return;
  }
  char TmpStr[4096];
  *TmpStr=0;
  for (Str=StartStr;*Str!=0;Str++)
  {
    int Replace=-1;
    for (int I=0;I<StrPosSize;I++)
      if (Str-StartStr==StrPos[I])
      {
        Replace=I;
        break;
      }
    if (Replace!=-1)
    {
      strcat(TmpStr,DlgData[Replace*2+1].Data);
      Str=strchr(Str+1,'!');
    }
    else
      strncat(TmpStr,Str,1);
  }
  strcpy(StartStr,TmpStr);
  /* $ 01.08.2000 SVS
     ����� "��������" Enter "���������" ������
  */
  ExpandEnvironmentStr(TmpStr,StartStr,sizeof(DlgData[0].Data));
  /* SVS $ */
  /* $ 13.07.2000 SVS
     ������ ��� �� realloc
  */
  free(DlgData);
  /* SVS $ */
}

int Panel::MakeListFile(char *ListFileName,int ShortNames,char *Modifers)
{
  FILE *ListFile;

  strcpy(ListFileName,Opt.TempPath);
  strcat(ListFileName,FarTmpXXXXXX);
  if (mktemp(ListFileName)==NULL || (ListFile=fopen(ListFileName,"wb"))==NULL)
  {
    Message(MSG_WARNING,1,MSG(MError),MSG(MCannotCreateListFile),MSG(MCannotCreateListTemp),MSG(MOk));
    return(FALSE);
  }

  char FileName[NM*2],ShortName[NM];
  int FileAttr;
  GetSelName(NULL,FileAttr);
  while (GetSelName(FileName,FileAttr,ShortName))
  {
    if (ShortNames)
      strcpy(FileName,ShortName);

    if(Modifers && *Modifers)
    {
      if(strchr(Modifers,'F')) // 'F' - ������������ ������ ����;
      {
        char TempFileName[NM*2];
        strcpy(TempFileName,CurDir);
        sprintf(TempFileName,"%s%s%s",CurDir,(CurDir[strlen(CurDir)-1] != '\\'?"\\":""),FileName);
        if (ShortNames)
          ConvertNameToShort(TempFileName,TempFileName);
        strcpy(FileName,TempFileName);
      }
      if(strchr(Modifers,'Q')) // 'Q' - ��������� ����� � ��������� � �������;
        QuoteSpaceOnly(FileName);
      if(strchr(Modifers,'A')) // 'A' - ������������ ANSI ���������.
        OemToChar(FileName,FileName);

      if(strchr(Modifers,'S')) // 'S' - ������������ '/' ������ '\' � ����� ������;
      {
        int I,Len=strlen(FileName);
        for(I=0; I < Len; ++I)
          if(FileName[I] == '\\')
            FileName[I]='/';
      }
    }
//_D(SysLog("%s[%s] %s",__FILE__,Modifers,FileName));
    if (fprintf(ListFile,"%s\r\n",FileName)==EOF)
    {
      fclose(ListFile);
      remove(ListFileName);
      Message(MSG_WARNING,1,MSG(MError),MSG(MCannotCreateListFile),MSG(MCannotCreateListWrite),MSG(MOk));
      return(FALSE);
    }
  }
  if (fclose(ListFile)==EOF)
  {
    clearerr(ListFile);
    fclose(ListFile);
    remove(ListFileName);
    Message(MSG_WARNING,1,MSG(MError),MSG(MCannotCreateListFile),MSG(MOk));
    return(FALSE);
  }
  return(TRUE);
}
