<?php
$socket = fsockopen("localhost", 8080);
$bordereau;

$data = fread($socket, 1024);
echo $data;
fwrite($socket, "CONN Alizon Super4\n");

?>

<!DOCTYPE html>
<html>
    <head>
        <title>Script php</title>
    </head>

    <body>
        <p>
            <?php
            $res = fread($socket, 1024);
            echo $res;
            ?>
        </p>

        <p>
            <?php
            $res = fread($socket, 1024);
            echo $res;
            ?>
        </p>

        <p>
        <?php
            
            fwrite($socket, "GET 1628272142\n");
            $res = fread($socket, 1024);
            echo $res;

            $lecture = true;
            file_put_contents('./image.png', '');
            while($lecture){
                $res = fread($socket, 8192);
                if(strlen($res) == 8192){
                    file_put_contents('image.png' , $res, FILE_APPEND);
                    $res = '';
                }
                else{
                    file_put_contents('image.png' , $res, FILE_APPEND);
                    $lecture = false;
                }

            }
            
            ?>
        </p>
        <?php echo '<img src="data:image/png;base64,'.base64_encode(file_get_contents("image.png")).'" />';?>
            
    </body>
</html>