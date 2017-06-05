<?php
require_once('unxsapi.php');

if(count($_GET))
{
	ConnectDb();

	$cZone=$_GET['cZone'];
	$Zone=new unxsBindZone();
	$Zone->uOwner=5;
	$Zone->uCreatedBy=6;
	$Zone->cZone=$cZone;

	echo("Zone name: $cZone<br>");
	echo("Zone RRs:<br>");
	
	$RRs=$Zone->GetRRs();
	?>
	<table>
	<tr><th>Name</th><th>TTL</th><th>Type</th><th>Param 1</th><th>Param 2</th><th>Param 3</th><th>Param 4</th><th>Comment</th></tr>
	<?
	foreach ($RRs as $RR)
	{
		echo("<tr>");
		echo("<td>".$RR->GetProperty("Name")."</td>");
		echo("<td>".$RR->GetProperty("TTL")."</td>");
		echo("<td>".$RR->GetProperty("Type")."</td>");
		echo("<td>".$RR->GetProperty("Param 1")."</td>");
		echo("<td>".$RR->GetProperty("Param 2")."</td>");
		echo("<td>".$RR->GetProperty("Param 3")."</td>");
		echo("<td>".$RR->GetProperty("Param 4")."</td>");
		echo("<td>".$RR->GetProperty("Comment")."</td>");
		echo("</tr>");
	}
	echo("</table>");
	return;
}

if(!count($_POST))
{
	$cMessage="&nbsp;";
	UI();
	return;
}
$gcFunction=$_POST['gcFunction'];
$cZone=$_POST['cZone'];
$cIP=$_POST['cIP'];

ConnectDb();

$Zone=new unxsBindZone();
$Zone->uOwner=5;
$Zone->uCreatedBy=6;
$Zone->cZone=$cZone;
if($gcFunction=='Add Zone')
{
	$Zone->Create();
	$cMessage=$Zone->cErrMsg;
	//Add the two webzone RRs
	$Zone->AddRR("@",0,$cIP,"","","","A",0);
	$Zone->AddRR("www",0,"$cZone.","","","","CNAME",0);

	if($cMesage=='')
		$cMessage="Zone added OK";
}
/*else if($gcFunction=='Modify Zone')
{
	UpdateZone($cZone,$cIP);
	SubmitJob("Modify",1,$cZone,$uTime,$uOwner,$uCreatedBy);
	$cMessage="Zone modified OK";
}
*/
else if($gcFunction=='Delete Zone')
{
	$Zone->Delete();
	$cMessage=$Zone->cErrMsg;
	if($cMessage=='')
		$cMessage="Zone deleted OK";
}

UI();

function ConnectDb()
{
        mysql_connect('localhost','idns','wsxedc') or die ('Could not connect:'. mysql_error());
        mysql_select_db('idns') or die ('Could not select iDNS database:'. mysql_error());
}//function ConnectDb()


function UI()
{
	global $cMessage;
?>
<html>
<body>
<form method=post>
<? echo $cMessage ?>
<table>
<tr><td>Zone name</td></td><td><input type text name=cZone></td></tr>
<tr><td>IP address</td></td><td><input type text name=cIP></td></tr>
<tr><td>&nbsp;</td></tr>
<tr><td colspan=2><input type=submit name=gcFunction value='Add Zone'> <input type=submit name=gcFunction value='Modify Zone'> <input type=submit name=gcFunction value='Delete Zone'></td></tr>
</table>
</form>
</html>
<?
}//function UI()

?>
