#!/usr/bin/env php
<?php

// Read the raw input from the POST request
$input = file_get_contents('php://input');

// Parse the plain text input (e.g., "color=red")
parse_str(str_replace("\n", "&", $input), $data);

// Extract the color value
$color = isset($data['color']) ? $data['color'] : 'white';

?>

<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>Selected Color</title>
    <style>
        body {
            background-color: <?= htmlspecialchars($color) ?>;
            font-family: Arial, sans-serif;
            text-align: center;
            padding: 2rem;
        }
    </style>
</head>
<body>
    <h1>The selected color is <?= htmlspecialchars($color) ?>!</h1>
</body>
</html>