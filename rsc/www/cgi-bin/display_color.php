#!/usr/bin/env php
<?php

// Read POST input from STDIN
$input = file_get_contents('php://stdin');

// Extract the value of "color" from the input
$color = 'white'; // Default color
if (strpos($input, 'color=') !== false) {
    // Split on 'color=' and take the part after it
    $parts = explode('color=', $input, 2);
    if (isset($parts[1])) {
        // Take the value up to the next delimiter (e.g., '&' or end)
        $color_part = explode('&', $parts[1])[0];
        $color = trim($color_part) ?: 'white';
    }
}

// Output HTTP header
header('Content-Type: text/html');

// Generate HTML response
echo <<<HTML
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>Selected Color</title>
    <style>
        body {
            background-color: $color;
            font-family: Arial, sans-serif;
            text-align: center;
            padding: 2rem;
        }
    </style>
</head>
<body>
    <h1>The selected color is $color!</h1>
</body>
</html>
HTML;

?>