#!/usr/bin/env python3

import sys


# Read the raw input from the POST request
input_data = sys.stdin.read()

# Parse the plain text input (e.g., "name=John\nage=30")
data = dict(item.split('=') for item in input_data.splitlines() if '=' in item)

# Extract the values for "name" and "age"
name = data.get("name", "Unknown")
age = data.get("age", "Unknown")

# Generate the HTML content
html_content = f"""
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>Informations de l'Utilisateur</title>
    <style>
        body {{
            font-family: Arial, sans-serif;
            background-color: #f0f0f0;
            text-align: center;
            padding: 2rem;
        }}
    </style>
</head>
<body>
    <h1>Welcome, {name}!</h1>
    <p>You are {age} years old.</p>
</body>
</html>
"""

print(html_content)