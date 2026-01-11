<?php
$socket = fsockopen("localhost", 8080);
$bordereau;


$data = fread($socket, 1024);
echo $data;
fwrite($socket, "CONN Alizon Super4\n");

function next_func(){

}
?>

<!DOCTYPE html>
<html>
    <head>
        <title>Script php</title>
    </head>

    <body>

        <form action="cnx_func()" method="post">
            <button type="submit">next</button>
        </form>


    </body>
</html>