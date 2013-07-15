<?php

	print("<H4><center>DHCP TFTP Server (Option 66) Configuration</center></H4>\n");
	print("<pre>");

	$default="";

	$servers=array();
	$dhcp=file("/etc/dhcpd.conf");
	foreach ($dhcp as $line)
	{
		if (substr($line,0,1)=="#")
			continue;
		if (preg_match('/\s*option\s+tftp-server-name\s+\"(.*)\";/',$line,$matches) && $default=="")
			$default=$matches[1];
		if (preg_match('/\s*host\s+(.*)\s+\{\s+hardware ethernet\s+(\S+);\s+option tftp-server-name \"(.*)\";/',$line,$matches))
			$servers[$matches[2]]=$matches[3];
	}

	if (!empty($_POST['mac']))
	{
		$mac=$_POST['mac'];
		$server=$_POST['server'];
		print('Changing '.$mac.' to "'.$server.'"...'."\n");

		if ($mac=="default")
			$default=$server;
		else
			$servers[$mac]=$server;

		$conf=<<<EOF
authoritative;
ddns-update-style interim;
#ddns-updates on;
ignore client-updates;
#one-lease-per-client false;
#allow bootp;

subnet 10.1.1.0 netmask 255.255.255.0 {

  option routers                  10.1.1.254;
  option subnet-mask              255.255.255.0;
  option nis-domain               "axiatp.net";
  option domain-name              "axiatp.net";
  option domain-name-servers      208.67.222.222, 4.2.2.4;
  option time-offset              -18000; # Eastern Standard Time
  option ntp-servers              69.160.201.3;
  option tftp-server-name         "$default";


  range dynamic-bootp 10.1.1.32 10.1.1.199;
  default-lease-time 43200;
  max-lease-time 86400;
}
EOF;

		$conf.="\ngroup {\n";

		$i=0;
		foreach ($servers as $m => $s)
		{
			if ($s=="")
				continue;
			$i=$i+1;
			$qs='"'.$s.'"';
			$conf.=" host n$i { hardware ethernet $m; option tftp-server-name $qs; }\n";
		}
		$conf.="}\n";

		file_put_contents("/tmp/dhcpd.conf",$conf);
		print("Installing configuration...\n");
		system("sudo cp /tmp/dhcpd.conf /etc");
		print("Restarting dhcp service...\n");
		system("sudo service dhcpd restart");
	}

	print("</pre>");

	function vendor($mac)
	{
		$dashed=substr($mac,0,2)."-".substr($mac,3,2)."-".substr($mac,6,2);
		$handle=popen("fgrep ".strtoupper($dashed)." oui.txt","r");
		$line=fread($handle,256);
		pclose($handle);
		preg_match('/.*\(hex\)\s+(.*$)/',$line,$matches);
		return($matches[1]);
	}

	$leases=array();

	$ip="";

	$file=file("/var/lib/dhcpd/dhcpd.leases");
	foreach ($file as $line)
	{
		if (substr($line,0,1)=="#")
			continue;

		if (preg_match('/^lease\s*([0-9.]+)\s/',$line,$matches))
		{
			$ip=$matches[1];
			$leases[$ip]=array();
		}
		elseif (preg_match('/^\s.(\S+\s+\S+\s+\S+)\s+(\S+);/',$line,$matches))
			$leases[$ip][$matches[1]]=$matches[2];
		elseif (preg_match('/^\s.(\S+\s+\S+)\s+(\S+);/',$line,$matches))
			$leases[$ip][$matches[1]]=$matches[2];
		elseif (preg_match('/^\s.(\S+)\s+\"(\S+)\";/',$line,$matches))
			$leases[$ip][$matches[1]]=$matches[2];
		elseif (preg_match('/^\s.(\S+)\s+(\S+);/',$line,$matches))
			$leases[$ip][$matches[1]]=$matches[2];
	}	

	
	print('<form method="post" action="dhcp.php">');
	print('Default server: ');
	print('<input type="hidden" name="mac" value="default" />');
	print('<input type="text" name="server" value="'.$default.'" />');
	print('<input type="submit" value="Save" />');
	print('</form>');

	print("<table border=1>\n");
	print("<tr><td>IP</td><td>State</td><td>MAC</td><td>Name</td><td>TFTP</td></tr>\n");
	foreach ($leases as $ip => $lease)
	{
		$mac=strtoupper($lease['hardware ethernet']);
		$vendor=vendor($lease['hardware ethernet']);
		$server=$servers[$mac];

		$form='<form method="post" action="dhcp.php" >'; //action="'.$PHP_SELF.'" >'; // enctype="multipart/form-data">';
		$form.='<input type="hidden" name="mac" value="'.$mac.'" />';
		$form.='<input type="text" name="server" value="'.$server.'" />';
		$form.='<input type="submit" value="Save" />';
		$form.='</form>';


		print("<tr><td valign=top>".$ip."</td><td valign=top>".$lease['binding state']."</td>");
		print("<td valign=top>".$mac."<br><font size=-1>".$vendor."</font></td>");
		print("<td valign=top>".$lease['client-hostname']."</td>");
		print("<td valign=top>".$form."</td></tr>\n");
	}
	print("</table>\n");

?>
