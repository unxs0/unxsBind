/*
FILE 
	glossary.c
	svn ID removed
AUTHOR
	(C) 2006-2009 Gary Wallis and Hugo Urquiza for Unixservice
PURPOSE
	idnsOrg Interface
	program file.
*/

#include "interface.h"

static char cLabel[33]={""};
static char cText[16384]={""};

//
//Local only
void SelectGlossary(char *cLabel);

void GlossaryGetHook(entry gentries[],int x)
{
	register int i;
	
	for(i=0;i<x;i++)
	{
		if(!strcmp(gentries[i].name,"cLabel"))
			sprintf(cLabel,"%.32s",gentries[i].val);
	}

	if(cLabel[0])
	{
		MYSQL_RES *res;
		MYSQL_ROW field;
		
		SelectGlossary(cLabel);
		res=mysql_store_result(&gMysql);
		
		if((field=mysql_fetch_row(res)))
			sprintf(cText,"%.16383s",field[0]);		
		else
			sprintf(cText,"No description available");

		mysql_free_result(res);
	}
		
	htmlGlossary();
	

}//void GlossaryGetHook(entry gentries[],int x)


void htmlGlossary(void)
{
	htmlHeader("idnsOrg","Header");
	htmlGlossaryPage("idnsOrg","OrgGlossary.Body");
	htmlFooter("Footer");

}//void htmlGlossary(void)


void htmlGlossaryPage(char *cTitle, char *cTemplateName)
{
	if(cTemplateName[0])
	{
        	MYSQL_RES *res;
	        MYSQL_ROW field;

		TemplateSelectInterface(cTemplateName,uPLAINSET,uIDNSORGTYPE);
		res=mysql_store_result(&gMysql);
		if((field=mysql_fetch_row(res)))
		{
			struct t_template template;
			
			template.cpName[0]="cTitle";
			template.cpValue[0]=cTitle;
			
			template.cpName[1]="cCGI";
			template.cpValue[1]="idnsAdmin.cgi";
			
			template.cpName[2]="gcLogin";
			template.cpValue[2]=gcLogin;

			template.cpName[3]="gcName";
			template.cpValue[3]=gcName;

			template.cpName[4]="gcOrgName";
			template.cpValue[4]=gcOrgName;

			template.cpName[5]="cUserLevel";
			template.cpValue[5]=(char *)cUserLevel(guPermLevel);

			template.cpName[6]="gcHost";
			template.cpValue[6]=gcHost;

			template.cpName[7]="gcMessage";
			template.cpValue[7]=gcMessage;

			template.cpName[8]="cLabel";
			template.cpValue[8]=cLabel;

			template.cpName[9]="cText";
			template.cpValue[9]=cText;

			template.cpName[10]="";
			
			printf("\n<!-- Start htmlGlossaryPage(%s) -->\n",cTemplateName); 
			Template(field[0], &template, stdout);
			printf("\n<!-- End htmlGlossaryPage(%s) -->\n",cTemplateName); 
		}
		else
		{
			printf("<hr>");
			printf("<center><font size=1>%s</font>\n",cTemplateName);
		}
		mysql_free_result(res);
	}

}//void htmlGlossaryPage()


void SelectGlossary(char *cLabel)
{
	sprintf(gcQuery,"SELECT cText FROM tGlossary WHERE cLabel='%s'",cLabel);
	mysql_query(&gMysql,gcQuery);

	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
	
}//void SelectGlossary(char *cLabel)

