#!/bin/bash

# Read POST data (plain text format like "color=blue")
read -r input

# Extract color from input
color=$(echo "$input" | awk -F '=' '/color/ {print $2}')
color=${color:-white} # Default if empty

# Generate styled HTML response
cat <<EOF
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Selected Color</title>
  <link href="https://fonts.googleapis.com/css2?family=Inter:wght@400;600;700&display=swap" rel="stylesheet"/>
  <link rel="stylesheet" href="/style.css"/>
  <style>
    html, body {
      margin: 0;
      height: 100%;
      width: 100%;
      font-family: 'Inter', sans-serif;
      color: #fff;
      display: flex;
      justify-content: center;
      align-items: center;
      text-align: center;
      background-color: $color;
    }

    .wrapper {
      max-width: 600px;
      max-height: 400px;
      padding: 3rem;
      background: rgba(0, 0, 0, 0.7);
      border-radius: 16px;
      box-shadow: 0 0 20px rgba(0, 0, 0, 0.3);
      display: flex;
      flex-direction: column;
      justify-content: center;
      overflow: auto;  /* Allow scrolling if content overflows */
    }

    h2 {
      font-size: 2rem;
      margin-bottom: 1rem;
    }

    p {
      font-size: 1.2rem;
      margin-bottom: 1.5rem;
    }

    .button {
      padding: 0.75rem 2rem;
      background-color: #007BFF;
      color: white;
      text-decoration: none;
      border-radius: 10px;
      font-size: 1.1rem;
      display: inline-block;
      margin-top: 1.5rem;
      cursor: pointer;
    }

    .button:hover {
      background-color: #0056b3;
    }
  </style>
</head>
<body>
  <div class="wrapper">
    <h2>Color Selected</h2>
    <p>The selected background color is <strong style="color:$color;">$color</strong>.</p>
    <a href="/" class="button">Back to Home</a>
  </div>
</body>
</html>
EOF

