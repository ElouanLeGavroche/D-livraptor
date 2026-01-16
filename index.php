<?php
$socket = fsockopen("localhost", 8080);
$bordereau;

$data = fread($socket, 1024);
echo $data;
fwrite($socket, "CONN Alizon Super4\n");

/*
fread($socket, 1024);
$data = fwrite($socket, "CREATE 420\n");
*/
?>

<!DOCTYPE html>
<html>
    <head>
        <title>Script php</title>
        <?php include_once "elements_page_general/head.php" ?>
   
        <link rel="stylesheet" type="text/css" href="elements_page_catalogue/article.css">
        <link rel="stylesheet" type="text/css" href="elements_page_catalogue/carousselle.css">
        <link rel="stylesheet" type="text/css" href="elements_page_catalogue/une.css">
        <link rel="stylesheet" type="text/css" href="style_catalogue/style.css">

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
            <?php
            $res = fread($socket, 1024);
            echo $res;
            ?>
        </p>

        <p>
        <?php
            
            fwrite($socket, "GET 1700327895\n");
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