<?php
// set up server-side configuration:
// https://www.spiceworks.co.jp/blog/?p=7603

require_once __DIR__.'/../google-api-php-client-1.1.7/src/Google/autoload.php';
$json_path = __DIR__.'/../<calendar-setting.json>'; // to be configured
$json_string = file_get_contents($json_path, true);
$json = json_decode($json_string, true);
 
$private_key = $json['private_key'];
$client_email = $json['client_email'];
 
$scopes = array(Google_Service_Calendar::CALENDAR_READONLY);
 
$credentials = new Google_Auth_AssertionCredentials(
    $client_email,
    $scopes,
    $private_key
);
 
$client = new Google_Client();
$client->setApplicationName("Google Calendar PHP API");
$client->setAssertionCredentials($credentials);
if ($client->getAuth()->isAccessTokenExpired()) {
    $client->getAuth()->refreshTokenWithAssertion();
}
 
$service = new Google_Service_Calendar($client);
 
// ここでデータを取得する範囲を決めています
$t = time();
//$t2 = $t + 4* 7 * 24 * 60 * 60; // 1 month
$t2 = $t + 7 * 24 * 60 * 60; // 1 week

$calendarId = 'akita@ifdl.jp';
$optParams = array(
  'maxResults' => 100,
  'orderBy' => 'startTime',
  'singleEvents' => TRUE,
  'timeMin' => date('c', $t),
  'timeMax' => date('c', $t2),
);
 
$events = $service->events->listEvents($calendarId, $optParams);

$delim0 = '';

echo "{\"data\":[";
//for ($d = 0; $d < 4*7; $d++){
for ($d = 0; $d < 7; $d++){
  echo $delim0;
  echo "{";
  $ttime = intval(time() /(24*60*60)) * 24*60*60 - 9 * 60*60+ $d * 24 * 60 * 60;
  echo "\"date\":";
  echo "\"".date("Y/m/d(D)", $ttime)."\",";
  echo "\"item\":[";
  $delim='';
  $prev_is_allday=false;
  foreach ($events as $item)
  {
    $ctime = intval(date("ymd",$ttime));
    $stime = intval(substr($item->start->date,2,2).substr($item->start->date,5,2).substr($item->start->date,8,2));
    $etime = intval(substr($item->end->date,2,2).substr($item->end->date,5,2).substr($item->end->date,8,2));
    if (date("d", $ttime) == substr($item->start->dateTime,8,2))
    {
        echo $delim;
        echo "{";
        echo "\"start\":\"".substr($item->start->dateTime,11,5)."\",";
        echo "\"end\":\"".substr($item->end->dateTime,11,5)."\",";
        echo "\"summary\":\"".$item->summary."\",";
        echo "\"description\":\"".$item->description."\"";
        echo "}";
        $delim = ',';
    }
     else if ($ctime >= $stime && $ctime < $etime)
    {
        echo $delim;
        echo "{";
        echo "\"start\":\"[終日]\",";
        echo "\"end\":\"\",";
        echo "\"summary\":\"".$item->summary."\",";
        echo "\"description\":\"".$item->description."\"";
        echo "}";
        $delim = ',';
     }
  }
  echo "]}";
  $delim0 = ',';
}
echo "]}";
