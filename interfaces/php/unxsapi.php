<?
/*
FILE
	unxsapi.php
PURPOSE
	Provide the required PHP functions
	to interact with the iDNS database
AUTHOR
	(C) 2009 Hugo Urquiza for Unixservice
*/


class unxsBindZone
{
	public $cZone='';
	public $uNSSet=1; //Default to first tNSSet record
	public $uOwner=1; //Default Root
	public $uCreatedBy=1; //Default Root
	public $uModBy=1; //Default Root
	public $cErrMsg='';
	public $uERrCode=0;


	public function Create()
	{
		if($this->cZone=='')
		{
			$this->uErrCode=1;
			$this->cErrMsg="Can't create a zone without defining the cZone property";
			return($this->uErrCode);
		}
		
		if($this->uNSSet==0)
		{
			$this->uErrCode=2;
			$this->cErrMsg="Can't create a zone without defining the uNSSet property";
			return($this->uErrCode);
		}

		//Check if zone exists

		if($this->ZoneExists())
		{
			$this->uErrCode=3;
			$this->cErrMsg="Zone with name $this->cZone already exists";
			return($this->uErrCode);
		}

		$uSerial=$this->SerialNum();

		$gcQuery="INSERT INTO tZone SET cZone='$this->cZone',uNSSet=1,cHostmaster='support.unixservice.com',"
			."uSerial='$uSerial',uExpire=604800,uRefresh=28800,uTTL=86400,"
			."uRetry=7200,uZoneTTL=86400,uMailServers=0,uView=2,uOwner=$this->uOwner,"
			."uCreatedBy=$this->uCreatedBy,uCreatedDate=UNIX_TIMESTAMP(NOW())";
		mysql_query($gcQuery);
		
		if(mysql_errno())
		{
			$this->uErrCode=5; 
			$this->cErrMsg=mysql_error();
			return($this->uErrCode);
		}
		
		if($this->SubmitJob("New"))
		{
			$this->cErrMsg="Could not submit job: ".$this->cErrMsg;
			return($this->uErrCode);
		}

		return(0);

	}//public function Create()


	public function Delete()
	{
		if($this->cZone=='')
		{
			$this->uErrCode=1;
			$this->cErrMsg="Can't delete a zone without defining the cZone property";
			return($this->uErrCode);
		}

		if($this->uNSSet==0)
		{
			$this->uErrCode=2;
			$this->cErrMsg="Can't delete a zone without defining the uNSSet property";
			return($this->uErrCode);
		}
		
		if(!$this->ZoneExists())
		{
			$this->uErrCode=4;
			$this->cErrMsg="Zone with name $this->cZone doesn't exist";
			return($this->uErrCode);
		}
		
		$gcQuery="DELETE FROM tResource WHERE uZone IN "
			."(SELECT uZone FROM tZone WHERE cZone='$this->cZone' AND uView=2)";
		mysql_query($gcQuery);
		
		if(mysql_errno())
		{
			$this->uErrCode=5; 
			$this->cErrMsg=mysql_error();
			return($this->uErrCode);
		}
		
		$gcQuery="DELETE FROM tZone WHERE cZone='$this->cZone' AND uView=2";
		mysql_query($gcQuery);
		
		if(mysql_errno())
		{
			$this->uErrCode=5; 
			$this->cErrMsg=mysql_error();
			return($this->uErrCode);
		}

		if($this->SubmitJob("Delete"))
		{
			$this->uErrCode=6;
			$this->cErrMsg="Could not submit iDNS job";
			return($this->uErrCode);
		}

	}//public function Delete()


	public function AddRR($cName,$uTTL,$cParam1,$cParam2,$cParam3,$cParam4,$cRRType,$uOwner)
	{
		//This function creates a RR and returns a new unxsBindResourceRecord class
		//to handle it

		$RR=new unxsBindResourceRecord();
		$RR->SetProperty("Zone RID",$this->GetuZone());
		$RR->SetProperty("Name",$cName);
		$RR->SetProperty("TTL",$uTTL);
		$RR->SetProperty("Param 1",$cParam1);
		$RR->SetProperty("Name",$cParam2);
		$RR->SetProperty("Name",$cParam3);
		$RR->SetProperty("Name",$cParam4);
		$uRRType=$RR->GetRRTypeRID($cRRType);
		$RR->SetProperty("Type RID",$uRRType);
		if($uOwner!=0)
			$RR->SetProperty("Owner RID",$uOwner);
		else
			$RR->SetProperty("Owner RID",$this->uOwner);
		$RR->SetProperty("Created By RID",$this->uCreatedBy);

		$RR->CommitChanges();
		if($RR->uErrCode!=0)
			return(NULL);

		$this->UpdateSerial();
		$this->SubmitJob("Modify");

		return($RR);

	}//public function AddRR($cName,$uTTL,$cParam1,$cParam2,$cParam3,$cParam4,$cRRtype)

	
	private function GetuZone()
	{
		$res=mysql_query("SELECT uZone FROM tZone WHERE cZone='$this->cZone' AND uView=2");
		if(mysql_errno())
		{
			$this->uErrCode=5;
			$this->cErrMsg=mysql_error();
			return(NULL);
		}
		if(($field=mysql_fetch_row($res)))
			return($field[0]);

		return(0);
	}//private function GetuZone()

		
	public function GetRRs()
	{
		//
		//This function returns an array, containing a RR per each element.
		//For doing so we use a unxsBindResourceRecord class array
		//Each of this array elements can be then manipulated
		//using the unxsBindResourceRecord methods and properties
		$res=mysql_query("SELECT uResource FROM tResource WHERE uZone IN (SELECT uZone "
				."FROM tZone WHERE cZone='$this->cZone' AND uView=2)");
		if(mysql_errno())
		{
			$this->uErrCode=5;
			$this->cErrMsg=mysql_error();
			return(NULL);
		}
	
		$i=0;
		while(($field=mysql_fetch_row($res)))
		{
			$retArray[$i]=new unxsBindResourceRecord();
			$retArray[$i]->LoadRR($field[0]);
			$i++;
		}
		return($retArray);
	
	}//public function GetRRs()


	public function GetProperty($cPropName)
	{
		if($this->cZone=='')
		{
			$this->uErrCode=1;
			$this->cErrMsg="Can't delete a zone without defining the cZone property";
			return($this->uErrCode);
		}
		if($cPropName=='Hostmaster')
			return($this->Get_tZoneField("cHostmaster"));
		else if($cPropName=='NS Set')
			return($this->Get_tZoneField("uNSSet"));
		else if($cPropName=='Serial')
			return($this->Get_tZoneField("uSerial"));
		else if($cPropName=='Expire TTL')
			return($this->Get_tZoneField("uExpire"));
		else if($cPropName=='Refresh TTL')
			return($this->Get_tZoneField("uRefresh"));
		else if($cPropName=='Default TTL')
			return($this->Get_tZoneField("uTTL"));
		else if($cPropName=='Retry TTL')
			return($this->Get_tZoneField("uRetry"));
		else if($cPropName=='Zone TTL')
			return($this->Get_tZoneField("uZoneTTL"));
		else if($cPropName=='View RID')
			return($this->Get_tZoneField("uView"));
		else if($cPropName=='Registrar RID')
			return($this->Get_tZoneField("uRegistrar"));
		else if($cPropName=='Is Secondary Only')
			return($this->Get_tZoneField("uSecondaryOnly"));
		else if($cPropName=='Options')
			return($this->Get_tZoneField("cOptions"));
		else if($cPropName=='Owner RID')
			return($this->Get_tZoneField("uOwner"));
		else if($cPropName=='Created By RID')
			return($this->Get_tZoneField("uCreatedBy"));
		else if($cPropName=='Creation Date')
			return($this->Get_tZoneField("uCreatedDate"));
		else if($cPropName=='Modified By RID')
			return($this->Get_tZoneField("uModBy"));
		else if($cPropName=='Modification Date')
			return($this->Get_tZoneField("uModDate"));
	}


	private function Get_tZoneField($cFieldname)
	{
		$res=mysql_query("SELECT $cFieldname FROM tZone WHERE cZone='$this->cZone' AND uView=2");
		if(mysql_errno())
		{
			$this->uErrCode=5;
			$this->cErrMsg=mysql_error();
			return(NULL);
		}
		if($field=mysql_fetch_row($res))
			return($field[0]);
		else
			return(NULL);

	}//private function Get_tZoneField($cFieldname)


	private function Set_tZoneField($cFieldname,$cValue)
	{
		mysql_query("UPDATE tZone SET $cFieldname='$cValue' WHERE cZone='$this->cZone' AND uView=2");
		if(mysql_errno())
		{
			$this->uErrCode=5;
			$this->cErrMsg=mysql_error();
		}
		return(0);

	}//private function Set_tZoneField($cFieldname,$cValue)


	public function SetProperty($cPropName,$cValue)
	{
		if($this->cZone=='')
		{
			$this->uErrCode=1;
			$this->cErrMsg="Can't set a zone property without defining the cZone property";
			return($this->uErrCode);
		}
		
		if($this->uNSSet==0)
		{
			$this->uErrCode=2;
			$this->cErrMsg="Can't set a zone property without defining the uNSSet property";
			return($this->uErrCode);
		}
		
		if($cPropName=='Hostmaster')
			return($this->Set_tZoneField("cHostmaster",$cValue));
		else if($cPropName=='NS Set')
			return($this->Set_tZoneField("uNSSet",$cValue));
		else if($cPropName=='Serial')
			return($this->Set_tZoneField("uSerial",$cValue));
		else if($cPropName=='Expire TTL')
			return($this->Set_tZoneField("uExpire",$cValue));
		else if($cPropName=='Refresh TTL')
			return($this->Set_tZoneField("uRefresh",$cValue));
		else if($cPropName=='Default TTL')
			return($this->Set_tZoneField("uTTL",$cValue));
		else if($cPropName=='Retry TTL')
			return($this->Set_tZoneField("uRetry",$cValue));
		else if($cPropName=='Zone TTL')
			return($this->Set_tZoneField("uZoneTTL",$cValue));
		else if($cPropName=='View RID')
			return($this->Set_tZoneField("uView",$cValue));
		else if($cPropName=='Registrar RID')
			return($this->Set_tZoneField("uRegistrar",$cValue));
		else if($cPropName=='Is Secondary Only')
			return($this->Set_tZoneField("uSecondaryOnly",$cValue));
		else if($cPropName=='Options')
			return($this->Set_tZoneField("cOptions",$cValue));
		else if($cPropName=='Owner RID')
			return($this->Set_tZoneField("uOwner",$cValue));
		else if($cPropName=='Created By RID')
			return($this->Set_tZoneField("uCreatedBy",$cValue));
		else if($cPropName=='Creation Date')
			return($this->Set_tZoneField("uCreatedDate",$cValue));
		else if($cPropName=='Modified By RID')
			return($this->Set_tZoneField("uModBy",$cValue));
		else if($cPropName=='Modification Date')
			return($this->Set_tZoneField("uModDate",$cValue));
	}


	private function ZoneExists()
	{
		$res=mysql_query("SELECT uZone FROM tZone WHERE cZone='$this->cZone' "
				."AND uView=2") or die(mysql_error());
		return(mysql_num_rows($res));

	}//private function ZoneExists()


	private function SubmitJob($cCommand)
	{
		//This function inserts a new tJob entry per each NS Set member
		$gcQuery="SELECT tNS.cFQDN,tNSType.cLabel,tServer.cLabel FROM "
			."tNS,tNSType,tServer WHERE tNSType.uNSType=tNS.uNSType "
			."AND tServer.uServer=tNS.uServer AND tNS.uNSSet=$this->uNSSet "
			."ORDER BY tNS.uNSType";
		$res=mysql_query($gcQuery);
	
		if(mysql_errno())
		{
			$this->uErrCode=5; 
			$this->cErrMsg=mysql_error();
			return($this->uErrCode);
		}

		$uTime=time();
		while(($field=mysql_fetch_row($res)))
		{
			$gcQuery="INSERT INTO tJob SET cJob='$cCommand',cZone='$this->cZone',"
				."uNSSet=$this->uNSSet,cTargetServer='$field[0] $field[1]',"
				."uTime=$uTime,cJobData='$field[2]',uCreatedBy=$this->uCreatedBy,"
				."uOwner=$this->uOwner,uCreatedDate=UNIX_TIMESTAMP(NOW())";
			mysql_query($gcQuery);
			if(mysql_errno())
			{
				$this->uErrCode=5; 
				$this->cErrMsg=mysql_error();
				return($this->uErrCode);
			}
	
			$uJob=mysql_insert_id();
			if($field[1]='MASTER')
				$uMasterJob=$uJob;
			if($uMasterJob)
			{
				$gcQuery="UPDATE tJob SET uMasterJob=$uMasterJob WHERE uJob=$uJob";
				mysql_query($gcQuery);
				if(mysql_errno())
				{
					$this->uErrCode=5; 
					$this->cErrMsg=mysql_error();
					return($this->uErrCode);
				}
			}
		}
		
		return(0);

	}//private function SubmitJob($cCommand)
	

	private function UpdateSerial()
	{
	
		$gcQuery="SELECT uSerial,uZone FROM tZone WHERE cZone='$this->cZone' AND uView=2";
		$res=mysql_query($gcQuery) or die(mysql_error());
		
		if(($field=mysql_fetch_row($res)))
		{
			$uSerial=$field[0];
			$uZone=$field[1];
		}
	
		$luYearMonDay=$this->SerialNum();

		//Typical year month day and 99 changes per day max
		//to stay in correct date format. Will still increment even if>99 changes in one day
		//but will be stuck until 1 day goes by with no changes.
		if($uSerial<$luYearMonDay)
			$gcQuery="UPDATE tZone SET uSerial=$luYearMonDay WHERE uZone=$uZone";
		else
			$gcQuery="UPDATE tZone SET uSerial=uSerial+1 WHERE uZone=$uZone";

		mysql_query($gcQuery) or die(mysql_error());
	
	}//private function UpdateSerial()


	private function SerialNum()
	{
		return(strftime("%Y%m%d00"));

	}//private function SerialNum()

}//class unxsBindZone


class unxsBindResourceRecord
{
	var $uResource=0;
	var $uZone;
	var $cName;
	var $uTTL;
	var $uRRType;
	var $cParam1;
	var $cParam2;
	var $cParam3;
	var $cParam4;
	var $cComment;
	var $uOwner=1;
	var $uCreatedBy=1;
	var $uCreatedDate;
	var $uModBy=1;
	var $uModDate;
	
	var $uErrCode=0;
	var $cErrMsg='';


	public function GetProperty($cPropName)
	{
		if($cPropName=="RID")
			return($this->uResource);
		if($cPropName=="Name")
			return($this->cName);
		if($cPropName=="Zone RID")
			return($this->uZone);
		if($cPropName=="TTL")
			return($this->uTTL);
		if($cPropName=="Type RID")
			return($this->uRRtype);
		if($cPropName=="Param 1")
			return($this->cParam1);
		if($cPropName=="Param 2")
			return($this->cParam2);
		if($cPropName=="Param 3")
			return($this->cParam3);
		if($cPropName=="Param 4")
			return($this->cParam4);
		if($cPropName=="Owner RID")
			return($this->uOwner);
		if($cPropName=="Creation Date")
			return($this->uCreatedDate);
		if($cPropName=="Created By RID")
			return($this->uCreatedBy);
		if($cPropName=="Modified By RID")
			return($this->uModBy);
		if($cPropName=="Modification Date")
			return($this->uModDate);
		if($cPropName=="Type")
			return($this->GetRRTypeLabel($this->uRRType));
		
		return(NULL);	

	}//public function GetProperty($cPropName)


	public function SetProperty($cPropName,$cValue)
	{
		if($cPropName=="Name")
			$this->cName=$cValue;
		if($cPropName=="Zone RID")
			$this->uZone=$cValue;
		if($cPropName=="TTL")
			$this->uTTL=$cValue;
		if($cPropName=="Type RID")
			$this->uRRtype=$cValue;
		if($cPropName=="Param 1")
			$this->cParam1=$cValue;
		if($cPropName=="Param 2")
			$this->cParam2=$cValue;
		if($cPropName=="Param 3")
			$this->cParam3=$cValue;
		if($cPropName=="Param 4")
			$this->cParam4=$cValue;
		if($cPropName=="Owner RID")
			$this->uOwner=$cValue;
		if($cPropName=="Created By RID")
			$this->uCreatedBy=$cValue;
		if($cPropName=="Modified By RID")
			$this->uModBy=$cValue;
	
	}//public function SetProperty($cPropName,$cValue)


	public function CommitChanges()
	{
		//This function will commit the class data to the database
		//To know if we are adding a new record or updating one
		//we check $uResource. if(==0) new record

		if($this->uResource)
		{
			$gcQuery="UPDATE tResource SET uZone='$this->uZone',"
				."cName='$this->cName',uTTL='$this->uTTL',"
				."uRRType='$this->uRRtype',cParam1='$this->cParam1',"
				."cParam2='$this->cParam2',cParam3='$this->cParam3',"
				."cParam4='$this->cParam4',cComment='$this->cComment',"
				."uOwner='$this->uOwner',uModBy='$this->uModBy',"
				."uModDate=UNIX_TIMESTAMP(NOW())";
		}
		else
		{
			$gcQuery="INSERT INTO tResource SET uZone='$this->uZone',"
				."cName='$this->cName',uTTL='$this->uTTL',"
				."uRRType='$this->uRRtype',cParam1='$this->cParam1',"
				."cParam2='$this->cParam2',cParam3='$this->cParam3',"
				."cParam4='$this->cParam4',cComment='$this->cComment',"
				."uOwner='$this->uOwner',uCreatedBy='$this->uCreatedBy',"
				."uCreatedDate=UNIX_TIMESTAMP(NOW())";
		}
		mysql_query($gcQuery);
		if(mysql_errno())
		{
			$this->uErrCode=5;
			$this->cErrMsg=mysql_error();
			return($this->uErrCode);
		}
		return(0);

	}//public function CommitChanges()


	public function LoadRR($uResource)
	{
		$res=mysql_query("SELECT uZone,cName,uTTL,uRRType,cParam1,"
				."cParam2,cParam3,cParam4,cComment,uOwner,"
				."uCreatedBy,uCreatedDate,uModBy,uModDate "
				."FROM tResource WHERE uResource=$uResource");
		if(mysql_errno())
		{
			$this->uErrCode=5;
			$this->cErrMsg=mysql_error()."uResource=$uResource";
			return(NULL);
		}
		if($field=mysql_fetch_row($res))
		{
			$this->uResource=$uResource;
			$this->uZone=$field[0];
			$this->cName=$field[1];
			$this->uTTL=$field[2];
			$this->uRRType=$field[3];
			$this->cParam1=$field[4];
			$this->cParam2=$field[5];
			$this->cParam3=$field[6];
			$this->cParam4=$field[7];
			$this->cComment=$field[8];
			$this->uOwner=$field[9];
			$this->uCreatedBy=$field[10];
			$this->uCreatedDate=$field[11];
			$this->uModBy=$field[12];
			$this->uModDate=$field[13];
		}
		return(0);

	}//public function LoadRR($uResource)


	public function GetRRTypeRID($cRRType)
	{
		$res=mysql_query("SELECT uRRType FROM tRRType WHERE cLabel='$cRRType'");
		if(mysql_errno())
		{
			$this->uErrCode=5;
			$this->cErrMsg=mysql_error();
			return(NULL);
		}
		if(($field=mysql_fetch_row($res)))
			return($field[0]);

		return(0);

	}//public function GetRRTypeRID($cRRType)
	

	public function GetRRTypeLabel($uRRType)
	{
		$res=mysql_query("SELECT cLabel FROM tRRType WHERE uRRType='$uRRType'");
		if(mysql_errno())
		{
			$this->uErrCode=5;
			$this->cErrMsg=mysql_error();
			return(NULL);
		}
		if(($field=mysql_fetch_row($res)))
			return($field[0]);

		return('');

	}//private function GetRRTypeLabel($uRRType)


}//class unxsBindResourceRecord
?>
