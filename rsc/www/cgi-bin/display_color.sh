#!/bin/bash

# Read the input from the POST request
read -r input

# Extract the value of the "color" key from the plain text input
color=$(echo "$input" | awk -F '=' '/color/ {print $2}')

# Default to white if no color is provided
color=${color:-white}

# HTTP Header
echo

# Generate the HTML response
cat << EOF
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
EOF