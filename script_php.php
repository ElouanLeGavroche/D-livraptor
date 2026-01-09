<?php
$host = 'localhost';
$port = '5432';
$dbname = 'saedb';
$user = 'sae';
$password = 'racine';

$dsn = "pgsql:host=$host;port=$port;dbname=$dbname";

try {
    $pdo = new PDO($dsn, $user, $password, [
        PDO::ATTR_ERRMODE => PDO::ERRMODE_EXCEPTION,
        PDO::ATTR_DEFAULT_FETCH_MODE => PDO::FETCH_ASSOC
    ]);

} catch (PDOException $e) {
    die("Erreur de connexion à la base de données : " . $e->getMessage());
}


?>