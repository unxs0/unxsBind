/*
FILE
	svn ID removed
PURPOSE
	Non-schema dependent tresource.c expansion.
AUTHOR
	GPL License applies, see www.fsf.org for details
	See LICENSE file in this distribution
	(C) 2001-2009 Gary Wallis for Unixservice.
*/

//File scope vars
static char cSearch[100]={""};
static char cuNSSetPullDown[256]={""};
static unsigned uSelectNSSet=0;
static unsigned uCIDR=0;
static unsigned uStartBlock=0;
static unsigned uZoneOwner=0;
static unsigned uSubmitJob=0;

//Aux drop/pull downs
void CustomerDropDown(unsigned uSelector);//tzonefunc.h
static char cCustomerDropDown[256]={""};
static unsigned uClient=0;
static unsigned uGroup=0;

//Called from tzonefunc.h
void tResourceTableAddRR(unsigned uZoneId);

static char cZone[255]={""};
static unsigned uAddArpaPTR=0;

int AutoAddPTRResource(const unsigned d,const char *cDomain,const unsigned uInZone,const unsigned uSourceZoneOwner);
int AutoAddPTRResourceIPv6(const char *cIPv6PTR,const char *cDomain,const unsigned uInZone,const unsigned uSourceZoneOwner);

unsigned IsSecondaryOnlyZone(unsigned uZone);

void UpdateSerialNum(unsigned uZone);//tzonefunc.h
void PrepareTestData(void);

/*
void DebugOnly(const char *cLabel1, const char *cValue1, const char *cLabel2, const char *cValue2)
{
	printf("Content-type: text/plain\n\n");
	printf("%s=(%s) %s=(%s)\n",cLabel1,cValue1,cLabel2,cValue2);
	exit(0);
}//DebugOnly()
*/

/*
unsigned guGrpDelete=0;
*/

void ExtProcesstResourceVars(pentry entries[], int x)
{

	register int i;
	
	for(i=0;i<x;i++)
	{
		if(!strcmp(entries[i].name,"uAddArpaPTR"))
			uAddArpaPTR=1;	
		else if(!strcmp(entries[i].name,"cSearch"))
			sprintf(cSearch,"%.99s",entries[i].val);
		else if(!strcmp(entries[i].name,"cuNSSetPullDown"))
		{
			sprintf(cuNSSetPullDown,"%.255s",entries[i].val);
			uSelectNSSet=ReadPullDown("tNSSet","cLabel",cuNSSetPullDown);
		}
		else if(!strcmp(entries[i].name,"uCIDR"))
			sscanf(entries[i].val,"%u",&uCIDR);
		else if(!strcmp(entries[i].name,"uStartBlock"))
		{
			sscanf(entries[i].val,"%u",&uStartBlock);
			if(uStartBlock>249) uStartBlock=249;
			else if(uStartBlock<0) uStartBlock=0;
		}
		else if(!strcmp(entries[i].name,"cCustomerDropDown"))
		{
			strcpy(cCustomerDropDown,entries[i].val);
			uClient=ReadPullDown("tClient","cLabel",
					cCustomerDropDown);
		}
		else if(!strcmp(entries[i].name,"uSubmitJobNoCA"))
			uSubmitJob=1;			
	}//for all name value pairs


}//void ExtProcesstResourceVars(pentry entries[], int x)


void RRCheck(int uMode)
{
	char cRRType[100]={""};
	unsigned a=0,b=0,c=0,d=0;
			
	strcpy(cRRType,ForeignKey("tRRType","cLabel",uRRType));
	strcpy(cZone,ForeignKey("tZone","cZone",uZone));
	
	//For all TODO
	if(!cName[0] && strcmp(cRRType,"NS"))
	{
		guMode=uMode;
		tResource("cName is required");
	}


	//TODO
	if(cName[strlen(cName)-1]=='.')
	{
		//Another bug may allow zones to be added with trailing dot.
		if(cZone[strlen(cZone)-1]=='.')
			sprintf(gcQuery,"%s",cZone);
		else
			sprintf(gcQuery,"%s.",cZone);
		if(strcmp(gcQuery,cName))
		{
			//Another bug may allow zones to be added with trailing dot. See above
			//We are only adding a prefix dot.
			if(cZone[strlen(cZone)-1]=='.')
			{
				sprintf(gcQuery,".%s",cZone);
			}
			else
			{
				sprintf(gcQuery,".%s.",cZone);
			}

			if(!strstr(cName+(strlen(cName)-strlen(gcQuery)),gcQuery))
			{

				guMode=uMode;
				tResource("If cName is fully qualified it must end with cZone and final period.");
			}
		}
	}

	//More cName validation. Do not allow .. or .- or -. in cName
	if(strstr(cName,".."))
	{
		guMode=uMode;
		tResource("cName can't contain '..' (two or more consecutive periods.)");
	}
	if(strstr(cName,".-"))
	{
		guMode=uMode;
		tResource("cName can't contain '.-'.");
	}
	if(strstr(cName,"-."))
	{
		guMode=uMode;
		tResource("cName can't contain '-.'.");
	}
	//More cName validation. If @ present it must be only char.
	if(strchr(cName,'@') && strlen(cName)>1)
	{
		guMode=uMode;
		tResource("If @ is used it must be the only char");
	}


	//TODO
	//Make sure only one cName exists with same value per zone...

		
	if(!strcmp(cRRType,"CNAME"))
	{
		//TODO get docs on valid use and default helper use of this RR
		//(cName) LHS should have no dot '.' or end with zone name. This is done for all
		//zone RRs.
		//(cParam1) RHS can be any FQDN. If with trailing dot ok. If not add zone with dot.
		//All cases
		//cParam2 not used. Erased.
		cParam2[0]=0;

		guMode=uMode;
		if(!cParam1[0])
		{
				sprintf(cParam1,"%s.",cZone);
				tResource("cParam1 is required. Common default entry made for you, check/change.");
		}
		else
		{
			char cParam1Save[101]={""};
			char cParam1Temp[101]={""};

			sprintf(cParam1Save,"%.100s",cParam1);
			//converts, eliminates illegal chars.
			//helpers try to format for you...lol
			FQDomainName(cParam1);

			if(strchr(cParam1,'.'))
			{
				//FQDN
				//Missing trailing dot
				if(cParam1[strlen(cParam1)-1]!='.')
				{
					sprintf(cParam1Temp,"%.100s.",cParam1);
					strcpy(cParam1,cParam1Temp);
				}
				if(strcmp(cParam1,cParam1Save))
					tResource("cParam1 was changed check/fix");
			}
			else
			{
				//Not FQDN, but internal zone member or another zone?

				//TODO
				//Advanced help not implemented yet.
				//Check in this zone for A record. Should not be another CNAME.
				//Check in tZone

				if(cParam1[strlen(cParam1)-1]!='.')
				{
					sprintf(cParam1Temp,"%.49s.%.49s.",cParam1,cZone);
					strcpy(cParam1,cParam1Temp);
				}
				if(strcmp(cParam1,cParam1Save))
					tResource("cParam1 was changed check/fix");
			}

		}
	}
	else if(!strcmp(cRRType,"A"))
	{

		if(!strcmp(cZone+strlen(cZone)-5,".arpa"))
			tResource("Can not add A records to arpa zones");
		sscanf(cParam1,"%u.%u.%u.%u",&a,&b,&c,&d);
		if(a>255) a=0;
		if(b>255) b=0;
		if(c>255) c=0;  
		if(d>255) d=0;  

		sprintf(cParam1,"%u.%u.%u.%u",a,b,c,d);

		cParam2[0]=0;
		//we now allow for .0 IP numbers
		if(!a)
		{
			guMode=uMode;
			tResource("Invalid IP Number for cParam1");
		}
		if(strchr(cName,'_'))
		{
			guMode=uMode;
			tResource("A RR cName/Hostname can't contain '_'");
		}
	}
	else if(!strcmp(cRRType,"AAAA"))
	{
		register int i;
		unsigned h1=0;
		unsigned h2=0;
		unsigned h3=0;
		unsigned h4=0;
		unsigned h5=0;
		unsigned h6=0;
		unsigned h7=0;
		unsigned h8=0;
		char *cp;
		unsigned uColonCount=0;
		unsigned uRead=0;

		//Insure these are empty
		cParam2[0]=0;

		if(strchr(cName,'_'))
		{
			guMode=uMode;
			tResource("AAAA RR cName/Hostname can't contain '_'");
		}

		if(!strcmp(cZone+strlen(cZone)-5,"arpa"))
			tResource("Can not add AAAA records to arpa zones");

		if(strlen(cParam1)<4)
		{
			guMode=uMode;
			tResource("<blink>IPv6 number must be at least 4 chars long (e.g. 1::a)</blink>");
		}

		//if cParam1 has no consecutive colons we can simply:
		if((cp=strstr(cParam1,"::")))
		{
			if(strstr(cp+2,"::"))
			{
				guMode=uMode;
				tResource("<blink>IPv6 number can not have more than one double colon!</blink>");
			}
		}

		//Now for the hard work
		for(i=0;cParam1[i];i++)
		{
			if(cParam1[i]==':')
				uColonCount++;
			if(cParam1[i]!=':' && !isxdigit(cParam1[i]))
			{
				guMode=uMode;
				tResource("<blink>IPv6 number can only have hexadecimal digits and colons!</blink>");
			}
		}

		switch(uColonCount)
		{
			case 0:
			case 1:
				guMode=uMode;
				tResource("<blink>IPv6 too few colons: Min is 2!</blink>");
			break;

			case 2:
				uRead=sscanf(cParam1,"%x::%x",&h1,&h8);
				if(uRead!=2)
				{
					guMode=uMode;
					tResource("<blink>IPv6 format-2 error!</blink>");
				}
			break;

			case 3:
				uRead=sscanf(cParam1,"%x::%x:%x",&h1,&h7,&h8);
				if(uRead!=3)
				{
					uRead=sscanf(cParam1,"%x:%x::%x",&h1,&h2,&h8);
					if(uRead!=3)
					{
						guMode=uMode;
						tResource("<blink>IPv6 format-3 error!</blink>");
					}
				}
			break;

			case 4:
				uRead=sscanf(cParam1,"%x::%x:%x:%x",&h1,&h6,&h7,&h8);
				if(uRead!=4)
				{
					uRead=sscanf(cParam1,"%x:%x::%x:%x",&h1,&h2,&h7,&h8);
					if(uRead!=4)
					{
						uRead=sscanf(cParam1,"%x:%x:%x::%x",&h1,&h2,&h3,&h8);
						if(uRead!=4)
						{
							guMode=uMode;
							tResource("<blink>IPv6 format-4 error!</blink>");
						}
					}
				}
			break;

			case 5:
				uRead=sscanf(cParam1,"%x::%x:%x:%x:%x",&h1,&h5,&h6,&h7,&h8);
				if(uRead!=5)
				{
					uRead=sscanf(cParam1,"%x:%x::%x:%x:%x",&h1,&h2,&h6,&h7,&h8);
					if(uRead!=5)
					{
						uRead=sscanf(cParam1,"%x:%x:%x::%x:%x",&h1,&h2,&h3,&h7,&h8);
						if(uRead!=5)
						{
							uRead=sscanf(cParam1,"%x:%x:%x:%x::%x",&h1,&h2,&h3,&h4,&h8);
							if(uRead!=5)
							{
								guMode=uMode;
								tResource("<blink>IPv6 format-5 error!</blink>");
							}
						}
					}
				}
			break;

			case 6:
				uRead=sscanf(cParam1,"%x::%x:%x:%x:%x:%x",&h1,&h4,&h5,&h6,&h7,&h8);
				if(uRead!=6)
				{
					uRead=sscanf(cParam1,"%x:%x::%x:%x:%x:%x",&h1,&h2,&h5,&h6,&h7,&h8);
					if(uRead!=6)
					{
						uRead=sscanf(cParam1,"%x:%x:%x::%x:%x:%x",&h1,&h2,&h3,&h6,&h7,&h8);
						if(uRead!=6)
						{
							uRead=sscanf(cParam1,"%x:%x:%x:%x::%x:%x",&h1,&h2,&h3,&h4,&h7,&h8);
							if(uRead!=6)
							{
								uRead=sscanf(cParam1,"%x:%x:%x:%x:%x::%x",
											&h1,&h2,&h3,&h4,&h5,&h8);
								if(uRead!=6)
								{
									guMode=uMode;
									tResource("<blink>IPv6 format-6 error!</blink>");
								}
							}
						}
					}
				}
			break;

			case 7:
				uRead=sscanf(cParam1,"%x:%x:%x:%x:%x:%x:%x:%x",&h1,&h2,&h3,&h4,&h5,&h6,&h7,&h8);
				if(uRead!=8)
				{
					guMode=uMode;
					tResource("<blink>IPv6 format-7 error!</blink>");
				}
			break;

			default:
				guMode=uMode;
				tResource("<blink>IPv6 too many colons: Max is 7!</blink>");
			
		}

		//First basic checks for AAAA hosts
		if(!h1)
		{
			guMode=uMode;
			tResource("IPv6 number can not have a 0 in first 16 bit hex word.");
		}

		if(!h8)
		{
			guMode=uMode;
			sprintf(gcQuery," IPv6 number can not have a 0 in last 16 bit hex word:"
					" %x:%x:%x:%x:%x:%x:%x:%x",h1,h2,h3,h4,h5,h6,h7,h8);
			tResource(gcQuery);
		}

		//Mandatory rewrite in shortest possible IPv6 format.
		//Should follow RFC 5952 canonical format
		//This is needed to speed up DNSSEC and reduce BIND zone file size.
		//This may not be a good idea. Need to research further: If someone wants to
		//write a bunch of 0's why not?
		//Compress empty words: Double colon. Can only be used once.
		//Trying KISS method here. sprintf does the leading 0 removal for us.
		//6 consecutive 0 case
		if(!h2 && !h3 && !h4 && !h5 && !h6 && !h7)
			sprintf(cParam1,"%x::%x",h1,h8);
		//5 consecutive 0 cases
		else if(!h2 && !h3 && !h4 && !h5 && !h6)
			sprintf(cParam1,"%x::%x:%x",h1,h7,h8);
		else if(!h3 && !h4 && !h5 && !h6 && !h7)
			sprintf(cParam1,"%x:%x::%x",h1,h2,h8);
		//4 consecutive 0 cases
		else if(!h2 && !h3 && !h4 && !h5)
			sprintf(cParam1,"%x::%x:%x:%x",h1, h6,h7,h8);
		else if(!h3 && !h4 && !h5 && !h6)
			sprintf(cParam1,"%x:%x::%x:%x",h1,h2, h7,h8);
		else if(!h4 && !h5 && !h6 && !h7)
			sprintf(cParam1,"%x:%x:%x::%x",h1,h2,h3, h8);
		//3 consecutive 0 cases
		else if(!h2 && !h3 && !h4)
			sprintf(cParam1,"%x::%x:%x:%x:%x",h1, h5,h6,h7,h8);
		else if(!h3 && !h4 && !h5)
			sprintf(cParam1,"%x:%x::%x:%x:%x",h1,h2, h6,h7,h8);
		else if(!h4 && !h5 && !h6)
			sprintf(cParam1,"%x:%x:%x::%x:%x",h1,h2,h3, h7,h8);
		else if(!h5 && !h6 && !h7)
			sprintf(cParam1,"%x:%x:%x:%x::%x",h1,h2,h3,h4, h8);
		//2 consecutive 0 cases
		//RFC 5952 issue spotted here
		else if(!h2 && !h3)
			sprintf(cParam1,"%x::%x:%x:%x:%x:%x",h1, h4,h5,h6,h7,h8);
		else if(!h3 && !h4)
			sprintf(cParam1,"%x:%x::%x:%x:%x:%x",h1,h2, h5,h6,h7,h8);
		else if(!h4 && !h5)
			sprintf(cParam1,"%x:%x:%x::%x:%x:%x",h1,h2,h3, h6,h7,h8);
		else if(!h5 && !h6)
			sprintf(cParam1,"%x:%x:%x:%x::%x:%x",h1,h2,h3,h4, h7,h8);
		else if(!h6 && !h7)
			sprintf(cParam1,"%x:%x:%x:%x:%x::%x",h1,h2,h3,h4,h5, h8);
		//0 consecutive 0 case, i.e. no double colon case
		else if(1)
			sprintf(cParam1,"%x:%x:%x:%x:%x:%x:%x:%x",h1,h2,h3,h4,h5,h6,h7,h8);
	}
	else if(!strcmp(cRRType,"PTR"))
	{
		cParam2[0]=0;
		FQDomainName(cParam1);
		if(!cParam1[0])
		{
			guMode=uMode;
			tResource("cParam1 is required");
		}
		if(cParam1[strlen(cParam1)-1]!='.') strcat(cParam1,".");

		//Initial support for only one ip6.arpa zone per view
		if(!strcmp(cZone+strlen(cZone)-8,"ip6.arpa"))
		{
			if(strlen(cName)!= (32+31+10))
			{
				guMode=uMode;
				tResource("<blink>IPv6 PTR cName format must be 32 nibbles seperated"
						" by periods and end with '.ip6.arpa.'!</blink>");
			}
		}
	}
	else if(!strcmp(cRRType,"MX"))
	{
		if(!strcmp(cZone+strlen(cZone)-5,".arpa"))
			tResource("Can not add MX records to arpa zones");
		sscanf(cParam1,"%u",&a);
		sprintf(cParam1,"%u",a);
		if(!a || a>1000)
		{
			guMode=uMode;
			tResource("Invalid MX Priority Number for cParam1");
		}
		FQDomainName(cParam2);
		if(!cParam2[0])
		{
			guMode=uMode;
			tResource("cParam2 is required");
		}
		if(cParam2[strlen(cParam2)-1]!='.') strcat(cParam2,".");
		
	}
	else if(!strcmp(cRRType,"NS"))
	{
		//All cases
		cParam2[0]=0;
		if(!cParam1[0])
		{
			guMode=uMode;
			tResource("cParam1 is required");
		}
		FQDomainName(cParam1);
		if(cParam1[strlen(cParam1)-1]!='.') strcat(cParam1,".");

		//Allow subdomains
		//if(strcmp(cZone+strlen(cZone)-5,".arpa"))
		//{
		//	sprintf(cName,"%s.",cZone);
		//}
		//else no other rules for arpa zone for now TODO
	}
	else if(!strcmp(cRRType,"HINFO"))
	{
		if(!cParam1[0])
		{
			guMode=uMode;
			tResource("cParam1 is required");
		}
		if(cParam1[0]!='"' && cParam1[strlen(cParam1)-1]!='"')
		{
			sprintf(gcQuery,"\"%s\"",cParam1);
			strcpy(cParam1,gcQuery);
		}

		if(!cParam2[0])
		{
			guMode=uMode;
			tResource("cParam2 is required");
		}
		if(cParam2[0]!='"' && cParam1[strlen(cParam2)-1]!='"')
		{
			sprintf(gcQuery,"\"%s\"",cParam2);
			strcpy(cParam2,gcQuery);
		}
	}
	else if(!strcmp(cRRType,"TXT"))
	{
		cParam2[0]=0;
		if(!cParam1[0])
		{
			guMode=uMode;
			tResource("cParam1 is required");
		}
		if(cParam1[0]!='"' && cParam1[strlen(cParam1)-1]!='"')
		{
			if(strlen(cParam1)>250)
			{
				sprintf(gcQuery,"\"%.250s\" \"%.260s\"",cParam1,cParam1+250);
				strcpy(cParam1,gcQuery);
			}	
			else
			{
			sprintf(gcQuery,"\"%s\"",cParam1);
			strcpy(cParam1,gcQuery);
		}
		}

	}
	else if(!strcmp(cRRType,"SPF"))
	{
		cParam2[0]=0;
		if(!cParam1[0])
		{
			guMode=uMode;
			tResource("cParam1 is required");
		}
		if(cParam1[0]!='"' && cParam1[strlen(cParam1)-1]!='"')
		{
			sprintf(gcQuery,"\"%s\"",cParam1);
			strcpy(cParam1,gcQuery);
		}

	}
	else if(!strcmp(cRRType,"SRV"))
	{
		unsigned uI=0;
		if(!cName[0])
		{
			guMode=uMode;
			tResource("Service protocol and domain required");
		}
		else
		{
			register int x=0;
			
			//All lowercase
			for(x=0;x<strlen(cName);x++)
				cName[x]=tolower(cName[x]);
			if((strstr(cName,"_tcp")==NULL)&&(strstr(cName,"_udp")==NULL)&&
				(strstr(cName,"_tls")==NULL)&&(strstr(cName,"_sctp")==NULL))
			{
				guMode=uMode;
				tResource("Service protocol required: _tcp, _udp, _tls and _sctp allowed");
			}
		}	
			
		if(!cParam1[0])
		{
			guMode=uMode;
			tResource("Priority required");
		}
		if(!cParam2[0])
		{
			guMode=uMode;
			tResource("Weight required");
		}
		if(!cParam3[0])
		{
			guMode=uMode;
			tResource("Port required");
		}
		if(!cParam4[0])
		{
			guMode=uMode;
			tResource("Target host required");
		}

		if((strstr(cName,ForeignKey("tZone","cZone",uZone))==NULL))
		{
			guMode=uMode;
			tResource("Must include zone name in service parameter. E.g.: _sip._tcp.example.com.");
		}
		sscanf(cParam1,"%u",&uI);
		if(!uI && !(isdigit(cParam1[0])))
		{
			guMode=uMode;
			tResource("Must specify numerical priority");
		}
		uI=0;
		sscanf(cParam2,"%u",&uI);
		if(!uI && (!isdigit(cParam2[0])))
		{
			guMode=uMode;
			tResource("Must specify numerical weight");
		}
		uI=0;
		sscanf(cParam3,"%u",&uI);
		if((!uI) || (uI>65535))
		{
			guMode=uMode;
			tResource("Invalid port number");
		}
		FQDomainName(cParam4);
		if(cParam4[strlen(cParam4)-1]!='.') strcat(cParam4,".");
		if(cName[strlen(cName)-1]!='.') strcat(cName,".");

	}
	//Very initial NAPTR validation. Needs much more work to follow RFC2915 and RFC3403.
	else if(!strcmp(cRRType,"NAPTR"))
	{
		register int i;
		unsigned uI=0;

		if(!cName[0])
		{
			guMode=uMode;
			tResource("cName: Resource name required");
		}
		else
		{
			register int x=0;
			
			//All lowercase
			for(x=0;x<strlen(cName);x++)
				cName[x]=tolower(cName[x]);
		}	
		if(!cParam1[0])
		{
			guMode=uMode;
			tResource("cParam1: Order value required");
		}
		if(!cParam2[0])
		{
			guMode=uMode;
			tResource("cParam2: Preference value required");
		}
		if(!cParam3[0])
		{
			guMode=uMode;
			tResource("cParam3: Flags and ENUM double quoted strings required");
		}
		if(!cParam4[0])
		{
			guMode=uMode;
			tResource("cParam4: Double quoted regex string and optional SRV target required.");
		}

		sscanf(cParam1,"%u",&uI);
		if(!uI && !(isdigit(cParam1[0])))
		{
			guMode=uMode;
			tResource("cParam1: Must specify numerical order");
		}

		uI=0;
		sscanf(cParam2,"%u",&uI);
		if(!uI && (!isdigit(cParam2[0])))
		{
			guMode=uMode;
			tResource("cParam2: Must specify numerical preference");
		}

		//Check for double quotes
		uI=0;
		for(i=0;cParam3[i];i++)
			if(cParam3[i]=='\"') uI++;
		if(uI!=4)
		{
			guMode=uMode;
			tResource("cParam3: Must double quote both flags and ENUM string."
					" Ex: \"U\" \"E2U+sip\"");
		}

		uI=0;
		for(i=0;cParam4[i];i++)
			if(cParam4[i]=='\"') uI++;
		if(uI<2)
		{
			guMode=uMode;
			tResource("Must double quote REGEX."
					" Ex: \"!^.*$!sip:customer-service@example.com!\" _sip._udp.example.com");
		}

	}
	else if(1)
	{
		guMode=uMode;
		tResource("Must select valid uRRType");
	}
	
	PrepareTestData();
	OnLineZoneCheck(uZone,uMode,0);

}//void RRCheck(int uMode)


//Public domain from http://www.programmingsimplified.com/c-program-reverse-string
void ReverseString(char *x, int beg, int end)
{
   char c;
 
   if ( beg >= end )
      return;   
 
   c = *(x+beg);
   *(x+beg) = *(x+end);
   *(x+end) = c;
 
   ReverseString(x, ++beg, --end);

}//void ReverseString(char *x, int beg, int end)


char *CreateArpaZoneFileNameFromIPv6(const char *cIPv6, char *cIPv6FileName, char *cIPv6PTR)
{
	register int i;
	unsigned h1=0;
	unsigned h2=0;
	unsigned h3=0;
	unsigned h4=0;
	unsigned h5=0;
	unsigned h6=0;
	unsigned h7=0;
	unsigned h8=0;
	unsigned uColonCount=0;
	unsigned uRead=0;

	char cInBuffer[100];

	//Now for the hard work
	for(i=0;cIPv6[i];i++)
	{
		if(cIPv6[i]==':')
			uColonCount++;
		if(cIPv6[i]!=':' && !isxdigit(cIPv6[i]))
		{
			tResource("<blink>IPv6 number can only have hexadecimal digits and colons!</blink>");
		}
	}

	switch(uColonCount)
	{
		case 0:
		case 1:
			tResource("<blink>IPv6 too few colons: Min is 2!</blink>");
		break;

		case 2:
			uRead=sscanf(cIPv6,"%x::%x",&h1,&h8);
			if(uRead!=2)
			{
				tResource("<blink>IPv6 format-2 error!</blink>");
			}
		break;

		case 3:
			uRead=sscanf(cIPv6,"%x::%x:%x",&h1,&h7,&h8);
			if(uRead!=3)
			{
				uRead=sscanf(cIPv6,"%x:%x::%x",&h1,&h2,&h8);
				if(uRead!=3)
				{
					tResource("<blink>IPv6 format-3 error!</blink>");
				}
			}
		break;

		case 4:
			uRead=sscanf(cIPv6,"%x::%x:%x:%x",&h1,&h6,&h7,&h8);
			if(uRead!=4)
			{
				uRead=sscanf(cIPv6,"%x:%x::%x:%x",&h1,&h2,&h7,&h8);
				if(uRead!=4)
				{
					uRead=sscanf(cIPv6,"%x:%x:%x::%x",&h1,&h2,&h3,&h8);
					if(uRead!=4)
					{
						tResource("<blink>IPv6 format-4 error!</blink>");
					}
				}
			}
		break;

		case 5:
			uRead=sscanf(cIPv6,"%x::%x:%x:%x:%x",&h1,&h5,&h6,&h7,&h8);
			if(uRead!=5)
			{
				uRead=sscanf(cIPv6,"%x:%x::%x:%x:%x",&h1,&h2,&h6,&h7,&h8);
				if(uRead!=5)
				{
					uRead=sscanf(cIPv6,"%x:%x:%x::%x:%x",&h1,&h2,&h3,&h7,&h8);
					if(uRead!=5)
					{
						uRead=sscanf(cIPv6,"%x:%x:%x:%x::%x",&h1,&h2,&h3,&h4,&h8);
						if(uRead!=5)
						{
							tResource("<blink>IPv6 format-5 error!</blink>");
						}
					}
				}
			}
		break;

		case 6:
			uRead=sscanf(cIPv6,"%x::%x:%x:%x:%x:%x",&h1,&h4,&h5,&h6,&h7,&h8);
			if(uRead!=6)
			{
				uRead=sscanf(cIPv6,"%x:%x::%x:%x:%x:%x",&h1,&h2,&h5,&h6,&h7,&h8);
				if(uRead!=6)
				{
					uRead=sscanf(cIPv6,"%x:%x:%x::%x:%x:%x",&h1,&h2,&h3,&h6,&h7,&h8);
					if(uRead!=6)
					{
						uRead=sscanf(cIPv6,"%x:%x:%x:%x::%x:%x",&h1,&h2,&h3,&h4,&h7,&h8);
						if(uRead!=6)
						{
							uRead=sscanf(cIPv6,"%x:%x:%x:%x:%x::%x",
										&h1,&h2,&h3,&h4,&h5,&h8);
							if(uRead!=6)
							{
								tResource("<blink>IPv6 format-6 error!</blink>");
							}
						}
					}
				}
			}
		break;

		case 7:
			uRead=sscanf(cIPv6,"%x:%x:%x:%x:%x:%x:%x:%x",&h1,&h2,&h3,&h4,&h5,&h6,&h7,&h8);
			if(uRead!=8)
			{
				tResource("<blink>IPv6 format-7 error!</blink>");
			}
		break;

		default:
			tResource("<blink>IPv6 too many colons: Max is 7!</blink>");
		
	}

	//First basic checks for AAAA hosts
	if(!h1)
	{
		tResource("IPv6 number can not have a 0 in first 16 bit hex word.");
	}

	if(!h8)
	{
		sprintf(gcQuery," IPv6 number can not have a 0 in last 16 bit hex word:"
				" %x:%x:%x:%x:%x:%x:%x:%x",h1,h2,h3,h4,h5,h6,h7,h8);
		tResource(gcQuery);
	}


	//KISS standardize on 128-16= /112 CIDR or /16 subnet ip6.arpa files
	//this will keep PTRs small and the possible number of PTRs per file
	//a manageable 65535 max number
	sprintf(cInBuffer,"%4.4x%4.4x%4.4x%4.4x%4.4x%4.4x%4.4x",h1,h2,h3,h4,h5,h6,h7);
	ReverseString(cInBuffer,0,strlen(cInBuffer)-1);
	register int j=0;
	for(i=0;cInBuffer[i] && j<99;i++)
	{
		cIPv6FileName[j++]=cInBuffer[i];
		cIPv6FileName[j++]='.';
	}
	cIPv6FileName[j]=0;
	strcat(cIPv6FileName,"ip6.arpa");

	//The PTR part
	sprintf(cInBuffer,"%4.4x",h8);
	ReverseString(cInBuffer,0,strlen(cInBuffer)-1);
	j=0;
	for(i=0;cInBuffer[i] && j<99;i++)
	{
		cIPv6PTR[j++]=cInBuffer[i];
		cIPv6PTR[j++]='.';
	}
	cIPv6PTR[j-1]=0;

	return(cIPv6FileName);

}//char *CreateArpaZoneFileNameFromIPv6()


void ExttResourceCommands(pentry entries[], int x)
{
	MYSQL_RES *res;
	MYSQL_ROW field;
	unsigned uNSSet;
	time_t clock;

	if(!strcmp(gcFunction,"tResourceTools"))
	{
		//ModuleFunctionProcess()

		if(!strcmp(gcCommand,"Create ip6.arpa Zone"))
                {
			unsigned uNSSet=0;

			ProcesstResourceVars(entries,x);
			sprintf(cZone,"%.64s",ForeignKey("tZone","cZone",uZone));
			sscanf(ForeignKey("tZone","uNSSet",uZone),"%u",&uNSSet);
			//tResource(ForeignKey("tZone","uNSSet",uZone));

			if(guPermLevel>=9 && uZone && uOwner && cName[0] && cZone[0] && cParam1[0] && uNSSet)
			{
				char cFQDN[512];
				char cIPv6PTR[100];
				char cIPv6FileName[100];
				char cNameSave[256];

				sprintf(cNameSave,"%.255s",cName);
				if(cName[strlen(cName)-1]!='.')
				{
					if(strcmp(cName,"@"))
						sprintf(cFQDN,"%.255s.%.255s.",cName,cZone);
					else
						sprintf(cFQDN,"%.511s.",cZone);
				}
				else
					sprintf(cFQDN,"%.511s",cName);

	                        ProcesstResourceVars(entries,x);
				//cIPv6PTR e.g. 1.0.0.0
	                        CreateArpaZoneFileNameFromIPv6(cParam1,cIPv6FileName,cIPv6PTR);
	                        //tResource(cIPv6FileName);
	                        //tResource(cIPv6PTR);
	                        //tResource(cFQDN);
				guLoginClient=uZoneOwner;//For tJob coolness
				PopulateArpaZoneIPv6(cIPv6FileName,cIPv6PTR,cFQDN,1,uZone,uOwner,uNSSet);
				sprintf(cName,"%.255s",cNameSave);
	                        tResource("ip6.arpa record added");
			}
			else
			{
				tResource("<blink>Error:</blink> Denied by permissions settings");
			}
		}
		else if(!strcmp(gcCommand,"Search Set Operations"))
                {
			if(guPermLevel>=9)
			{
	                        ProcesstResourceVars(entries,x);
				uRRType=0;
				if(uZone)
				{
					sscanf(ForeignKey("tZone","uView",uZone),"%u",&uView);
					sprintf(cZoneSearch,"%.63s",ForeignKey("tZone","cZone",uZone));
				}
                        	guMode=12001;
	                        tResource("Search Set Operations");
			}
			else
			{
				tResource("<blink>Error:</blink> Denied by permissions settings");
			}
                }
		else if(!strcmp(gcCommand,"Reload Search Set"))
                {
			if(guPermLevel>=9)
			{
	                        ProcesstResourceVars(entries,x);
                        	guMode=12002;
	                        tResource("Search set reloaded");
			}
			else
			{
				tResource("<blink>Error:</blink> Denied by permissions settings");
			}
		}

		else if(!strcmp(gcCommand,"Remove from Search Set"))
                {
			if(guPermLevel>=9)
			{
	                        ProcesstResourceVars(entries,x);
                        	guMode=12002;
				char cQuerySection[256];
				unsigned uLink=0;

				if(cZoneSearch[0]==0 && cParam1Search[0]==0 && cParam2Search[0]==0 && cParam3Search[0]==0 && 
								cParam4Search[0]==0 && cCommentSearch[0]==0 && cNameSearch[0]==0 &&
								uView==0 && uRRType==0 && uOwner==0 && uSelectNSSet==0)
	                        	tResource("You must specify at least one search parameter");

				if((uGroup=uGetSearchGroup(gcUser))==0)
		                        tResource("No search set exists. Please create one first.");

				//Initial query section
				sprintf(gcQuery,"DELETE FROM tGroupGlue WHERE uGroup=%u AND uResource IN"
						" (SELECT uResource FROM tResource WHERE",uGroup);

				//Build AND query section

				if(cZoneSearch[0])
				{
					sprintf(cQuerySection," uZone IN (SELECT uZone FROM tZone WHERE cZone LIKE '%s%%')",cZoneSearch);
					strcat(gcQuery,cQuerySection);
					uLink=1;
				}
				else
				{
					uLink=0;
				}

				if(cParam1Search[0])
				{
					if(uLink)
						strcat(gcQuery," AND");
					sprintf(cQuerySection," cParam1 LIKE '%s%%'",cParam1Search);
					strcat(gcQuery,cQuerySection);
					uLink=1;
				}

				if(cParam2Search[0])
				{
					if(uLink)
						strcat(gcQuery," AND");
					sprintf(cQuerySection," cParam2 LIKE '%s%%'",cParam2Search);
					strcat(gcQuery,cQuerySection);
					uLink=1;
				}

				if(cParam3Search[0])
				{
					if(uLink)
						strcat(gcQuery," AND");
					sprintf(cQuerySection," cParam3 LIKE '%s%%'",cParam3Search);
					strcat(gcQuery,cQuerySection);
					uLink=1;
				}

				if(cParam4Search[0])
				{
					if(uLink)
						strcat(gcQuery," AND");
					sprintf(cQuerySection," cParam4 LIKE '%s%%'",cParam4Search);
					strcat(gcQuery,cQuerySection);
					uLink=1;
				}

				if(cCommentSearch[0])
				{
					if(uLink)
						strcat(gcQuery," AND");
					sprintf(cQuerySection," cComment LIKE '%s%%'",cCommentSearch);
					strcat(gcQuery,cQuerySection);
					uLink=1;
				}

				if(cNameSearch[0])
				{
					if(uLink)
						strcat(gcQuery," AND");
					sprintf(cQuerySection," cName LIKE '%s%%'",cNameSearch);
					strcat(gcQuery,cQuerySection);
					uLink=1;
				}

				if(uForClient)
				{
					if(uLink)
						strcat(gcQuery," AND");
					sprintf(cQuerySection," uOwner=%u",uForClient);
					strcat(gcQuery,cQuerySection);
					uLink=1;
				}

				if(uSelectNSSet)
				{
					if(uLink)
						strcat(gcQuery," AND");
					sprintf(cQuerySection," uZone IN (SELECT uZone FROM tZone WHERE uNSSet=%u)",uSelectNSSet);
					strcat(gcQuery,cQuerySection);
					uLink=1;
				}

				if(uView)
				{
					if(uLink)
						strcat(gcQuery," AND");
					sprintf(cQuerySection," uZone IN (SELECT uZone FROM tZone WHERE uView=%u)",uView);
					strcat(gcQuery,cQuerySection);
					uLink=1;
				}

				if(uRRType)
				{
					if(uLink)
						strcat(gcQuery," AND");
					sprintf(cQuerySection," uRRType=%u",uRRType);
					strcat(gcQuery,cQuerySection);
					uLink=1;
				}

				strcat(gcQuery,")");
				//debug only
	                        //tResource(gcQuery);

				mysql_query(&gMysql,gcQuery);
				if(mysql_errno(&gMysql))
						htmlPlainTextError(mysql_error(&gMysql));
				unsigned uNumber=0;
				if((uNumber=mysql_affected_rows(&gMysql))>0)
				{
	                        	sprintf(gcQuery,"%u resource records were removed from your search set",uNumber);
	                        	tResource(gcQuery);
				}
				else
				{
	                        	tResource("No records were removed from your search set");
				}
			}
			else
			{
				tResource("<blink>Error:</blink> Denied by permissions settings");
			}
		}
		else if(!strcmp(gcCommand,"Add to Search Set") || !strcmp(gcCommand,"Create Search Set"))
                {
			if(guPermLevel>=9)
			{
	                        ProcesstResourceVars(entries,x);
                        	guMode=12002;
				char cQuerySection[256];
				unsigned uLink=0;

				if(cZoneSearch[0]==0 && cParam1Search[0]==0 && cParam2Search[0]==0 && cParam3Search[0]==0 && 
								cParam4Search[0]==0 && cCommentSearch[0]==0 && cNameSearch[0]==0 &&
								uView==0 && uRRType==0 && uOwner==0 && uSelectNSSet==0)
	                        	tResource("You must specify at least one search parameter");

				if((uGroup=uGetSearchGroup(gcUser))==0)
				{
					sprintf(gcQuery,"INSERT INTO tGroup SET cLabel='%s',uGroupType=2"
						",uOwner=%u,uCreatedBy=%u,uCreatedDate=UNIX_TIMESTAMP(NOW())",
							gcUser,guCompany,guLoginClient);//2=search set type TODO
					mysql_query(&gMysql,gcQuery);
					if(mysql_errno(&gMysql))
							htmlPlainTextError(mysql_error(&gMysql));
					if((uGroup=mysql_insert_id(&gMysql))==0)
		                        	tResource("An error ocurred when attempting to create your search set");
				}
				else
				{
					if(!strcmp(gcCommand,"Create Search Set"))
					{
						sprintf(gcQuery,"DELETE FROM tGroupGlue WHERE uGroup=%u",uGroup);
						mysql_query(&gMysql,gcQuery);
						if(mysql_errno(&gMysql))
							htmlPlainTextError(mysql_error(&gMysql));
					}
                		}

				//Initial query section
				sprintf(gcQuery,"INSERT INTO tGroupGlue (uGroupGlue,uGroup,uZone,uResource)"
						" SELECT 0,%u,0,uResource FROM tResource WHERE",uGroup);

				//Build AND query section

				if(cZoneSearch[0])
				{
					sprintf(cQuerySection," uZone IN (SELECT uZone FROM tZone WHERE cZone LIKE '%s%%')",cZoneSearch);
					strcat(gcQuery,cQuerySection);
					uLink=1;
				}
				else
				{
					uLink=0;
				}

				if(cParam1Search[0])
				{
					if(uLink)
						strcat(gcQuery," AND");
					sprintf(cQuerySection," cParam1 LIKE '%s%%'",cParam1Search);
					strcat(gcQuery,cQuerySection);
					uLink=1;
				}

				if(cParam2Search[0])
				{
					if(uLink)
						strcat(gcQuery," AND");
					sprintf(cQuerySection," cParam2 LIKE '%s%%'",cParam2Search);
					strcat(gcQuery,cQuerySection);
					uLink=1;
				}

				if(cParam3Search[0])
				{
					if(uLink)
						strcat(gcQuery," AND");
					sprintf(cQuerySection," cParam3 LIKE '%s%%'",cParam3Search);
					strcat(gcQuery,cQuerySection);
					uLink=1;
				}

				if(cParam4Search[0])
				{
					if(uLink)
						strcat(gcQuery," AND");
					sprintf(cQuerySection," cParam4 LIKE '%s%%'",cParam4Search);
					strcat(gcQuery,cQuerySection);
					uLink=1;
				}

				if(cCommentSearch[0])
				{
					if(uLink)
						strcat(gcQuery," AND");
					sprintf(cQuerySection," cComment LIKE '%s%%'",cCommentSearch);
					strcat(gcQuery,cQuerySection);
					uLink=1;
				}

				if(cNameSearch[0])
				{
					if(uLink)
						strcat(gcQuery," AND");
					sprintf(cQuerySection," cName LIKE '%s%%'",cNameSearch);
					strcat(gcQuery,cQuerySection);
					uLink=1;
				}

				if(uForClient)
				{
					if(uLink)
						strcat(gcQuery," AND");
					sprintf(cQuerySection," uOwner=%u",uForClient);
					strcat(gcQuery,cQuerySection);
					uLink=1;
				}

				if(uSelectNSSet)
				{
					if(uLink)
						strcat(gcQuery," AND");
					sprintf(cQuerySection," uZone IN (SELECT uZone FROM tZone WHERE uNSSet=%u)",uSelectNSSet);
					strcat(gcQuery,cQuerySection);
					uLink=1;
				}

				if(uView)
				{
					if(uLink)
						strcat(gcQuery," AND");
					sprintf(cQuerySection," uZone IN (SELECT uZone FROM tZone WHERE uView=%u)",uView);
					strcat(gcQuery,cQuerySection);
					uLink=1;
				}

				if(uRRType)
				{
					if(uLink)
						strcat(gcQuery," AND");
					sprintf(cQuerySection," uRRType=%u",uRRType);
					strcat(gcQuery,cQuerySection);
					uLink=1;
				}

				//debug only
	                        //tResource(gcQuery);

				mysql_query(&gMysql,gcQuery);
				if(mysql_errno(&gMysql))
						htmlPlainTextError(mysql_error(&gMysql));
				unsigned uNumber=0;
				if((uNumber=mysql_affected_rows(&gMysql))>0)
				{
	                        	sprintf(gcQuery,"%u resource records were added to your search set",uNumber);
	                        	tResource(gcQuery);
				}
				else
				{
	                        	tResource("No records were added to your search set. Filter returned 0 records");
				}
			}
			else
			{
				tResource("<blink>Error:</blink> Denied by permissions settings");
			}
		}
		//Default wizard like two step creation and deletion
		else if(!strcmp(gcCommand,LANG_NB_NEW))
                {
			ProcesstResourceVars(entries,x);
			sscanf(ForeignKey("tZone","uOwner",uZone),"%u",
					&uZoneOwner);
			if(guPermLevel<10)
			{
				if(uZoneOwner) GetClientOwner(uZoneOwner,&guReseller);
			}
			if( (guPermLevel>=7 && uOwner==uZoneOwner)
				|| (guPermLevel>7 && guReseller==guLoginClient)
				|| (guPermLevel>9) )
			{
				strcpy(cZone,ForeignKey("tZone","cZone",uZone));

				if(uZone==0)
					tResource("Must start from valid zone");
				uOwner=uZoneOwner;
				uCreatedBy=guLoginClient;

				//Check global conditions for new record here
                        	guMode=2000;
                        	tResource(LANG_NB_CONFIRMNEW);
			}
			else
				tResource("<blink>Error</blink>: Denied by permissions settings");
                }
		else if(!strcmp(gcCommand,LANG_NB_CONFIRMNEW))
                {
			ProcesstResourceVars(entries,x);
			sscanf(ForeignKey("tZone","uOwner",uZone),"%u",
					&uZoneOwner);
			if(guPermLevel<10)
				if(uZoneOwner) GetClientOwner(uZoneOwner,&guReseller);

			//Check this out
			if( (guPermLevel>=7 && guLoginClient==uZoneOwner)
				|| (guPermLevel>7 && guReseller==guLoginClient)
				|| (guPermLevel>9 ) )
			{

				//char query[256];
				unsigned uSaveLoginClient=guLoginClient;

				RRCheck(2000);

				//New PTR record in automated arpa zones
				//exception allowed
				if(!uZoneOwner && uRRType!=RRTYPE_PTR)
				{
					guMode=0;
					tResource("uZoneOwner==0 contact admin asap");
				}

				uResource=0;
				uCreatedBy=guLoginClient;
				uOwner=uZoneOwner;
				uModBy=0;//Never modified
				
				if(uAddArpaPTR && uRRType==RRTYPE_A)
				{
					char cFQDN[512];
					char cNameSave[256];

					sprintf(cNameSave,"%.255s",cName);
				
					if(cName[strlen(cName)-1]!='.')
					{
						if(strcmp(cName,"@"))
							sprintf(cFQDN,"%.255s.%.255s.",cName,cZone);
						else
							sprintf(cFQDN,"%.511s.",cZone);
					}
					else
						sprintf(cFQDN,"%.511s",cName);

					guLoginClient=uZoneOwner;//For tJob coolness
					unsigned uNSSet=0;
					sscanf(ForeignKey("tZone","uNSSet",uZone),"%u",&uNSSet);
					PopulateArpaZone(cFQDN,cParam1,1,uZone,uZoneOwner,uNSSet);
					sprintf(cName,"%.255s",cNameSave);
					guLoginClient=uSaveLoginClient;
				}

				if(strstr(cZone,"in-addr.arpa") && uClient)
					uOwner=uClient;
				//tZone needs to be updated
				sscanf(ForeignKey("tZone","uNSSet",uZone),"%u",
					&uNSSet);
				time(&clock);
				//cZone set in RRCheck
				guLoginClient=uZoneOwner;//For tJob coolness
				if(SubmitJob("Modify",uNSSet,cZone,0,clock))
					tResource(mysql_error(&gMysql));
				guLoginClient=uSaveLoginClient;
				UpdateSerialNum(uZone);
                        	guMode=5;
				NewtResource(0);
			}
			else
				tResource("<blink>Error</blink>: Denied by permissions settings");
		}
		else if(!strcmp(gcCommand,LANG_NB_DELETE))
                {
                        ProcesstResourceVars(entries,x);
			if(uAllowDel(uOwner,uCreatedBy))
			{
                        	guMode=2001;
	                        tResource(LANG_NB_CONFIRMDEL);
			}
			else
				tResource("<blink>Error</blink>: Denied by permissions settings");
                }
                else if(!strcmp(gcCommand,LANG_NB_CONFIRMDEL))
                {
                        ProcesstResourceVars(entries,x);
			if(uAllowDel(uOwner,uCreatedBy))
			{
                        	guMode=5;
				//tZone needs to be updated
				sscanf(ForeignKey("tZone","uNSSet",uZone),"%u",
					&uNSSet);
				time(&clock);
				if(SubmitJob("Modify",
					uNSSet,ForeignKey("tZone","cZone",uZone),0,clock))
				tResource(mysql_error(&gMysql));
				UpdateSerialNum(uZone);                      	
				sprintf(gcQuery,"INSERT INTO tDeletedResource SET uDeletedResource='%u',uZone='%u',"
						"cName='%s',uTTL='%u',uRRType='%u',cParam1='%s',cParam2='%s',"
						"cParam3='%s',cParam4='%s',"
						"cComment='%s',uOwner='%u',uCreatedBy=1,uCreatedDate=UNIX_TIMESTAMP(NOW())"
						" ON DUPLICATE KEY UPDATE uDeletedResource='%u',uZone='%u',"
						"cName='%s',uTTL='%u',uRRType='%u',cParam1='%s',cParam2='%s',"
						"cParam3='%s',cParam4='%s',"
						"cComment='%s',uOwner='%u',uCreatedBy=1,uCreatedDate=UNIX_TIMESTAMP(NOW())",
						uResource,
						uZone,
						cName,
						uTTL,
						uRRType,
						cParam1,
						cParam2,
						cParam3,
						cParam4,
						cComment,
						uOwner,
						uResource,
						uZone,
						cName,
						uTTL,
						uRRType,
						cParam1,
						cParam2,
						cParam3,
						cParam4,
						cComment,
						uOwner);
				mysql_query(&gMysql,gcQuery);
				if(mysql_errno(&gMysql))
					htmlPlainTextError(mysql_error(&gMysql));
				
				DeletetResource();
			}
			else
				tResource("<blink>Error</blink>: Denied by permissions settings");
                }
		else if(!strcmp(gcCommand,LANG_NB_MODIFY))
                {
                        ProcesstResourceVars(entries,x);
			if(uAllowMod(uOwner,uCreatedBy))
			{
                        	guMode=2002;
                        	tResource(LANG_NB_CONFIRMMOD);
			}
			else
				tResource("<blink>Error</blink>: Denied by permissions settings");
                }
                else if(!strcmp(gcCommand,LANG_NB_CONFIRMMOD))
                {
                        ProcesstResourceVars(entries,x);
			if(uAllowMod(uOwner,uCreatedBy))
			{

				//Check entries here
				RRCheck(2002);
				uModBy=guLoginClient;
				if(uAddArpaPTR && uRRType==RRTYPE_A)
				{
					char cFQDN[512];
					char cParam1Save[256];
					char cNameSave[256];
					unsigned uResourceSave=uResource;

					sprintf(cParam1Save,"%.255s",cParam1);
					sprintf(cNameSave,"%.255s",cName);
				
					if(cName[strlen(cName)-1]!='.')
					{
						if(strcmp(cName,"@"))
							sprintf(cFQDN,"%.255s.%.255s.",cName,cZone);
						else
							sprintf(cFQDN,"%.511s.",cZone);
					}
					else
						sprintf(cFQDN,"%.511s",cName);


					sscanf(ForeignKey("tZone","uOwner",uZone),"%u",
							&uZoneOwner);
					unsigned uNSSet=0;
					sscanf(ForeignKey("tZone","uNSSet",uZone),"%u",&uNSSet);
					PopulateArpaZone(cFQDN,cParam1,1,uZone,uZoneOwner,uNSSet);
					sprintf(cParam1,"%.255s",cParam1Save);
					sprintf(cName,"%.255s",cNameSave);
					uResource=uResourceSave;
				}

				if(strstr(cZone,"in-addr.arpa") && uClient && uResource)
				{
					sprintf(gcQuery,"UPDATE tResource SET uOwner=%u WHERE uResource=%u",
							uClient,uResource);
					mysql_query(&gMysql,gcQuery);
					if(mysql_errno(&gMysql))
						htmlPlainTextError(mysql_error(&gMysql));
				}

				//tZone needs to be updated
				sscanf(ForeignKey("tZone","uNSSet",uZone),"%u",
					&uNSSet);
				time(&clock);
				if(SubmitJob("Modify",uNSSet,cZone,0,clock))
					tResource(mysql_error(&gMysql));
				UpdateSerialNum(uZone);
                        	guMode=5;
                        	ModtResource();
			}
			else
				tResource("<blink>Error</blink>: Denied by permissions settings");
                }
                else if(!strcmp(gcCommand,"Delegation Wizard"))
                {
                        ProcesstResourceVars(entries,x);
			if(uAllowMod(uOwner,uCreatedBy))
			{
				guMode=3001;
			}
			else
				tResource("<blink>Error</blink>: Denied by permissions settings");
		}
                else if(!strcmp(gcCommand,"Create Delegation"))
                {
                        ProcesstResourceVars(entries,x);
			if(uAllowMod(uOwner,uCreatedBy))
			{
				register unsigned i;
				unsigned uNumIPs=0;
				char cParam1[16]={""};
				char cNSSet[4][100]={"ns1.clientisp.net","ns2.clientisp.net","",""};
				unsigned uNumNSSets=2;

				//TODO
				guMode=3001;
				//Check uZone,uCIDR and uSelectNSSet
				if(!uZone)
					tResource("Undefined uZone!");
				if(uCIDR>29 || uCIDR<24)
					tResource("CIDR must be in range 24 to 29 (256 to 8 IPs)");
				if(!uSelectNSSet)
					tResource("Must select a valid tNSSet.cLabel from select");
				sprintf(gcQuery,"SELECT tNS.cFQDN FROM tNSSet,tNS WHERE tNSSet.uNSSet=tNS.uNSSet AND"
						" tNSSet.uNSSet=%u",uSelectNSSet);
				mysql_query(&gMysql,gcQuery);
				if(mysql_errno(&gMysql)) tResource(mysql_error(&gMysql));
				res=mysql_store_result(&gMysql);
				if(!(field=mysql_fetch_row(res)))
					tResource("No NS found for given uNSSet!");

				uNumIPs=2<<(32-uCIDR-1);
				//jic
				if(uNumIPs>256 || uNumIPs<8)
					tResource("uNumIPs must be in range 8 to 256");
				if((uNumIPs+uStartBlock-1)>255)
					tResource("Start/CIDR goes past last octet 255."
							" Please correct CIDR and/or uStartBlock.");

				//Special case 0/24 (Class C)
				if(uNumIPs==256)
				{
					//TODO must fix tzonefunc also...
					strcpy(cZone,ForeignKey("tZone","cZone",uZone));

					for(i=0;i<uNumNSSets;i++)
					{
						sprintf(gcQuery,"INSERT INTO tResource SET uZone=%u,cName='%.99s',"
								"uRRType=%u,cParam1='%.99s.',"
								"cComment='DelegationWizard created',uOwner=%u,"
								"uCreatedDate=UNIX_TIMESTAMP(NOW())",
								uZone
								,cZone
								,RRTYPE_NS
								,cNSSet[i]
								,guLoginClient);
						mysql_query(&gMysql,gcQuery);
						if(mysql_errno(&gMysql))
							tResource(mysql_error(&gMysql));

					}
					guMode=0;
					sprintf(gcQuery,"Delegation done for tNSSet set: %s and direct"
							" classC delegation</font>",
								ForeignKey("tNSSet","cLabel",uSelectNSSet));
					tResource(gcQuery);
				}

				//Loop for each cList line: Add the NS records.
				for(i=0;i<uNumNSSets;i++)
				{
					sprintf(cParam1,"%u-%u",uStartBlock,uNumIPs-1);
					sprintf(gcQuery,"INSERT INTO tResource SET uZone=%u,cName='%.15s',"
							"uRRType=%u,cParam1='%.99s.'"
							",cComment='DelegationWizard created',uOwner=%u,"
							"uCreatedDate=UNIX_TIMESTAMP(NOW())",
							uZone
							,cParam1
							,RRTYPE_NS
							,cNSSet[i]
							,guLoginClient);
					mysql_query(&gMysql,gcQuery);
					if(mysql_errno(&gMysql))
						tResource(mysql_error(&gMysql));
				}
				

				//Loop for all CNAME records
				for(i=0;i<uNumIPs;i++)
				{
					sprintf(cParam1,"%u.%u-%u",
						i+uStartBlock,uStartBlock,uNumIPs-1);
					sprintf(gcQuery,"INSERT INTO tResource SET uZone=%u,cName='%u',uRRType=5,"
							"cParam1='%s',cComment='DelegationWizard created',uOwner=%u,"
							"uCreatedDate=UNIX_TIMESTAMP(NOW())",
							uZone
							,uStartBlock+i
							,cParam1
							,guLoginClient);
					mysql_query(&gMysql,gcQuery);
					if(mysql_errno(&gMysql))
						tResource(mysql_error(&gMysql));
				}

				guMode=0;
				sprintf(gcQuery,"Delegation done for tNSSet set: %s and for %u IPs. CIDR %u/%u",
						ForeignKey("tNSSet","cLabel",uSelectNSSet)
						,uNumIPs
						,uStartBlock
						,uCIDR);
				tResource(gcQuery);
			}
			else
				tResource("<blink>Error</blink>: Denied by permissions settings");
		}
                else if(!strcmp(gcCommand,"Delete Delegation"))
                {
                        ProcesstResourceVars(entries,x);
			if(uAllowDel(uOwner,uCreatedBy))
			{
				sprintf(gcQuery,"DELETE FROM tResource WHERE cComment LIKE 'DelegationWizard%%'"
						" AND uZone=%u",uZone);
				mysql_query(&gMysql,gcQuery);
				if(mysql_errno(&gMysql))
					tResource(mysql_error(&gMysql));
				tResource("Delete delegation done");
			}
			else
				tResource("<blink>Error</blink>: Denied by permissions settings");
		}
                else if(!strncmp(gcCommand,"Group ",6) || !strncmp(gcCommand,"Delete Checked",14))
                {
			ProcesstResourceVars(entries,x);
                        guMode=12002;
			tResource(gcCommand);
		}
	}

}//void ExttResourceCommands(pentry entries[], int x)


void ResourceLinks(unsigned uZone)
{
	MYSQL_RES *res;
	unsigned uArpa=0;
	unsigned uCount=0;

	if(!strcmp(cZone+strlen(cZone)-5,".arpa"))
	{
		sprintf(gcQuery,"SELECT cName,uResource,cParam1 FROM tResource WHERE uZone=%u ORDER BY ABS(cName)",uZone);
		uArpa=1;
	}
	else
	{
	sprintf(gcQuery,"SELECT tResource.cName,tResource.uResource,tRRType.cLabel FROM tResource,tRRType WHERE "
			"tResource.uZone=%u AND tResource.uRRType=tRRType.uRRType ORDER BY tResource.cName"
				,uZone);
	}
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql)) tResource(mysql_error(&gMysql));

	res=mysql_store_result(&gMysql);
	uCount=mysql_num_rows(res);
/*
	MYSQL_ROW field;

	printf("<p><u>RRNavList(%u)</u><br>",uCount);
	while((field=mysql_fetch_row(res)))
	{
		printf("<input type=checkbox name=uGrpResource%s> ",field[1]);
		if(uArpa)
			printf("<a class=darkLink href=?gcFunction=tResource&uResource=%s&cZone=%s>%s %s</a><br>\n",
				field[1],cZone,field[0],field[2]);
		else
			printf("<a class=darkLink href=?gcFunction=tResource&uResource=%s>%s %s</a><br>\n",
				field[1],field[0],field[2]);
	}

	printf("<p><u>Group Operations</u><br>");
	printf("<input title='Will remove the above checkbox selected records' "
		"class=lwarnButton type=submit name=gcCommand value='Delete Selected Records'>");
*/

}//void ResourceLinks(unsigned uZone)


void ExttResourceButtons(void)
{
	unsigned uDefault=0;

	sprintf(gcQuery,"tResource Aux Panel");
	OpenFieldSet(gcQuery,100);

	strcpy(cZone,ForeignKey("tZone","cZone",uZone));

	switch(guMode)
        {
                case 12001:
                case 12002:
			printf("<u>Create or refine your user search set</u><br>");
			printf("In the right panel you can select your search criteria. When refining you do not need"
				" to reuse your initial search critieria. Your search set is persistent even across unxsVZ sessions.<p>");
			printf("<input type=submit class=largeButton title='Create an initial or replace an existing search set'"
				" name=gcCommand value='Create Search Set'>\n");
			printf("<input type=submit class=largeButton title='Add the results to your current search set. Do not add the same search"
				" over and over again it will not result in any change but may slow down processing.'"
				" name=gcCommand value='Add to Search Set'>\n");
			printf("<p><input type=submit class=largeButton title='Apply the right panel filter to refine your existing search set"
				" by removing set elements that match the filter settings.'"
				" name=gcCommand value='Remove from Search Set'>\n");
			printf("<p><input type=submit class=largeButton title='Reload current search set. Good for checking for any new status updates'"
				" name=gcCommand value='Reload Search Set'>\n");
			printf("<input type=submit class=largeButton title='Return to main tResource tab page'"
				" name=gcCommand value='Cancel'>\n");
                break;

                case 2000:
			printf("<p><u>Enter required data</u><br>");
                        printf(LANG_NBB_CONFIRMNEW);
			if(uRRType==RRTYPE_A && strcmp(cZone+strlen(cZone)-5,".arpa"))
			{
				printf("<br><input title='For some RR types this will add a PTR entry to"
					" the correct .arpa zone' type=checkbox name=uAddArpaPTR> RevDNS");
			}
			else if(!strcmp(cZone+strlen(cZone)-5,".arpa"))
			{
				if(guPermLevel>9)
				{
					printf("<p><u>Create for customer</u><br>");
					CustomerDropDown(uClient);
				}
			}
                break;

                case 2001:
			printf("<p><u>Think twice</u><br>");
                        printf(LANG_NBB_CONFIRMDEL);
                break;

                case 2002:
			printf("<p><u>Review record data</u><br>");
                        printf(LANG_NBB_CONFIRMMOD);
			if(uRRType==RRTYPE_A && strcmp(cZone+strlen(cZone)-5,".arpa"))
			{
				printf("<br><input title='For some RR types this will add a PTR entry to"
					" the correct .arpa zone' type=checkbox name=uAddArpaPTR> RevDNS");
			}
			else if(!strcmp(cZone+strlen(cZone)-5,".arpa"))
			{
				if(guPermLevel>9)
				{
					printf("<p><u>Change uOwner for customer</u><br>");
					CustomerDropDown(uClient);
				}
				printf("<p><input title='Will add NS records and the CIDR defined number of"
					" CNAME records in n.0-(2^(32-CIDR))-1 "
					"style for supplied NS previously defined in tNSSet to this zone' class=largeButton "
					"type=submit name=gcCommand value='Delegation Wizard'>");
			}

                break;

		case 3001:
		if(guPermLevel>9)
		{
			printf("<p><u>Delegation Wizard</u><br>");
			tTablePullDown("tNSSet;cuNSSetPullDown","cLabel","cLabel",
				uSelectNSSet,1);
			printf("<br><input title='Enter CIDR block start number (uStartBlock.) "
				"Acceptable range 0-248 depends on CIDR below' type=text "
				"name=uStartBlock size=8 maxlength=3 value=0>");
			printf("<br><input title='Enter CIDR number (uCIDR.) Ex. 26 for 64 IP block. "
				"Range 24-29' type=text name=uCIDR size=8 maxlength=2>");
			printf("<br><input title='Will add NS records -as per select set- and the CIDR "
				"defined number of CNAME records in n.0-(2^(32-CIDR))-1 style to this zone' "
				"class=largeButton type=submit name=gcCommand value='Create Delegation'>");
			printf("<br><input title='Will remove all records that say DelegationWizard in the "
				"cComment field for this zone' class=lwarnButton type=submit name=gcCommand"
				" value='Delete Delegation'>");
		}
		break;

		default:
			uDefault=1;
			printf("<u>Table Tips</u><br>");
			printf("Here we gather all zone resource records, usually organized by a single given uZone. "
				"Quite a bit of input and context input checking takes place, but you probably should "
				"know DNS/BIND configuration. A little knowledge will help you use this backend quickly"
				" and correctly.\n");
			printf("<p><u>Search Tools</u><br>");
			printf("<input type=text title=\"cName search input. Use %% . and _ for power searching.\" "
				"name=cSearch value=\"%s\" maxlength=99 size=20>",cSearch);
			printf("<p><input type=submit class=largeButton title='Open user search set page. There you can create search sets and operate"
				" on selected resources of the loaded resource set.'"
				" name=gcCommand value='Search Set Operations'>\n");
			if(uZone)
			{
				unsigned uView=0;

				sscanf(ForeignKey("tZone","uView",uZone),"%u",&uView);

				printf("<p><u>ZoneNavList</u><br>");
				printf("<a class=darkLink href=?gcFunction=tZone&uZone=%u&cZone=%s>",uZone,cZone);
				if(cZone[0])
					printf("%s %s</a>",cZone,ForeignKey("tView","cLabel",uView));
				else
					printf("Back to Zone</a>");
				ResourceLinks(uZone);
			}
			if(uRRType==9)
			{
				printf("<p><input type=submit class=largeButton title='Create if needed an ip6.arpa zone and add PTR for loaded RR."
				" Will use /16 zone file reduced PTR format if creating a new ip6.arpa zone.'"
				" name=gcCommand value='Create ip6.arpa Zone'>\n");
			}


	}

	if(!uDefault && cSearch[0])
		printf("<input type=hidden name=cSearch value=\"%s\">",cSearch);

	CloseFieldSet();	

}//void ExttResourceButtons(void)


void ExttResourceAuxTable(void)
{
	MYSQL_RES *res;
	MYSQL_ROW field;
	unsigned uGroup=0;
	unsigned uRow=0;
	unsigned uNumRows=0;
	unsigned uOnlyOncePerSet=1;

	switch(guMode)
	{
		case 12001:
		case 12002:
			//Set operation buttons
			OpenFieldSet("Set Operations",100);
			printf("<input title='For supported set operations (like Group Delete)"
				" create the appropiate jobs for changes to go into effect.'"
				" type=checkbox name=uSubmitJobNoCA");
			if(uSubmitJob)
				printf(" checked");
			printf("> uSubmitJob");
			printf("<p><input title='Delete checked resource records from your search set. They will still be visible but will"
				" marked deleted and will not be used in any subsequent set operation'"
				" type=submit class=largeButton name=gcCommand value='Delete Checked'>\n");
			printf("<input disabled title='Append comment to selected resource records.'"
				" type=submit class=largeButton name=gcCommand value='Group Append Comment'>\n");
			printf("<input disabled title='Replace comment of selected resource records.'"
				" type=submit class=largeButton name=gcCommand value='Group Replace Comment'>\n");
			printf("<input disabled title='Change RRType of selected resource records.'"
				" type=submit class=lwarnButton name=gcCommand value='Group Change RRType'>\n");
			printf("<input disabled title='Change TTL of selected resource records.'"
				" type=submit class=lwarnButton name=gcCommand value='Group Change TTL'>\n");
			printf("<input title='Change owner via select'"
				" type=submit class=lwarnButton name=gcCommand value='Group Change Owner'>\n");
			printf("<input disabled title='Add a string to the end of selected resource record cName.'"
				" type=submit class=lwarnButton name=gcCommand value='Group Append cName'>\n");
			printf("<input title='Delete selected resource records from tResource -no undo available'"
				" type=submit class=lwarnButton name=gcCommand value='Group Delete'>\n");
			// move this very dangerous button to tZone seearch set OPs
			//printf("<input title='Delete all resource records and the parent zone from tZone and tResource -no undo available'"
			//	" type=submit class=lwarnButton name=gcCommand value='Group Zone Delete'>\n");
			CloseFieldSet();

			sprintf(gcQuery,"Search Set Contents");
			OpenFieldSet(gcQuery,100);
			uGroup=uGetSearchGroup(gcUser);
			//we need to use left joins here to allow for null uClient
			sprintf(gcQuery,"SELECT"
					" tResource.uResource,"
					" tZone.cZone,"
					" tView.cLabel,"
					" tResource.cName,"
					" tResource.uTTL,"
					" tRRType.cLabel,"
					" tResource.cParam1,"
					" tResource.cParam2,"
					" tResource.cParam3,"
					" tResource.cParam4,"
					" tResource.cComment,"
					" FROM_UNIXTIME(tResource.uCreatedDate,'%%a %%b %%d %%T %%Y'),"
					//" tClient.cLabel,"
					" '',"
					" tNSSet.cLabel,"
					" tZone.uZone"
					//" FROM tResource,tZone,tRRType,tClient,tView,tNSSet"
					" FROM tResource,tZone,tRRType,tView,tNSSet"
					" WHERE tResource.uRRType=tRRType.uRRType"
					//" AND tResource.uOwner=tClient.uClient"
					" AND tResource.uZone=tZone.uZone"
					" AND tZone.uView=tView.uView"
					" AND tZone.uNSSet=tNSSet.uNSSet"
					" AND uResource IN (SELECT uResource FROM tGroupGlue WHERE uGroup=%u) ORDER BY tZone.uZone",uGroup);
		        mysql_query(&gMysql,gcQuery);
		        if(mysql_errno(&gMysql))
				htmlPlainTextError(mysql_error(&gMysql));
		        res=mysql_store_result(&gMysql);
			if((uNumRows=mysql_num_rows(res)))
			{
				char cResult[100]={""};
				char cCtLabel[100]={""};

				printf("<table>");
				printf("<tr>");
				printf("<td valign=top>"
					"<input title='Check all records' type=checkbox name=all onClick='checkAll(document.formMain,this)'></td>"
					"<td title='For display purposes only: @ replaces the zone name'><u>cName</u></td>"
					"<td><u>cZone</u></td>"
					"<td><u>View</u></td>"
					"<td><u>uTTL</u></td>"
					"<td><u>RRType</u></td>"
					"<td><u>cParam1</u></td>"
					"<td><u>cParam2</u></td>"
					"<td><u>cParam3</u></td>"
					"<td><u>cParam4</u></td>"
					"<td><u>cComment</u></td>"
					"<td><u>uCreatedDate</u></td>"
					"<td><u>Owner</u></td>"
					"<td><u>NSSet</u></td>"
					"<td><u>Set operation result</u></td></tr>");
//Reset margin start
while((field=mysql_fetch_row(res)))
{
	if(guMode==12002)
	{
		register int i;
		unsigned uCtResource=0;

		cResult[0]=0;
		sscanf(field[0],"%u",&uCtResource);
		sprintf(cCtLabel,"Ct%u",uCtResource);
		for(i=0;i<x;i++)
		{
			//insider xss protection
			if(guPermLevel<10)
				continue;

			if(!strcmp(entries[i].name,cCtLabel))
			{
				unsigned uZone=0;

				sscanf(field[14],"%u",&uZone);

				if(!strcmp(gcCommand,"Delete Checked"))
				{
					sprintf(gcQuery,"DELETE FROM tGroupGlue WHERE uGroup=%u AND uResource=%u",uGroup,uCtResource);
					mysql_query(&gMysql,gcQuery);
					if(mysql_errno(&gMysql))
					{
						sprintf(cResult,mysql_error(&gMysql));
						break;
					}

					if(mysql_affected_rows(&gMysql)>0)
						sprintf(cResult,"Deleted from set");
					else
						sprintf(cResult,"Unexpected non deletion");
					break;
				}//Delete Checked

				else if(!strcmp(gcCommand,"Group Change Owner"))
				{
					if(uForClient)
					{
						sprintf(gcQuery,"UPDATE tResource SET uOwner=%u WHERE uResource=%u",uForClient,uCtResource);
						mysql_query(&gMysql,gcQuery);
						if(mysql_errno(&gMysql))
						{
							sprintf(cResult,mysql_error(&gMysql));
							break;
						}
	
						if(mysql_affected_rows(&gMysql)>0)
							sprintf(cResult,"Owner changed");
						else
							sprintf(cResult,"Owner not changed");
					}
					else
					{
						sprintf(cResult,"No client selected");
					}
					break;
				}//Group Change Owner

				else if(!strcmp(gcCommand,"Group Delete"))
				{
					static unsigned uPrevZone=0;

					sprintf(gcQuery,"DELETE FROM tResource WHERE uResource=%u",uCtResource);
					mysql_query(&gMysql,gcQuery);
					if(mysql_errno(&gMysql))
					{
						sprintf(cResult,mysql_error(&gMysql));
						break;
					}
					if(mysql_affected_rows(&gMysql)>0)
					{
						sprintf(cResult,"Deleted");

						if(uZone && uPrevZone!=uZone && uSubmitJob)
						{
							unsigned uNSSet;
							time_t clock;
	
							sscanf(ForeignKey("tZone","uNSSet",uZone),"%u",&uNSSet);
							time(&clock);
							UpdateSerialNum(uZone);                      	
							if(SubmitJob("Modify",uNSSet,ForeignKey("tZone","cZone",uZone),0,clock+60))
								strcat(cResult,mysql_error(&gMysql));
							else
								strcat(cResult," +SubmitJob(Modify)");
							uPrevZone=uZone;
						}
					}
					else
						sprintf(cResult,"Unexpected non deletion");
					break;
				}//Group Delete
		
				else if(!strcmp(gcCommand,"Group Zone Delete"))
				{
					static unsigned uPrevZone=0;
					unsigned uNSSet;

					sscanf(ForeignKey("tZone","uNSSet",uZone),"%u",&uNSSet);

					sprintf(gcQuery,"DELETE FROM tResource WHERE uResource=%u",uCtResource);
					mysql_query(&gMysql,gcQuery);
					if(mysql_errno(&gMysql))
					{
						sprintf(cResult,mysql_error(&gMysql));
						break;
					}
					if(mysql_affected_rows(&gMysql)>0)
					{
						unsigned uDelJob=0;
						sprintf(cResult,"Deleted");

						sprintf(gcQuery,"DELETE FROM tZone WHERE uZone=%u",uZone);
						mysql_query(&gMysql,gcQuery);
						if(mysql_errno(&gMysql))
						{
							sprintf(cResult,mysql_error(&gMysql));
							break;
						}
						if(mysql_affected_rows(&gMysql)>0)
						{
							strcat(cResult," +zone-deleted");
							//We only need one Delete job
							if(uOnlyOncePerSet)
							{
								uDelJob=1;
								uOnlyOncePerSet=0;
							}
						}
						if(uZone && uPrevZone!=uZone && uSubmitJob)
						{
							time_t clock;
	
							time(&clock);
							UpdateSerialNum(uZone);                      	
							if(uDelJob)
							{
								if(SubmitJob("Delete",uNSSet,ForeignKey("tZone","cZone",uZone),0,clock+60))
									strcat(cResult,mysql_error(&gMysql));
								else
									strcat(cResult," +SubmitJob(Delete)");
							}
							else
							{
								if(SubmitJob("Modify",uNSSet,ForeignKey("tZone","cZone",uZone),0,clock+60))
									strcat(cResult,mysql_error(&gMysql));
								else
									strcat(cResult," +SubmitJob(Modify)");
							}
							uPrevZone=uZone;
						}
					}
					else
						sprintf(cResult,"Unexpected non deletion");
					break;
				}//Group Delete
		
				else if(1)
				{
					sprintf(cResult,"Unexpected gcCommand=%.64s",gcCommand);
					break;
				}
			}//end if Ct block
		}//end for()
	}

	printf("<tr");
	if((uRow++) % 2)
		printf(" bgcolor=#E7F3F1 ");
	else
		printf(" bgcolor=#EFE7CF ");
	printf(">");

		char *cp;
		if((cp=strstr(field[3],field[1])))
		{
			*(cp)=0;
			if(!field[3][0])
				sprintf(field[3],"@");
			else
				strcat(field[3],"@");
		}
			
		printf("<td valign=top><input type=checkbox name=Ct%s ></td>"
		"<td valign=top><a class=darkLink href=?gcFunction=tResource&uResource=%s>%s</a></td>"
		"<td valign=top><a class=darkLink href=?gcFunction=tZone&uZone=%s>%s</a></td>"
		"<td valign=top>%.64s</td>"
		"<td valign=top>%.64s</td>"
		"<td valign=top>%.64s</td>"
		"<td valign=top>%.64s</td>"
		"<td valign=top>%.64s</td>"
		"<td valign=top>%.64s</td>"
		"<td valign=top>%.64s</td>"
		"<td valign=top>%.64s</td>"
		"<td valign=top>%.64s</td>"
		"<td valign=top>%.64s</td>"
		"<td valign=top>%.31s</td>"
		"<td valign=top>%.64s</td>\n",
			field[0],
			field[0],field[3],
			field[14],field[1],
			field[2],
			field[4],field[5],field[6],
			field[7],field[8],field[9],field[10],field[11],field[12],field[13],cResult);
	printf("</tr>");

}//while()
//Reset margin end

			printf("<tr><td valign=top>"
				"<input title='Check all records' type=checkbox name=all onClick='checkAll(document.formMain,this)'></td>"
				"<td valign=top colspan=3>Check all %u resource records</td></tr>\n",uNumRows);
			printf("</table></form>");

			}//If results
			mysql_free_result(res);
			CloseFieldSet();
		break;

	}//switch(guMode)

}//void ExttResourceAuxTable(void)


void ExttResourceGetHook(entry gentries[], int x)
{
	register int i;

	for(i=0;i<x;i++)
	{
		if(!strcmp(gentries[i].name,"uResource"))
		{
			sscanf(gentries[i].val,"%u",&uResource);
			guMode=6;
		}
		else if(!strcmp(gentries[i].name,"cZone"))
		{
			sprintf(cZone,"%.99s",gentries[i].val);
		}
	}
	tResource("");

}//void ExttResourceGetHook(entry gentries[], int x)


void ExttResourceSelect(void)
{

	if(uZone)
	{
		char cExtra[33]={""};

		sprintf(cExtra,"uZone=%u",uZone);

		if(cSearch[0])
			ExtSelectSearch("tResource",VAR_LIST_tResource,"cName",cSearch,cExtra,0);
		else
			ExtSelectSearch("tResource",VAR_LIST_tResource,NULL,NULL,cExtra,0);
	}
	else
	{
		if(cSearch[0])
			ExtSelectSearch("tResource",VAR_LIST_tResource,"cName",cSearch,NULL,0);
		else
			ExtSelect("tResource",VAR_LIST_tResource,0);
	}

}//void ExttResourceSelect(void)


void ExttResourceSelectRow(void)
{
	ExtSelectRow("tResource",VAR_LIST_tResource,uResource);

}//void ExttResourceSelectRow(void)


void ExttResourceListSelect(void)
{
	char cCat[512];

	ExtListSelect("tResource",VAR_LIST_tResource);

	//Changes here must be reflected below in ExttResourceListFilter()
        if(!strcmp(gcFilter,"uResource"))
        {
                sscanf(gcCommand,"%u",&uResource);
		if(guPermLevel<10)
			strcat(gcQuery," AND ");
		else
			strcat(gcQuery," WHERE ");
		sprintf(cCat,"tResource.uResource=%u ORDER BY uResource",uResource);
		strcat(gcQuery,cCat);
        }
        else if(!strcmp(gcFilter,"cName"))
        {
		if(guPermLevel<10)
			strcat(gcQuery," AND ");
		else
			strcat(gcQuery," WHERE ");
		sprintf(cCat,"tResource.cName LIKE '%s%%' ORDER BY tResource.cName",
				TextAreaSave(gcCommand));
		strcat(gcQuery,cCat);
        }
        else if(!strcmp(gcFilter,"cParam1"))
        {
		if(guPermLevel<10)
			strcat(gcQuery," AND ");
		else
			strcat(gcQuery," WHERE ");
		sprintf(cCat,"tResource.cParam1 LIKE '%s%%' ORDER BY tResource.cParam1",
				TextAreaSave(gcCommand));
		strcat(gcQuery,cCat);
        }
        else if(1)
        {
                //None NO FILTER
                strcpy(gcFilter,"None");
		strcat(gcQuery," ORDER BY uResource");
        }

}//void ExttResourceListSelect(void)


void ExttResourceListFilter(void)
{
        //Filter
        printf("<td align=right >Select ");
        printf("<select name=gcFilter>");
        if(strcmp(gcFilter,"uResource"))
                printf("<option>uResource</option>");
        else
                printf("<option selected>uResource</option>");
        if(strcmp(gcFilter,"cName"))
                printf("<option>cName</option>");
        else
                printf("<option selected>cName</option>");
        if(strcmp(gcFilter,"cParam1"))
                printf("<option>cParam1</option>");
        else
                printf("<option selected>cParam1</option>");
        if(strcmp(gcFilter,"None"))
                printf("<option>None</option>");
        else
                printf("<option selected>None</option>");
        printf("</select>");

}//void ExttResourceListFilter(void)


void ExttResourceNavBar(void)
{

	if(guMode>10000)
		return;

	printf(LANG_NBB_SKIPFIRST);
	printf(LANG_NBB_SKIPBACK);
	printf(LANG_NBB_SEARCH);

	if(guPermLevel>=10 && !guListMode)
		printf(LANG_NBB_NEW);
	
	if(uAllowMod(uOwner,uCreatedBy))
		printf(LANG_NBB_MODIFY);
	
	if(uAllowDel(uOwner,uCreatedBy))
		printf(LANG_NBB_DELETE);

	if(uOwner)
		printf(LANG_NBB_LIST);

	printf(LANG_NBB_SKIPNEXT);
	printf(LANG_NBB_SKIPLAST);
	printf("&nbsp;&nbsp;&nbsp;\n");

}//void ExttResourceNavBar(void)


unsigned IsSecondaryOnlyZone(unsigned uZone)
{
	//This function returns the value of tZone.uSecondaryOnly
	//for ExttResourceNavBar()
	unsigned uSecondaryOnly=0;
	MYSQL_RES *res;
	MYSQL_ROW field;
	char cQuery[100]={""};
	
	sprintf(cQuery,"SELECT uSecondaryOnly FROM tZone WHERE uZone='%u'",uZone);
	mysql_query(&gMysql,cQuery);

	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));

	res=mysql_store_result(&gMysql);

	if((field=mysql_fetch_row(res)))
		sscanf(field[0],"%u",&uSecondaryOnly);

	mysql_free_result(res);

	return(uSecondaryOnly);
	
}//unsigned IsSecondaryOnlyZone(unsigned uZone)


void tResourceTableAddRR(unsigned uZoneId)
{
	guMode=2000;
	uZone=uZoneId;

	tResource("Add new resource record");

}//void tResourceTableAddRR(unsigned uZoneId)


int AutoAddPTRResource(const unsigned d,const char *cDomain,const unsigned uInZone,const unsigned uSourceZoneOwner)
{
	char gcQuery[1024];
	char cParam1[512];
	char cComment[100]={"GenerateArpaZones()"};

	if(!d || !cDomain[0] || !uInZone || !uSourceZoneOwner) return(1);

	uResource=0;
	if(cDomain[strlen(cDomain)-1]!='.')
		sprintf(cParam1,"%s.",cDomain);
	else
		sprintf(cParam1,"%s",cDomain);

	//Only allow one PTR entry per FQDN for same uOwner

	sprintf(gcQuery,"DELETE FROM tResource WHERE uRRType=%u AND cParam1='%s' AND uOwner=%u",
				RRTYPE_PTR,cParam1,uSourceZoneOwner);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql)) 
		return(2);
	
	//Check for other entries, if they exist first come first served
	//(round-robin PTR records actually)
	//notify admin to resolve dispute for rev dns

	sprintf(cName,"%u",d);
	sprintf(gcQuery,"INSERT INTO tResource SET uZone=%u,cName='%.16s',uRRType=%u,cParam1='%.511s',cComment='%.99s',"
			"uOwner=%u,uCreatedBy=%u,uCreatedDate=UNIX_TIMESTAMP(NOW())",
				uInZone
				,cName
				,RRTYPE_PTR
				,cParam1
				,cComment
				,uSourceZoneOwner
				,guLoginClient);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql)) 
		return(3);

	return(0);

}//int AutoAddPTRResource()


int AutoAddPTRResourceIPv6(const char *cIPv6PTR,const char *cDomain,const unsigned uInZone,const unsigned uSourceZoneOwner)
{
	char gcQuery[1024];
	char cParam1[512];
	char cComment[100]={"AutoAddPTRResourceIPv6()"};

	if(!cIPv6PTR[0] || !cDomain[0] || !uInZone || !uSourceZoneOwner) return(1);

	uResource=0;
	if(cDomain[strlen(cDomain)-1]!='.')
		sprintf(cParam1,"%s.",cDomain);
	else
		sprintf(cParam1,"%s",cDomain);

	//Only allow one PTR entry per FQDN for same uOwner

	sprintf(gcQuery,"DELETE FROM tResource WHERE uRRType=%u AND cParam1='%s' AND uOwner=%u",
				RRTYPE_PTR,cParam1,uSourceZoneOwner);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql)) 
		return(2);
	
	//Check for other entries, if they exist first come first served
	//(round-robin PTR records actually)
	//notify admin to resolve dispute for rev dns

	sprintf(gcQuery,"INSERT INTO tResource SET uZone=%u,cName='%.99s',uRRType=%u,cParam1='%.511s',cComment='%.99s',"
			"uOwner=%u,uCreatedBy=%u,uCreatedDate=UNIX_TIMESTAMP(NOW())",
				uInZone
				,cIPv6PTR
				,RRTYPE_PTR
				,cParam1
				,cComment
				,uSourceZoneOwner
				,guLoginClient);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql)) 
		return(3);

	return(0);

}//int AutoAddPTRResourceIPv6()


void PrepareTestData(void)
{
	CreatetResourceTest();
	sprintf(gcQuery,"TRUNCATE tResourceTest");
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
	sprintf(gcQuery,"INSERT INTO tResourceTest (uResource,cName,uOwner,uCreatedBy,uCreatedDate,uModBy,"
			"uModDate,uTTL,uRRType,cParam1,cParam2,cParam3,cParam4,cComment,uZone) "
			"SELECT uResource,cName,uOwner,uCreatedBy,uCreatedDate,uModBy,uModDate,uTTL,uRRType,"
			"cParam1,cParam2,cParam3,cParam4,cComment,uZone FROM tResource WHERE "
			"uZone=%u",uZone);

	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
	
	if(!strcmp(gcCommand,LANG_NB_CONFIRMNEW))
		sprintf(gcQuery,"INSERT INTO tResourceTest SET cName='%s',uTTL=%u,uRRType=%u,cParam1='%s'"
				",cParam2='%s',cParam3='%s',cParam4='%s',cComment='%s',uOwner=%u,uCreatedBy=%u,"
				"uCreatedDate=UNIX_TIMESTAMP(NOW()),uZone=%u",
				cName,
				uTTL,
				uRRType,
				cParam1,
				cParam2,
				cParam3,
				cParam4,
				TextAreaSave(cComment),
				guCompany,
				guLoginClient,
				uZone);
		else if(!strcmp(gcCommand,LANG_NB_CONFIRMMOD))
		sprintf(gcQuery,"UPDATE tResourceTest SET cName='%s',uTTL=%u,uRRType=%u,cParam1='%s',cParam2='%s',"
				"cParam3='%s',cParam4='%s',cComment='%s',uModBy=%u,uModDate=UNIX_TIMESTAMP(NOW()) "
				"WHERE uResource=%u",
				cName,
				uTTL,
				uRRType,
				cParam1,
				cParam2,
				cParam3,
				cParam4,
				TextAreaSave(cComment),
				guLoginClient,
				uResource);
	mysql_query(&gMysql,gcQuery);
	if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
	
}//void PrepareTestData(void)


unsigned uGetSearchGroup(const char *gcUser)
{
        MYSQL_RES *res;
        MYSQL_ROW field;
	unsigned uGroup=0;

	sprintf(gcQuery,"SELECT uGroup FROM tGroup WHERE cLabel='%s'",gcUser);
        mysql_query(&gMysql,gcQuery);
        if(mysql_errno(&gMysql))
		htmlPlainTextError(mysql_error(&gMysql));
        res=mysql_store_result(&gMysql);
	if((field=mysql_fetch_row(res)))
	{
		if(field[0]!=NULL)
			sscanf(field[0],"%u",&uGroup);
	}
	mysql_free_result(res);

	return(uGroup);

}//unsigned uGetSearchGroup(const char *gcUser)
