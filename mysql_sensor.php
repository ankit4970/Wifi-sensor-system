<html>
<head>
<script src="http://maps.googleapis.com/maps/api/js"></script>
<style type="text/css">
  
td {
  height: 70px;
}

</style>
<script>
<?php
  //echo 'starting again'."\r\n"; 
    $link = mysql_connect('127.0.0.1', 'root', 'ankit');
    if(! $link) 
    {
          echo 'error';
    die('Could not connect: ' . mysql_error());
    }
   
    //echo 'Connected successfully'."\r\n"; 
   
    mysql_select_db('test') 
    or die('Could not select database');

  // Performing SQL query
  //$query = "SELECT  FROM sensorData";
  $query = "SELECT lat, lon, temperature, datetime, lightPercentValue FROM sensorData WHERE id=(SELECT MAX(id) FROM sensorData) limit 1";
        
  $result = mysql_query($query) or die('Query failed: ' . mysql_error());
  // Printing results in HTML
  $line = mysql_fetch_array($result);
   
  //print_r($line);  

  $latlonString = $line["lat"].','.$line["lon"];
    mysql_close($conn);
?>
function initialize() 
{
	var mapProp = {
		center:new google.maps.LatLng(<?php echo $latlonString; ?>),
    		zoom:14,
    		mapTypeId:google.maps.MapTypeId.ROADMAP
  		};
  
	var map=new google.maps.Map(document.getElementById("googleMap"),mapProp);
  var myLatLng = {lat: <?php echo $line["lat"]; ?>, lng: <?php echo $line["lon"]; ?>};

   var marker = new google.maps.Marker({
    position: myLatLng,
    map: map,
    title: 'Current Location'
  });
}

google.maps.event.addDomListener(window, 'load', initialize);
</script>
</head>
<body>
<div id="googleMap" style="width:50%;height:100%;float:right;"></div>
<div >
<h1 align="center" style="color : blue" >GPS Tracking</h1>
<table  style="font-size:30px;text-align:left;" width="50%" align="left">
   
    <tr>
      <td>Temprature</td><td><?php echo $line["temperature"]; ?></td>
      
    </tr>
    <tr>
      <td>Light</td><td><?php echo $line["lightPercentValue"]; ?></td>
      
    </tr>
    <tr>
      <td>Lattitude</td><td><?php echo $line["lat"]; ?></td>
      
    </tr>
    <tr>
      <td>Longitude</td><td><?php echo $line["lon"]; ?></td>      
    </tr>
    <tr>
      <td>Date Time</td><td><?php echo $line["datetime"]; ?></td>      
    </tr>

</table>
</div>
</body>
</html>
