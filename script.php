<?php

// URL cible
$url = "https://localhost:4242/data/www/manon.html";

// Envoi de la requête GET
$response = file_get_contents($url);

// Affichage de la réponse
echo $response;

?>
